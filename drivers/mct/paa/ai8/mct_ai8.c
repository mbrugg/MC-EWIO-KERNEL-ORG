/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$	

	Description:	paa_ai8 Client-Driver

	 Analoger-Input DRIVER (Gerät)  mit 8 DEVICES (Input Kanälen, je 32 bit
	 float genutzt!)


*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"	// debugging
#include "../mct_paa_msg.h"		// paa-messages
#include "mct_ai8_objs.h"		// objects
#include "mct_ai8_sysfs.h"		// sysfs-support
#include "mct_ai8.h"

#ifdef MCT_PAA_AI8_DEV_IF_MBUS		
	#include "mct_ai8_mbus.h"	/// MBUS	
#endif

#ifdef MCT_PAA_AI8_DEV_IF_MODBUS		
	#include "mct_ai8_modbus.h"	/// MODBUS	
#endif

static DECLARE_COMPLETION( device_is_free );

// process-stati
#define PROC_AI8_INFO		0
#define PROC_AI8_CONFIG		1
#define PROC_AI8_STATE		2
#define PROC_AI8_DATA		3
#define PROC_AI8_IDLE		4

unsigned char process;

#define DR_GET_STATE(val)	{	\
			spin_lock_bh(&_drv->Op_Control->lock);	\
			val = _drv->Op_Control->P_Control.value[1] ;	\
			spin_unlock_bh(&_drv->Op_Control->lock);	}

#define DR_SET_STATE(val)	{	\
			spin_lock_bh(&_drv->Op_Control->lock);	\
			_drv->Op_Control->P_Control.value[1] = val ;	\
			spin_unlock_bh(&_drv->Op_Control->lock);	}

#define DR_SET_INFO(msg)	{	\
			spin_lock_bh(&_drv->Op_Physic->lock);	\
			memcpy(&_drv->Op_Physic->P_INFO.value,msg,_len);	\
			spin_unlock_bh(&_drv->Op_Physic->lock);	}

#define DR_SET_INPUT(chan,val)	{	\
			spin_lock_bh(&_drv->Op_Inputs[chan]->Op_Value->lock);	\
			_drv->Op_Inputs[chan]->Op_Value->P_Value.value = val;	\
			spin_unlock_bh(&_drv->Op_Inputs[chan]->Op_Value->lock);	}

// kein sync-Schutz notwendig!
#define DR_GET_TYPE(chan,val)	\
			val = box_get_sel(&ai8_O_Inputs[chan].Op_Type->P_Type);

static struct paa_board_info chip_ai8 = {
	.modalias 		= "chip_ai8 new???",	// Wird noch überschrieben!
	.bus_num 		= PAA_BUS_NUM,
	.slot_select 	= 0,					// Wird noch überschrieben!
	.slot_mode 		= 0,					// z.Z. keine Verwendung
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: AI8-DRIVER (on the top of PAA-Core)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//---------------------------------------------------------------
//	AI8-info-completion
//---------------------------------------------------------------
void ai8_info_cpl_fct (void * _data)	{
	struct tai8_drv  * _drv	= (struct tai8_drv *) _data;		
	ai8_control * 		_ct		= (void *) _drv->ct;
	unsigned char		_typ;
	unsigned char 		_len;
	unsigned char 		_buf[PAA_BUFFER_SIZE];

	/// FINAL WORK MESSAGE! 
	if(atomic_read( &_ct->msg_stop_job ) ) {
		DR_SET_STATE(e_shutdown);		
		complete( &_ct->msg_stop_quit ); // final msg call, shutdown
		return;
	} 
	/// MASTER-NACHRICHT holen
	spin_lock_bh(&_ct->msg_lock);
	_typ = _ct->msg.rx_typ;
	_len = _ct->msg.rx_len;
	memcpy(&_buf,&_ct->msg.rx_buf,_len);
	spin_unlock_bh(&_ct->msg_lock);

	/// Typ auswerten
	switch(_typ)	{
		case PAA_MSG_INFO:	/// INFO
			printk("%s inf: %s len: %d\n",__FUNCTION__, _buf,_len);
			process = PROC_AI8_CONFIG;
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s err: %s len: %d\n",__FUNCTION__, _buf,_len);
			process = PROC_AI8_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__, PAA_MSG_STR_UNSUPPORTED);
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			process = PROC_AI8_IDLE;
			break;
	}
	DR_SET_INFO(_buf);
}
//---------------------------------------------------------------
//	AI8-config-completion
//---------------------------------------------------------------
void ai8_config_cpl_fct (void * _data)	{
	struct tai8_drv  * 	 _drv	= (struct tai8_drv *) _data;		
	ai8_control * 		 _ct	=  _drv->ct;
	unsigned char		 _typ;
	unsigned char 		 _len;
	unsigned char 		 _buf[PAA_BUFFER_SIZE];

	/// FINAL MESSAGE! 
	if(atomic_read( &_ct->msg_stop_job ) ) {
		DR_SET_STATE(e_shutdown);		
		complete( &_ct->msg_stop_quit ); // final msg call, shutdown
		return;
	} 
	/// MASTER-NACHRICHT holen
	spin_lock_bh(&_ct->msg_lock);
	_typ = _ct->msg.rx_typ;
	_len = _ct->msg.rx_len;
	 memcpy(&_buf,&_ct->msg.rx_buf,_len);
	spin_unlock_bh(&_ct->msg_lock);

	/// Typ auswerten
	switch(_typ)	{
		case PAA_MSG_STATE:		/// STATE
			//printk("%s cfg pending\n",__FUNCTION__);
			process = PROC_AI8_STATE;
			break;
		case PAA_MSG_CONFIG:	/// CONFIG
			DEBUG(MCT_DEBUG_LEVEL3,"%s cfg:%d-%d-%d-%d-%d-%d-%d-%d\n",__FUNCTION__,\
				_buf[0],_buf[1],_buf[2],_buf[3],_buf[4],_buf[5],_buf[6],_buf[7]);
			process = PROC_AI8_DATA;
			break;
		case PAA_MSG_ERROR:		/// ABBRUCH
			printk("%s err:%s len:%d\n",__FUNCTION__,_buf,_len);
			DR_SET_INFO(_buf);
			process = PROC_AI8_IDLE;
			break;
		default:
			printk("%s def: %s\n",__FUNCTION__,PAA_MSG_STR_UNSUPPORTED);
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			DR_SET_INFO(_buf);
			process = PROC_AI8_IDLE;
			break;
	}
};

//---------------------------------------------------------------
//	AI8-data-completion
//---------------------------------------------------------------
void ai8_data_cpl_fct (void * _data)	{
	struct tai8_drv  * 	 _drv	= (struct tai8_drv *) _data;		
	ai8_control * 		 _ct		=  _drv->ct;
	unsigned char		 _typ;
	unsigned char 		 _len;
	unsigned char 		 _buf[PAA_BUFFER_SIZE];
	static u32 			 _stc_buf[DEV_MCT_PAA_AI8_IN_MAX];
	u8					 _ch;
	u32					 _val;

	/// FINAL MESSAGE! 
	if(atomic_read( &_ct->msg_stop_job ) ) {
// HACK 29.10.10		printk("msg_stop_job!\n");
		DR_SET_STATE(e_shutdown);		
		complete( &_ct->msg_stop_quit ); // final msg call, shutdown
		return;
	} 
	/// MASTER-NACHRICHT holen
	spin_lock_bh(&_ct->msg_lock);
	_typ = _ct->msg.rx_typ;
	_len = _ct->msg.rx_len;
	 memcpy(&_buf,&_ct->msg.rx_buf,_len);
	spin_unlock_bh(&_ct->msg_lock);

	/// Typ auswerten
	switch(_typ)	{
		case PAA_MSG_DATA:	/// DATA
			for(_ch = 0; _ch < DEV_MCT_PAA_AI8_IN_MAX ; _ch++) {
				if(memcmp(&_buf[_ch*4],&_stc_buf[_ch], 4)) {
					memcpy(&_stc_buf[_ch],&_buf[_ch*4], 4);
					_val = ntohl(_stc_buf[_ch]);
					DEBUG(MCT_DEBUG_LEVEL3,"%s in[%i]:0x%08x\n",__FUNCTION__,_ch,_val);
					DR_SET_INPUT(_ch, _val); /// EXPORT INPUT-WERTE	
				}	
			}	
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			DR_SET_INFO(_buf);
			process = PROC_AI8_IDLE;
			break;
		default:
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			DR_SET_INFO(_buf);
			break;
	}
};

//---------------------------------------------------------------
//	AI8-byebye-completion
//---------------------------------------------------------------
void ai8_byebye_cpl_fct (void * _data)	{
	struct tai8_drv  * 	 _drv	= (struct tai8_drv *) _data;		
	ai8_control * 		 _ct	=  _drv->ct;
	unsigned char		 _typ;
	unsigned char 		 _len;
	unsigned char 		 _buf[PAA_BUFFER_SIZE];	
	
	/// MASTER-NACHRICHT holen
	spin_lock_bh(&_ct->msg_lock);
	_typ = _ct->msg.rx_typ;
	_len = _ct->msg.rx_len;
	 memcpy(&_buf,&_ct->msg.rx_buf,_len);
	spin_unlock_bh(&_ct->msg_lock);
	/// Typ auswerten
	switch(_typ)	{
		case PAA_MSG_BYEBYE:	/// BYEBYE
		case PAA_MSG_ERROR:		/// ABBRUCH
			DR_SET_INFO(_buf);
			process = PROC_AI8_IDLE;
			break;
		default:
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			DR_SET_INFO(_buf);
			break;
	}
	/// BYE BYE MESSAGE! 
	DR_SET_STATE(e_byebye);		
	complete( &_ct->msg_byebye_quit ); // byebye msg call, shutdown
};

//---------------------------------------------------------------
//	AI8-Timer
//---------------------------------------------------------------
static void ai8_timer_function(unsigned long _data) {
	struct tai8_drv  * 	_drv	= (struct tai8_drv *) _data;		
	ai8_control * 		_ct		= _drv->ct;
	enum e_state 		_state;
	u8					_i = 0;
	u8					_cfg = 0;

	_ct->timer.expires = jiffies + TIMER_AI8_GRANULARITY;

	if(atomic_read( &_ct->timer_stop_job ) ) {
		complete( &_ct->timer_stop_quit ); 			// final spi timer call
	} else {
		// ***************************
		// DRIVER-STATE 	
		// ***************************	
		DR_GET_STATE(_state);						// driver-status
		switch(_state)	{
			case e_stop: 							// Stoppen!
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver stopped!\n", __FILE__,__FUNCTION__);
				DR_SET_STATE(e_idle); 				// Idle-Mode
				break;				
			case e_start:							// Starten!			
			case e_reset:	
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver started!\n", __FILE__,__FUNCTION__);
					process = PROC_AI8_INFO;
					DR_SET_STATE(e_running);
				break;	
			case e_running:
				// PAA-Message
				switch(process) {
					case PROC_AI8_INFO:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_INFO;
						_ct->msg.tx_len = strlen(ai8_drv_name)+1;
						strcpy(_ct->tx_buf, ai8_drv_name); 
						_ct->msg.complete = ai8_info_cpl_fct;	
						_ct->msg.context = (void*) _data;			// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;

					case PROC_AI8_CONFIG:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_CONFIG;
						_ct->msg.tx_len = DEV_MCT_PAA_AI8_CFG_CFG;
						for(_i=0;_i < DEV_MCT_PAA_AI8_IN_MAX; _i++)	{
							DR_GET_TYPE(_i,_cfg);					
							_ct->tx_buf[_i] = _cfg;	
						}
						_ct->msg.complete = ai8_config_cpl_fct;	
						_ct->msg.context = (void*) _data;	// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;

					case PROC_AI8_STATE:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_STATE;
						_ct->msg.tx_len = 0;
						_ct->msg.complete = ai8_config_cpl_fct;	
						_ct->msg.context = (void*) _data;	// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;	

					case PROC_AI8_DATA:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_DATA;
						_ct->msg.tx_len = 1;
						_ct->msg.complete = ai8_data_cpl_fct;	
						_ct->msg.context = (void*) _data;	// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;
					default:
						 printk("PROC_AI8_UNKNONW!\n");
					case PROC_AI8_IDLE:
						DR_SET_STATE(e_stop);
						break;
				}
				break;
				
			case e_shutdown:
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver shutdown!\n", __FILE__,__FUNCTION__);
					paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
					_ct->msg.tx_typ = PAA_MSG_BYEBYE;
					_ct->msg.tx_len = 0;
					_ct->msg.complete = ai8_byebye_cpl_fct;	
					_ct->msg.context = (void*) _data;	// available to completion
					paa_async(_ct->paa, &_ct->msg);	
				break;

			case e_byebye:
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver byebye!\n", __FILE__,__FUNCTION__);
				break;

			default:		// e_idle e_error 
				break;	
		};
	add_timer( &_ct->timer); // Timer aufziehen
	} // end else
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SECTION: PAA-SLAVE-Driver 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int slave_probe(struct paa_device *paa)	{	
	ai8_control * ct;
	trace_call_dev(&paa->dev);
	ct = kzalloc(sizeof(ai8_control), GFP_ATOMIC);
	if(ct==NULL) {						// kein Speicher
//		kfree(ct);
		printk("kzalloc faild!\n");
		dev_err(&paa->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): PAA slot_select:%d slot_mode:%d Alias:%s \n",__FILE__, __FUNCTION__,  paa->slot_select, paa->slot_mode, paa->modalias);
	
	/// LINK: paa->dev to ai8_control and backward
	dev_set_drvdata(&paa->dev,ct);	 			// bereich merken
	ct->paa = paa;	
	
	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);		// completion  object
	init_timer(&ct->timer);						// timer-initialisieren

	/// MSG-STO PQUEUE 
	init_completion(&ct->msg_stop_quit);		// stop-completion object
	init_completion(&ct->msg_byebye_quit);		// byebye-completion object
	
	/// LINK
	ai8_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &ai8_O_Mct;
	ct->timer.function = ai8_timer_function;
	ct->timer.expires = jiffies + TIMER_AI8_GRANULARITY;
	add_timer( &ct->timer);						// timer-start
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 

#ifdef MODULE
static int slave_remove(struct paa_device *paa)	{	
	ai8_control *ct = dev_get_drvdata(&paa->dev);	
	trace_call_dev(&paa->dev);
	
	/// AI8-MESSAGE-STOP
	atomic_set( &ct->msg_stop_job, 1);	
	// Message nicht mehr einhängen max. 3 Sekunden warten
	wait_for_completion_timeout(&ct->msg_stop_quit, HZ*3); 	
	
	/// AI8-MESSAGE-BYEBYE 
	// Letzte Message an den PAA_Master senden.
	// Der PAA_Treiber verabschiedet sich nun und das
	// LBUS-Gerät wird in den CFG-PING Modus zurückgesetzt. 
	// Dadurch kann bei einem neuen Treiberstart eine neue
	// Konfiguration eingestellt werden!
	atomic_set( &ct->msg_byebye_job, 1);	
	// Auf Antwort warten max. 3 Sekunden warten	
	wait_for_completion_timeout(&ct->msg_byebye_quit, HZ*3); 	

	/// AI8-TIMER-STOP
	atomic_set( &ct->timer_stop_job, 1);		// Timer nicht mehr einhängen
	wait_for_completion(&ct->timer_stop_quit);	
	
	if( timer_pending( &ct->timer ) ) {
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer active!\n", __FILE__,__FUNCTION__);
	}
	if( del_timer_sync( &ct->timer ) )	{
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer not active!\n", __FILE__,__FUNCTION__);
	}
	else	{
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer stop!\n", __FILE__,__FUNCTION__);
	}	
	kfree(ct);	
	return 0;
};
#endif

static struct paa_driver slave_drv = {
	.driver = {
		.name = "slave_drv new???",	// Wird noch überschrieben!
		.bus = &paa_bus_type,
		.owner = THIS_MODULE,
	},
	.probe 	= slave_probe,
	.remove = __exit_p(slave_remove),
};

static struct paa_device * slave_dev;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: AI8-DRIVER (platform) with 4 DEVICEs  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static struct platform_driver ai8_drv = {
		.driver = {		
			.name = "ai8_drv new???",		// Wert wird erst später gesetzt 
			.bus = &platform_bus_type,		// ... /sys/bus/platform/drivers/DRIVER_NAME
	},										// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};

// INPUT-DEVICE-PTR (platform-device)
static struct platform_device * inputs[DEV_MCT_PAA_AI8_IN_MAX]; 

static void ai_dev_release(struct device * dev)	{
	complete(&device_is_free);
}

static int  ai_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;
	int idx;

	for (idx = 0; idx< DEV_MCT_PAA_AI8_IN_MAX; idx++)	{	
		inputs[idx] = platform_device_alloc(ai8_dev_in_name,idx);
		if(inputs[idx]) {
			inputs[idx]->dev.release = ai_dev_release;		// Release-Function
			platform_device_add(inputs[idx]);
			// dem AI-Platform Device die Datenstruktur mitgeben
			platform_set_drvdata(inputs[idx], &ai8_O_Inputs[idx]);
			inputs[idx]->dev.driver = &drv->driver;
			lnk_err = device_bind_driver(&inputs[idx]->dev);			
			ai8_sysfs_dev_add(&inputs[idx]->dev);			// AI-Devices-SYSFS erzeugen	
		}
		else	{
			// Resource-Meldung!
			printk(KERN_ERR "%s %s(): %s %s.%d error!\n",__FILE__, __FUNCTION__,"Resource ",ai8_dev_in_name, idx);
			ret = -ENOMEM;
		}
		if(ret)	{
			if(inputs[idx]) {	
				// AI-Platform-Device Datenzeiger aufheben
				platform_set_drvdata(inputs[idx],NULL);		
				platform_device_unregister(inputs[idx]);	// AI-Platform-Device entfernen
				ai8_sysfs_dev_del(&inputs[idx]->dev);		// AI-Devices-SYSFS entfernen
			}
		}	
	}	// end for DEVICE_IN_MAX
	return ret;
}
static void  ai_devs_destroy(void)	{
	//	trace_call();
	int idx;
	for (idx = 0; idx< DEV_MCT_PAA_AI8_IN_MAX; idx++)	{
		if(inputs[idx])	{	
			// AI-Platform-Device Datenzeiger aufheben
			platform_set_drvdata(inputs[idx],NULL);	
			platform_device_unregister(inputs[idx]);		// AI-Platform-Device entfernen
			ai8_sysfs_dev_del(&inputs[idx]->dev);			// AI-Devices-SYSFS entfernen 
		}
	}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init ai8_init(void)	{
	int ret = 0;
	struct paa_master * master;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);

	/// ÜBERLADEN!
	strcpy((char*)(ai8_drv.driver.name), ai8_drv_name);
	strcpy((char*)(chip_ai8.modalias), ai8_drv_name);
	chip_ai8.slot_select = ai8_slot;
	strcpy((char*)(slave_drv.driver.name), ai8_drv_name);

	ret = platform_driver_register(&ai8_drv);		// AI8-Platformtreiber anmelden ...
	if(ret)	{ 										// Error-0
		printk("Err!platform_driver_register\n");		
		goto RESOURCE_RWND_NULL;	
	}
	ai8_sysfs_drv_add(&ai8_drv.driver);				// AI8-Driver-SYSFS anlegen
	ret = 	ai_devs_build(&ai8_drv);				// AI-Platform-Device bauen
	if(ret) {										// Error-1
		printk("Err!ai_devs_build\n");				
		goto RESOURCE_RWND_ERROR_1;
	}
	master =paa_busnum_to_master(PAA_BUS_NUM);		// Master ermitteln
	if(!master) {									// Error-2
		printk("Err!paa_busnum_to_master\n");
		goto RESOURCE_RWND_ERROR_2;
	}
	slave_dev = paa_new_device(master,&chip_ai8);	// Geräteslot erzeugen
	if(!slave_dev) {								// EXPAND: paa_board_info
		printk("Err!paa_new_device\n");
		goto RESOURCE_RWND_ERROR_2;					// Error-2
	}
	ai8_objects_create();							// OBJECTs initialisieren
	if(strlen(ai8_json) != 0)						// modprobe driver json={....}
		ai8_objects_json(ai8_json);					// ...JSON-Parameter
	ai8_objects_generic();							// ...Classic-Parameter		

#ifdef	MCT_PAA_AI8_DEV_IF_MBUS
	mbus_build(&ai8_drv,chip_ai8.slot_select);		/// MBUS, Multi-Instanz
#endif
#ifdef	MCT_PAA_AI8_DEV_IF_MODBUS
	modbus_build(&ai8_drv,chip_ai8.slot_select);	/// MODBUS,Multi-Instanz
#endif

	ret = paa_register_driver(&slave_drv);			// SLAVE-PAA-DRIVER anmelden ...
	if(ret)	{										// Error
		goto RESOURCE_RWND_DEVICE;
	}												
	goto RESOURCE_OKAY;

RESOURCE_RWND_DEVICE:								// ***** REWIND on ERROR ************* 
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_AI8_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_AI8_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
RESOURCE_RWND_ERROR_2:
	ai_devs_destroy();								// AI-Platform-Devices entfernen!
RESOURCE_RWND_ERROR_1:								// FEHLER: Platform-Devices bauen
	ai8_sysfs_drv_del(&ai8_drv.driver);				// AI8-Driver-SYSFS entfernen
	platform_driver_unregister(&ai8_drv);			// AI8-Platform-Driver abmelden
RESOURCE_RWND_NULL:									// FEHLER: AI8-Platformtreiber anmelden
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource",ai8_drv_name);
		ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit ai8_exit(void)	{
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);		
	paa_unregister_driver(&slave_drv);				// SLAVE-DRV abmelden
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_AI8_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_AI8_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	ai_devs_destroy();								// AI-Platform-Device entfernen	
	ai8_sysfs_drv_del(&ai8_drv.driver);				// AI8-Driver-SYSFS entfernen
	platform_driver_unregister(&ai8_drv);			// AI8-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEV_MCT_PAA_AI8_IN_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};

module_init( ai8_init );
module_exit( ai8_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("(analog in 8 channel) slave paa_driver");

