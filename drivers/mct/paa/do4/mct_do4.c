/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		26.08.2011

	Description:	paa_do4 Client-Driver

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"	// debugging
#include "../mct_paa_msg.h"		// paa-messages
#include "mct_do4_objs.h"		// objects
#include "mct_do4_sysfs.h"		// sysfs-support
#include "mct_do4.h"

#ifdef MCT_PAA_DO4_DEV_IF_MBUS		
	#include "mct_do4_mbus.h"	/// MBUS	
#endif

#ifdef MCT_PAA_DO4_DEV_IF_MODBUS		
	#include "mct_do4_modbus.h"	/// MODBUS	
#endif


static DECLARE_COMPLETION( device_is_free );

// process-stati
#define PROC_DO4_INFO		0
#define PROC_DO4_DATA		1
#define PROC_DO4_IDLE		2
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

#define DR_GET_OUT_VALUE_OUT(val)	{	\
				spin_lock_bh(&_drv->Op_Outputs->Op_Value->lock);	\
				val = _drv->Op_Outputs->Op_Value->P_ValueOut.value;	\
				spin_unlock_bh(&_drv->Op_Outputs->Op_Value->lock);	}

#define DR_GET_OUT_VALUE_IN(val)	{	\
				spin_lock_bh(&_drv->Op_Outputs->Op_Value->lock);	\
				val = _drv->Op_Outputs->Op_Value->P_ValueIn.value;	\
				spin_unlock_bh(&_drv->Op_Outputs->Op_Value->lock);	}

#define DR_SET_OUT_VALUE_IN(val)	{	\
				spin_lock_bh(&_drv->Op_Outputs->Op_Value->lock);	\
				_drv->Op_Outputs->Op_Value->P_ValueIn.value = val;	\
				spin_unlock_bh(&_drv->Op_Outputs->Op_Value->lock);	}

#define DR_GET_MASK(val)	{	\
				spin_lock_bh(&_drv->Op_Outputs->Op_Mask->lock);	\
				val=_drv->Op_Outputs->Op_Mask->P_Value.value;	\
				spin_unlock_bh(&_drv->Op_Outputs->Op_Mask->lock);	}

#define DR_SET_MASK(val)	{	\
				spin_lock_bh(&_drv->Op_Outputs->Op_Mask->lock);	\
				_drv->Op_Outputs->Op_Mask->P_Value.value = val;	\
				spin_unlock_bh(&_drv->Op_Outputs->Op_Mask->lock);	}

static struct paa_board_info chip_do4 = {
	.modalias 		= "chip_do4 new???",	// Wird noch überschrieben!
	.bus_num 		= PAA_BUS_NUM,			
	.slot_select 	= 0,					// Wird noch überschrieben!
	.slot_mode 		= 100,					// z.Z. keine Verwendung 
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DO4-DRIVER (on the top of PAA-Core)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//---------------------------------------------------------------
//	DO4-info-completion
//---------------------------------------------------------------
void do4_info_cpl_fct (void * _data)	{
	struct tdo4_drv  * _drv	= (struct tdo4_drv *) _data;		
	do4_control * 		_ct		= (void *) _drv->ct;
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
			process = PROC_DO4_DATA;
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__, _buf,_len);
			process = PROC_DO4_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__, PAA_MSG_STR_UNSUPPORTED);
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			process = PROC_DO4_IDLE;
			break;
	}
	DR_SET_INFO(_buf);
}

//---------------------------------------------------------------
//	DO4-data-completion
//---------------------------------------------------------------
void do4_data_cpl_fct (void * _data)	{
	struct tdo4_drv  * 	_drv	= (struct tdo4_drv *) _data;		
	do4_control * 		_ct		=  _drv->ct;
	unsigned char		_typ;
	unsigned char 		_len;
	unsigned char 		_buf[PAA_BUFFER_SIZE];
	unsigned char		_out;
	unsigned char		_msk;

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
			_buf[0] &= SIZE_MAX_4bit;		// Filter: Output 
			_buf[1] &= SIZE_MAX_4bit;		// Filter: Ouput-Maske

			DR_GET_OUT_VALUE_IN(_out);		// alter Feedback Output
			DR_GET_MASK(_msk);				// alte Maske

			if((_buf[0] != _out) || (_buf[1] != _msk)) {
				DR_SET_OUT_VALUE_IN(_buf[0]); 	// neuer Wert
				DR_SET_MASK(_buf[1]);			// neue Maske
				DEBUG(MCT_DEBUG_LEVEL3,"%s _fb_out:%d msk:%d len:%d\n",__FUNCTION__,_buf[0],_buf[1], _len);
			}
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__,_buf,_len);
			DR_SET_INFO(_buf);
			process = PROC_DO4_IDLE;	
			break;
		default:
			printk("%s %s\n",__FUNCTION__,PAA_MSG_STR_UNSUPPORTED);
			memcpy(_buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			DR_SET_INFO(_buf);
			break;
	}
};

//---------------------------------------------------------------
//	DO4-Timer
//---------------------------------------------------------------
static void do4_timer_function(unsigned long _data) {
	struct tdo4_drv  * 	_drv	= (struct tdo4_drv *) _data;		
	do4_control * 		_ct		= _drv->ct;
	enum e_state 		_state;
	u8					_out;

	_ct->timer.expires = jiffies + TIMER_DO4_GRANULARITY;

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
					process = PROC_DO4_INFO;
					DR_SET_STATE(e_running); 		
				break;	
			case e_running:
				// PAA-Message
				switch(process) {
					case PROC_DO4_INFO:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_INFO;
						_ct->msg.tx_len = strlen(do4_drv_name)+1;
						strcpy(_ct->tx_buf, do4_drv_name); 
						_ct->msg.complete = do4_info_cpl_fct;	
						_ct->msg.context = (void*) _data;	// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;
					case PROC_DO4_DATA:
						paa_message_init(&_ct->msg,&_ct->msg_lock, _ct->tx_buf, _ct->rx_buf);
						_ct->msg.tx_typ = PAA_MSG_DATA;
						_ct->msg.tx_len = 1;
						DR_GET_OUT_VALUE_OUT(_out);	
						_ct->tx_buf[0] = _out;	
						_ct->msg.complete = do4_data_cpl_fct;	
						_ct->msg.context = (void*) _data;	// available to completion
						paa_async(_ct->paa, &_ct->msg);	
						break;
					default:
						printk("PROC_DO4_UNKNONW!\n");
					case PROC_DO4_IDLE:
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
	do4_control * ct;
	trace_call_dev(&paa->dev);
	ct = kzalloc(sizeof(do4_control), GFP_ATOMIC);
	if(ct==NULL) {						// kein Speicher
//		kfree(ct);
		printk("kzalloc faild!\n");
		dev_err(&paa->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): PAA slot_select:%d slot_mode:%d Alias:%s \n",__FILE__, __FUNCTION__,  paa->slot_select, paa->slot_mode, paa->modalias);
	
	/// LINK: paa->dev to do4_control and backward
	dev_set_drvdata(&paa->dev,ct);	 			// bereich merken
	ct->paa = paa;	
	
	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);		// completion  object
	init_timer(&ct->timer);						// timer-initialisieren

	/// MSG-QUEUE 
	init_completion(&ct->msg_stop_quit);		// completion  object
	
	/// LINK
	do4_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &do4_O_Mct;
	ct->timer.function = do4_timer_function;
	ct->timer.expires = jiffies + TIMER_DO4_GRANULARITY;
	add_timer( &ct->timer);						// timer-start
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 

#ifdef MODULE
static int slave_remove(struct paa_device *paa)	{	
	do4_control *ct = dev_get_drvdata(&paa->dev);	
	trace_call_dev(&paa->dev);
	
	/// DO4-MESSAGE-STOP
	atomic_set( &ct->msg_stop_job, 1);		
	// Message nicht mehr einhängen max. 1 Sekunde warten
	wait_for_completion_timeout(&ct->msg_stop_quit, HZ*3);	

	/// DO4-TIMER-STOP
	atomic_set( &ct->timer_stop_job, 1);		// Timer nicht mehr einhängen
	wait_for_completion(&ct->timer_stop_quit);	
	
	if( timer_pending( &ct->timer ) ) {
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer  active!\n", __FILE__,__FUNCTION__);
	}
	if( del_timer_sync( &ct->timer ) )	{
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer  not active!\n", __FILE__,__FUNCTION__);
	}
	else	{
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer  stop!\n", __FILE__,__FUNCTION__);
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
///	SECTION: DO4-DRIVER (platform) with 1 DEVICE  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static struct platform_driver do4_drv = {
		.driver = {		
				.name = "do4_drv new???",	// Wert wird erst später gesetzt 
			.bus = &platform_bus_type,		// ... /sys/bus/platform/drivers/DRIVER_NAME
	},										// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};

// OUTPUT-DEVICE-PTR (platform-device)
static struct platform_device * outputs; 

static void do_dev_release(struct device * dev)	{
	complete(&device_is_free);
}

static int  do_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;

	outputs  = platform_device_alloc(do4_dev_out_name,0);
	if(outputs) {
		outputs->dev.release = do_dev_release;		// Release-Function
		platform_device_add(outputs);
		platform_set_drvdata(outputs, &do4_O_Outputs);// dem DO-Platform Device die Datenstruktur mitgeben
		outputs->dev.driver = &drv->driver;
		lnk_err = device_bind_driver(&outputs->dev);			
		do4_sysfs_dev_add(&outputs->dev);			// DO-Devices-SYSFS erzeugen	
	}
	else	{
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",do4_dev_out_name);
		ret = -ENOMEM;
	}
	if(ret)	{
		if( outputs) {	
			platform_set_drvdata(outputs,NULL);		// DO-Platform-Device Datenzeiger aufheben
			platform_device_unregister(outputs);	// DO-Platform-Device entfernen
			do4_sysfs_dev_del(&outputs->dev);		// DO-Devices-SYSFS entfernen
		}
	}
	return ret;
}
static void  do_devs_destroy(void)	{
//	trace_call();
	if(outputs)	{	
		platform_set_drvdata(outputs,NULL);			// DO-Platform-Device Datenzeiger aufheben			
		platform_device_unregister(outputs);		// DO-Platform-Device entfernen
		do4_sysfs_dev_del(&outputs->dev);			// DO-Devices-SYSFS entfernen 
	}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init do4_init(void)	{
	int ret = 0;
	struct paa_master * master;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	
	/// ÜBERLADEN!
	strcpy((char*)(do4_drv.driver.name), do4_drv_name);
	strcpy((char*)(chip_do4.modalias), do4_drv_name);
	chip_do4.slot_select = do4_slot;
	strcpy((char*)(slave_drv.driver.name), do4_drv_name);

	ret = platform_driver_register(&do4_drv);		// DO4-Platformtreiber anmelden ...
	if(ret)	{ 										// Error-Message 0
		goto RESOURCE_RWND_NULL;	
	}
	do4_sysfs_drv_add(&do4_drv.driver);				// DO4-Driver-SYSFS anlegen
	ret = 	do_devs_build(&do4_drv);				// DO-Platform-Device bauen
	if(ret) {										// Error-1
		goto RESOURCE_RWND_ERROR_1;
	}
	master =paa_busnum_to_master(PAA_BUS_NUM);		// Master ermitteln
	if(!master) {									// Error-2
		goto RESOURCE_RWND_ERROR_2;
	}
	slave_dev = paa_new_device(master,&chip_do4);	// Geräteslot erzeugen
	if(!slave_dev) {								// EXPAND: paa_board_info
		goto RESOURCE_RWND_ERROR_2;					// Error-2
	}
	do4_objects_create();							// OBJECTs initialisieren
	if(strlen(do4_json) != 0)						// modprobe driver json={....}
		do4_objects_json(do4_json);					// ...JSON-Parameter
	do4_objects_generic();							// ...Classic-Parameter		

#ifdef	MCT_PAA_DO4_DEV_IF_MBUS
	mbus_build(&do4_drv,chip_do4.slot_select);		/// MBUS, Multi-Instanz
#endif
#ifdef	MCT_PAA_DO4_DEV_IF_MODBUS
	modbus_build(&do4_drv,chip_do4.slot_select);	/// MODBUS,Multi-Instanz
#endif

	ret = paa_register_driver(&slave_drv);			// SLAVE-PAA-DRIVER anmelden ...
	if(ret)	{										// Error
		goto RESOURCE_RWND_DEVICE;
	}												
	goto RESOURCE_OKAY;

RESOURCE_RWND_DEVICE:								// ***** REWIND on ERROR ************* 
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_DO4_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_DO4_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
RESOURCE_RWND_ERROR_2:
	do_devs_destroy();								// DO-Platform-Devices entfernen!
RESOURCE_RWND_ERROR_1:								// FEHLER: Platform-Devices bauen
	do4_sysfs_drv_del(&do4_drv.driver);				// DO4-Driver-SYSFS entfernen
	platform_driver_unregister(&do4_drv);			// DO4-Platform-Driver abmelden
RESOURCE_RWND_NULL:									// FEHLER: DO4-Platformtreiber anmelden
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource", do4_drv_name);
		ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit do4_exit(void)	{
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);		
	paa_unregister_driver(&slave_drv);				// SLAVE-DRV abmelden
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_DO4_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_DO4_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	do_devs_destroy();								// DO-Platform-Device entfernen	
	do4_sysfs_drv_del(&do4_drv.driver);				// DO4-Driver-SYSFS entfernen
	platform_driver_unregister(&do4_drv);			// DO4-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEV_MCT_PAA_DO4_OUT_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};

module_init( do4_init );
module_exit( do4_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("(digital out 4 channel) slave paa_driver");

