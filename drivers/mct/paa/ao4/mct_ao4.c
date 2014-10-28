/*********************************************************************************

 	Copyright MC-Technology GmbH 2010,2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$	

	Description:	paa_ao4 Client-Driver

	 Analoger-Output DRIVER (Gerät)  mit 4 DEVICES (Output Kanälen, je 16 bit
	 davon 10bit genutzt!)

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"	// debugging
#include "../mct_paa_msg.h"		// paa-messages
#include "mct_ao4_objs.h"		// objects
#include "mct_ao4_sysfs.h"		// sysfs-support
#include "mct_ao4.h"

#ifdef MCT_PAA_AO4_DEV_IF_MBUS		
	#include "mct_ao4_mbus.h"	/// MBUS	
#endif

#ifdef MCT_PAA_AO4_DEV_IF_MODBUS		
	#include "mct_ao4_modbus.h"	/// MODBUS	
#endif

static DECLARE_COMPLETION( device_is_free );

// process-stati
#define PROC_AO4_INFO		0
#define PROC_AO4_DATA		1
#define PROC_AO4_IDLE		2
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

#define DR_GET_OUPUT(chan,val)	{	\
			spin_lock_bh(&_drv->Op_Outputs[chan]->Op_Value->lock);	\
			val = _drv->Op_Outputs[chan]->Op_Value->P_Value.value;	\
			spin_unlock_bh(&_drv->Op_Outputs[chan]->Op_Value->lock);	}

#define DR_SET_OUPUT(chan,val)	{	\
			spin_lock_bh(&_drv->Op_Outputs[chan]->Op_Value->lock);	\
			_drv->Op_Outputs[chan]->Op_Value->P_Value.value = val;	\
			spin_unlock_bh(&_drv->Op_Outputs[chan]->Op_Value->lock);	}

static struct paa_board_info chip_ao4 = {
	.modalias 		= "chip_ao4 new???",	// Wird noch überschrieben!
	.bus_num 		= PAA_BUS_NUM,
	.slot_select 	= 0,					// Wird noch überschrieben!
	.slot_mode 		= 0,					// z.Z. keine Verwendung
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: AO4-DRIVER (on the top of PAA-Core)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//---------------------------------------------------------------
//	AO4-info-completion
//---------------------------------------------------------------
void ao4_info_cpl_fct (void * _data)	{
	struct tao4_drv  * _drv	= (struct tao4_drv *) _data;		
	ao4_control * 		_ct		= (void *) _drv->ct;
	unsigned char		_typ;
	unsigned char 		_len;
	unsigned char 		_buf[PAA_BUFFER_SIZE];

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
		case PAA_MSG_INFO:	/// INFO
			printk("%s %s len: %d\n",__FUNCTION__, _buf,_len);
			process = PROC_AO4_DATA;
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__, _buf,_len);
			process = PROC_AO4_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__, PAA_MSG_STR_UNSUPPORTED);
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			process = PROC_AO4_IDLE;
			break;
	}
	DR_SET_INFO(_buf);
}

//---------------------------------------------------------------
//	AO4-data-completion
//---------------------------------------------------------------
void ao4_data_cpl_fct (void * _data)	{
	struct tao4_drv  * 	 _drv	= (struct tao4_drv *) _data;		
	ao4_control * 		 _ct		=  _drv->ct;
	unsigned char		 _typ;
	unsigned char 		 _len;
	unsigned char 		 _buf[PAA_BUFFER_SIZE];
	static unsigned char _stc_out[DEV_MCT_PAA_AO4_CFG_OUT];

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
			if(memcmp(&_buf,&_stc_out,DEV_MCT_PAA_AO4_CFG_OUT) != 0) { 
				DEBUG(MCT_DEBUG_LEVEL3,"%s %x%x %x%x %x%x %x%x len: %d\n",__FUNCTION__,\
				_buf[0],_buf[1],_buf[2],_buf[3],_buf[4],_buf[5],_buf[6],_buf[7], _len);
				memcpy(&_stc_out,&_buf,DEV_MCT_PAA_AO4_CFG_OUT);
			}	
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			DR_SET_INFO(_buf);
			process = PROC_AO4_IDLE;
			break;
		default:
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			DR_SET_INFO(_buf);
			break;
	}
};

//---------------------------------------------------------------
//	AO4-Timer
//---------------------------------------------------------------
static void ao4_timer_function(unsigned long _data) {
	struct tao4_drv  * 	_drv	= (struct tao4_drv *) _data;		
	ao4_control * 		_ct		= _drv->ct;
	enum e_state 		_state;
	u16					_out16 = 0;
	u8					_i = 0;

	_ct->timer.expires = jiffies + TIMER_AO4_GRANULARITY;

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
					process = PROC_AO4_INFO;
					DR_SET_STATE(e_running); 		
				break;	
			case e_running:
				// PAA-Message
				switch(process) {
					case PROC_AO4_INFO:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_INFO;
						_ct->msg.tx_len = strlen(ao4_drv_name)+1;					
						strcpy(_ct->tx_buf, ao4_drv_name); 
						_ct->msg.complete = ao4_info_cpl_fct;	
						_ct->msg.context = (void*) _data;			// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;
					case PROC_AO4_DATA:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_DATA;
						_ct->msg.tx_len = DEV_MCT_PAA_AO4_CFG_OUT;
						/// OUTPUTs 
						for(_i=0;_i < DEV_MCT_PAA_AO4_OUT_MAX; _i++)	{
							DR_GET_OUPUT(_i,_out16 );					
								// HACK Untere Grenze 
							if(_out16 < 0)	{
								_out16 = 0;	
								DR_SET_OUPUT(_i,_out16);
							}	// HACK Obere Grenze
							else if (_out16 > SIZE_MAX_10bit)	{
		 						_out16 = SIZE_MAX_10bit;
								DR_SET_OUPUT(_i,_out16);
							}
							_ct->tx_buf[_i*2+1] = _out16;	
							_ct->tx_buf[_i*2]   = _out16 >> 8;	
						}
						_ct->msg.complete = ao4_data_cpl_fct;	
						_ct->msg.context = (void*) _data;	// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;
					default:
						printk("PROC_AO4_UNKNONW!\n");
					case PROC_AO4_IDLE:
						DR_SET_STATE(e_stop);
						break;
				}
				break;
				
			case e_shutdown:
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver shutdown!\n", __FILE__,__FUNCTION__);
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
	ao4_control * ct;
	trace_call_dev(&paa->dev);
	ct = kzalloc(sizeof(ao4_control), GFP_ATOMIC);
	if(ct==NULL) {						// kein Speicher
//		kfree(ct);
		printk("kzalloc faild!\n");
		dev_err(&paa->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): PAA slot_select:%d slot_mode:%d Alias:%s \n",__FILE__, __FUNCTION__,  paa->slot_select, paa->slot_mode, paa->modalias);
	
	/// LINK: paa->dev to ao4_control and backward
	dev_set_drvdata(&paa->dev,ct);	 			// bereich merken
	ct->paa = paa;	
	
	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);		// completion  object
	init_timer(&ct->timer);						// timer-initialisieren

	/// MSG-QUEUE 
	init_completion(&ct->msg_stop_quit);		// completion  object
	
	/// LINK
	ao4_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &ao4_O_Mct;
	ct->timer.function = ao4_timer_function;
	ct->timer.expires = jiffies + TIMER_AO4_GRANULARITY;
	add_timer( &ct->timer);						// timer-start
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 

#ifdef MODULE
static int slave_remove(struct paa_device *paa)	{	
	ao4_control *ct = dev_get_drvdata(&paa->dev);	
	trace_call_dev(&paa->dev);
	
	/// AO4-MESSAGE-STOP
	atomic_set( &ct->msg_stop_job, 1);	
	// Message nicht mehr einhängen max. 1 Sekunde warten
	wait_for_completion_timeout(&ct->msg_stop_quit, HZ*3); 	
	
	/// AO4-TIMER-STOP
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
///	SECTION: AO4-DRIVER (platform) with 4 DEVICEs  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static struct platform_driver ao4_drv = {
		.driver = {		
			.name = "ao4_drv new???",		// Wert wird erst später gesetzt 
			.bus = &platform_bus_type,		// ... /sys/bus/platform/drivers/DRIVER_NAME
	},										// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};

// OUTPUT-DEVICE-PTR (platform-device)
static struct platform_device * outputs[DEV_MCT_PAA_AO4_OUT_MAX]; 

static void ao_dev_release(struct device * dev)	{
	complete(&device_is_free);
}

static int  ao_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;
	int idx;

	for (idx = 0; idx< DEV_MCT_PAA_AO4_OUT_MAX; idx++)	{	
		outputs[idx] = platform_device_alloc(ao4_dev_out_name,idx);
		if(outputs[idx]) {
			outputs[idx]->dev.release = ao_dev_release;		// Release-Function
			platform_device_add(outputs[idx]);
			// dem AO-Platform Device die Datenstruktur mitgeben
			platform_set_drvdata(outputs[idx], &ao4_O_Outputs[idx]);
			outputs[idx]->dev.driver = &drv->driver;
			lnk_err = device_bind_driver(&outputs[idx]->dev);			
			ao4_sysfs_dev_add(&outputs[idx]->dev);			// AO-Devices-SYSFS erzeugen	
		}
		else	{
			// Resource-Meldung!
			printk(KERN_ERR "%s %s(): %s %s.%d error!\n",__FILE__, __FUNCTION__,"Resource ",ao4_dev_out_name, idx);
			ret = -ENOMEM;
		}
		if(ret)	{
			if(outputs[idx]) {	
				// AO-Platform-Device Datenzeiger aufheben
				platform_set_drvdata(outputs[idx],NULL);		
				platform_device_unregister(outputs[idx]);	// AO-Platform-Device entfernen
				ao4_sysfs_dev_del(&outputs[idx]->dev);		// AO-Devices-SYSFS entfernen
			}
		}	
	}	// end for DEVICE_OUT_MAX
	return ret;
}
static void  ao_devs_destroy(void)	{
	//	trace_call();
	int idx;
	for (idx = 0; idx< DEV_MCT_PAA_AO4_OUT_MAX; idx++)	{
		if(outputs[idx])	{	
			// AO-Platform-Device Datenzeiger aufheben
			platform_set_drvdata(outputs[idx],NULL);	
			platform_device_unregister(outputs[idx]);		// AO-Platform-Device entfernen
			ao4_sysfs_dev_del(&outputs[idx]->dev);			// AO-Devices-SYSFS entfernen 
		}
	}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init ao4_init(void)	{
	int ret = 0;
	struct paa_master * master;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	
	/// ÜBERLADEN!
	strcpy((char*)(ao4_drv.driver.name), ao4_drv_name);
	strcpy((char*)(chip_ao4.modalias), ao4_drv_name);
	chip_ao4.slot_select = ao4_slot;
	strcpy((char*)(slave_drv.driver.name), ao4_drv_name);

	ret = platform_driver_register(&ao4_drv);		// AO4-Platformtreiber anmelden ...
	if(ret)	{ 										// Error-Message 0
		printk("Err!platform_driver_register\n");		
		goto RESOURCE_RWND_NULL;	
	}
	ao4_sysfs_drv_add(&ao4_drv.driver);				// AO4-Driver-SYSFS anlegen
	ret = 	ao_devs_build(&ao4_drv);				// AO-Platform-Device bauen
	if(ret) {										// Error-1
		printk("Err!ao_devs_build\n");
		goto RESOURCE_RWND_ERROR_1;
	}
	master =paa_busnum_to_master(PAA_BUS_NUM);		// Master ermitteln
	if(!master) {									// Error-2
		printk("Err!paa_busnum_to_master\n");
		goto RESOURCE_RWND_ERROR_2;
	}
	slave_dev = paa_new_device(master,&chip_ao4);	// Geräteslot erzeugen
	if(!slave_dev) {								// EXPAND: paa_board_info
		printk("Err!paa_new_device\n");
		goto RESOURCE_RWND_ERROR_2;					// Error-2
	}
	ao4_objects_create();							// OBJECTs initialisieren
	if(strlen(ao4_json) != 0)						// modprobe driver json={....}
		ao4_objects_json(ao4_json);					// ...JSON-Parameter
	ao4_objects_generic();							// ...Classic-Parameter		

#ifdef	MCT_PAA_AO4_DEV_IF_MBUS
	mbus_build(&ao4_drv,chip_ao4.slot_select);		/// MBUS, Multi-Instanz
#endif
#ifdef	MCT_PAA_AO4_DEV_IF_MODBUS
	modbus_build(&ao4_drv,chip_ao4.slot_select);	/// MODBUS,Multi-Instanz
#endif

	ret = paa_register_driver(&slave_drv);			// SLAVE-PAA-DRIVER anmelden ...
	if(ret)	{										// Error
		goto RESOURCE_RWND_DEVICE;
	}												
	goto RESOURCE_OKAY;

RESOURCE_RWND_DEVICE:								// ***** REWIND on ERROR ************* 
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_AO4_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_AO4_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif

RESOURCE_RWND_ERROR_2:
	ao_devs_destroy();								// AO-Platform-Devices entfernen!
RESOURCE_RWND_ERROR_1:								// FEHLER: Platform-Devices bauen
	ao4_sysfs_drv_del(&ao4_drv.driver);				// AO4-Driver-SYSFS entfernen
	platform_driver_unregister(&ao4_drv);			// AO4-Platform-Driver abmelden
RESOURCE_RWND_NULL:									// FEHLER: AO4-Platformtreiber anmelden
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource",ao4_drv_name);
		ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit ao4_exit(void)	{
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);		
	paa_unregister_driver(&slave_drv);				// SLAVE-DRV abmelden
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_AO4_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_AO4_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	ao_devs_destroy();								// AO-Platform-Device entfernen	
	ao4_sysfs_drv_del(&ao4_drv.driver);				// AO4-Driver-SYSFS entfernen
	platform_driver_unregister(&ao4_drv);			// AO4-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEV_MCT_PAA_AO4_OUT_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};

module_init( ao4_init );
module_exit( ao4_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("(analog out 4 channel) slave paa_driver");


