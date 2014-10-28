/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

	Description:	paa_di10 Client-Driver

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"	// debugging
#include "../mct_paa_msg.h"		// paa-messages
#include "mct_di10_objs.h"		// objects
#include "mct_di10_sysfs.h"		// sysfs-support
#include "mct_di10.h"

#ifdef MCT_PAA_DI10_DEV_IF_MBUS		
	#include "mct_di10_mbus.h"		/// MBUS	
#endif

#ifdef MCT_PAA_DI10_DEV_IF_MODBUS		
	#include "mct_di10_modbus.h"	/// MODBUS	
#endif

static DECLARE_COMPLETION( device_is_free );
// process-stati
#define PROC_DI10_INFO		0
#define PROC_DI10_DATA		1
#define PROC_DI10_IDLE		2
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

#define DR_SET_VALUE(val)	{	\
				spin_lock_bh(&_drv->Op_Inputs->Op_Value->lock);	\
				_drv->Op_Inputs->Op_Value->P_Value.value = val;	\
				spin_unlock_bh(&_drv->Op_Inputs->Op_Value->lock);	}

static struct paa_board_info chip_di10 = {
	.modalias 		= "chip_di10 new???",
	.bus_num 		= PAA_BUS_NUM,
	.slot_select 	= 0,
	.slot_mode 		= 0,
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DI10-DRIVER (on the top of PAA-Core)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//HACK
unsigned char process;

//---------------------------------------------------------------
//	DI10-info-completion
//---------------------------------------------------------------
void di10_info_cpl_fct (void * data)	{
	OBJS_CREATE_DRV_PTR(data);	
	di10_control * ct = (void *) drv->ct;
	unsigned char	typ;
	unsigned char 	len;
	unsigned char 	buf[PAA_BUFFER_SIZE];

	/// FINAL MESSAGE! 
	if(atomic_read( &ct->msg_stop_job )) {
		OBJS_DRV_WR_STATE(drv,e_shutdown);		
		complete( &ct->msg_stop_quit ); // final msg call, shutdown
	return; 
	}
	/// MASTER-NACHRICHT holen
	spin_lock_bh(&ct->msg_lock);
	typ = ct->msg.rx_typ;
	len = ct->msg.rx_len;
	memcpy(&buf,&ct->msg.rx_buf,len);
	spin_unlock_bh(&ct->msg_lock);

	/// Typ auswerten
	switch(typ)	{
		case PAA_MSG_INFO:	/// INFO
			printk("%s %s len: %d\n",__FUNCTION__, buf,len);
			process = PROC_DI10_DATA;
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__, buf,len);
			process = PROC_DI10_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__, PAA_MSG_STR_UNSUPPORTED);
			memcpy(buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			process = PROC_DI10_IDLE;	
			break;
	}
	OBJS_DRV_WR_INFO(drv,buf,len);
}

//---------------------------------------------------------------
//	DI10-data-completion
//---------------------------------------------------------------
void di10_data_cpl_fct (void * data)	{
	OBJS_CREATE_DRV_PTR(data);
	OBJS_CREATE_IDEV_PTR;	
	di10_control * ct = (void *) drv->ct;
	unsigned char	typ;
	unsigned char 	len;
	unsigned char 	buf[PAA_BUFFER_SIZE];
	u16				val;
	static u16		shot;
	
	/// FINAL MESSAGE! 
	if(atomic_read( &ct->msg_stop_job ) ) {
// HACK 29.10.10		printk("msg_stop_job!\n");
		OBJS_DRV_WR_STATE(drv,e_shutdown);		
		complete( &ct->msg_stop_quit ); // final msg call, shutdown
		return;
	}
 	/// MASTER-NACHRICHT holen
	spin_lock_bh(&ct->msg_lock);
	typ = ct->msg.rx_typ;
	len = ct->msg.rx_len;
	memcpy(&buf,&ct->msg.rx_buf,len);
	spin_unlock_bh(&ct->msg_lock);

	/// Typ auswerten
	switch(typ)	{
		case PAA_MSG_DATA:	/// DATA
			val = buf[1];
			val = (val << 8) | buf[0];
			if( shot != val) {
				shot = val;
				DEBUG(MCT_DEBUG_LEVEL3,"%s in:%d\n",__FUNCTION__, val);
				OBJS_IDEV_WR_INPUT(idev,val);
			}
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__,buf,len);
			OBJS_DRV_WR_INFO(drv,buf,len);
			process = PROC_DI10_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__,PAA_MSG_STR_UNSUPPORTED);
			memcpy(buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			OBJS_DRV_WR_INFO(drv,buf,len);
			break;
	}
};

//---------------------------------------------------------------
//	DI10-Timer
//---------------------------------------------------------------
static void di10_timer_function(unsigned long data) {
	OBJS_CREATE_DRV_PTR(data);	
	di10_control * ct = (void *) drv->ct;
	enum e_state state;

	ct->timer.expires = jiffies + TIMER_DI10_GRANULARITY;
	if(atomic_read( &ct->timer_stop_job ) ) {
		complete( &ct->timer_stop_quit ); 					// final spi timer call
	} else {
		// ***************************
		// DRIVER-STATE 	
		// ***************************	
		OBJS_DRV_RD_STATE(drv,state);			// driver-status
		switch(state)	{
			case e_stop: 							// Stoppen!
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver stopped!\n", __FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_idle);	// Idle-Mode
				break;				
			case e_start:							// Starten!			
			case e_reset:	
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver started!\n", __FILE__,__FUNCTION__);
					process = PROC_DI10_INFO;
					OBJS_DRV_WR_STATE(drv,e_running);
				break;	
			case e_running:
				// PAA-Message
				switch(process) {
					case PROC_DI10_INFO:
						paa_message_init(&ct->msg,&ct->msg_lock,ct->tx_buf,ct->rx_buf);
						ct->msg.tx_typ = PAA_MSG_INFO;
						ct->msg.tx_len = strlen(di10_drv_name)+1;
						strcpy(ct->tx_buf, di10_drv_name); 
						ct->msg.complete = di10_info_cpl_fct;	
						ct->msg.context = (void*) data;			// available to completion
						paa_async(ct->paa, &ct->msg);	
					break;
					case PROC_DI10_DATA:
						paa_message_init(&ct->msg,&ct->msg_lock,ct->tx_buf,ct->rx_buf);
						ct->msg.tx_typ = PAA_MSG_DATA;
						ct->msg.tx_len = 2;
						ct->msg.complete = di10_data_cpl_fct;	
						ct->msg.context = (void*) data;	// available to completion
						paa_async(ct->paa, &ct->msg);
						break;
					default:
						printk("PROC_DI10_UNKNONW!\n");
					case PROC_DI10_IDLE:
						OBJS_DRV_WR_STATE(drv,e_stop);
						break;
				}		
				break;

			case e_shutdown:
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver shutdown!\n", __FILE__,__FUNCTION__);
				break;		

			default:		// e_idle e_error 
				break;	
		};
	add_timer( &ct->timer); // Timer aufziehen
	} // end else
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SECTION: PAA-SLAVE-Driver 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int slave_probe(struct paa_device *paa)	{	
	di10_control * ct;
	trace_call_dev(&paa->dev);
	ct = kzalloc(sizeof(di10_control), GFP_ATOMIC);
	if(ct==NULL) {						// kein Speicher
//		kfree(ct);
		printk("kzalloc faild!\n");
		dev_err(&paa->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): PAA slot_select:%d slot_mode:%d Alias:%s \n",__FILE__, __FUNCTION__,  paa->slot_select, paa->slot_mode, paa->modalias);
	
	/// LINK: paa->dev to di10_control and backward
	dev_set_drvdata(&paa->dev,ct);	 			// bereich merken
	ct->paa = paa;	
	
	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);		// completion  object
	init_timer(&ct->timer);						// timer-initialisieren
	
	/// MSG-QUEUE 
	init_completion(&ct->msg_stop_quit);		// completion  object

	/// LINK
	di10_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &di10_O_Mct;
	ct->timer.function = di10_timer_function;
	ct->timer.expires = jiffies + TIMER_DI10_GRANULARITY;
	add_timer( &ct->timer);						// timer-start
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 

#ifdef MODULE
static int slave_remove(struct paa_device *paa)	{	
	di10_control *ct = dev_get_drvdata(&paa->dev);	
	trace_call_dev(&paa->dev);
	
	/// DI10-MESSAGE-STOP
	atomic_set( &ct->msg_stop_job, 1);	
	// Message nicht mehr einhängen max. 1 Sekunde warten
	wait_for_completion_timeout(&ct->msg_stop_quit, HZ*3); 	

	/// DI10-TIMER-STOP
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
		.name = "slave_drv new???",
		.bus = &paa_bus_type,
		.owner = THIS_MODULE,
	},
	.probe 	= slave_probe,
	.remove = __exit_p(slave_remove),
};

static struct paa_device * slave_dev;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DI10-DRIVER (platform) with 1 DEVICE  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static struct platform_driver di10_drv = {
		.driver = {		
			.name = "di10_drv new???",// Wert wird erst später gesetzt 
			.bus = &platform_bus_type,// ... /sys/bus/platform/drivers/DRIVER_NAME
	},		// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};

// INPUT-DEVICE-PTR (platform-device)
static struct platform_device * inputs; 

static void di_dev_release(struct device * dev)	{
	complete(&device_is_free);
}
static int  di_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;

	inputs  = platform_device_alloc(di10_dev_in_name,0); 			
	if(inputs) {
		inputs->dev.release = di_dev_release;		// Release-Function
		platform_device_add(inputs);
		platform_set_drvdata(inputs, &di10_O_Inputs);// dem DI-Platform Device die Datenstruktur mitgeben
		inputs->dev.driver = &drv->driver;
		lnk_err = device_bind_driver(&inputs->dev);			
		di10_sysfs_dev_add(&inputs->dev);			// DI-Devices-SYSFS erzeugen	
	}
	else	{
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",di10_dev_in_name);
		ret = -ENOMEM;
	}
	if(ret)	{
		if( inputs) {	
			platform_set_drvdata(inputs,NULL);		// DI-Platform-Device Datenzeiger aufheben
			platform_device_unregister(inputs);		// DI-Platform-Device entfernen
			di10_sysfs_dev_del(&inputs->dev);		// DI-Devices-SYSFS entfernen
		}
	}
	return ret;
}
static void  di_devs_destroy(void)	{
//	trace_call();
	if(inputs)	{	
		platform_set_drvdata(inputs,NULL);			// DI-Platform-Device Datenzeiger aufheben			
		platform_device_unregister(inputs);			// DI-Platform-Device entfernen
		di10_sysfs_dev_del(&inputs->dev);			// DI-Devices-SYSFS entfernen 
	}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init di10_init(void)	{
	int ret = 0;
	struct paa_master * master;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);

	/// ÜBERLADEN!
	strcpy((char*)(di10_drv.driver.name), di10_drv_name);
	strcpy((char*)(chip_di10.modalias), di10_drv_name);
	chip_di10.slot_select = di10_slot;
	strcpy((char*)(slave_drv.driver.name), di10_drv_name);

	ret = platform_driver_register(&di10_drv);		// DI10-Platformtreiber anmelden ...
	if(ret)	{ 										// Error-Message 0
		printk("Err!platform_driver_register\n");
		goto RESOURCE_RWND_NULL;	
	}
	di10_sysfs_drv_add(&di10_drv.driver);			// DI10-Driver-SYSFS anlegen
	ret = 	di_devs_build(&di10_drv);				// DI-Platform-Device bauen
	if(ret) {										// Error-1
		printk("Err!di_devs_build\n");
		goto RESOURCE_RWND_ERROR_1;
	}
	master =paa_busnum_to_master(PAA_BUS_NUM);		// Master ermitteln
	if(!master) {									// Error-2
		printk("Err!paa_busnum_to_master\n");
		goto RESOURCE_RWND_ERROR_2;
	}
	slave_dev = paa_new_device(master,&chip_di10);	// Geräteslot erzeugen
	if(!slave_dev) {								// EXPAND: paa_board_info
		printk("Err!paa_new_device\n");
		goto RESOURCE_RWND_ERROR_2;					// Error-2
	}

	if(strlen(di10_json) != 0)						// modprobe driver json={....}
		di10_objects_json(di10_json);				// ...JSON-Parameter
	di10_objects_generic();							// ...Classic-Parameter		

#ifdef	MCT_PAA_DI10_DEV_IF_MBUS
	mbus_build(&di10_drv,chip_di10.slot_select);	/// MBUS, Multi-Instanz
#endif
#ifdef	MCT_PAA_DI10_DEV_IF_MODBUS
	modbus_build(&di10_drv,chip_di10.slot_select);	/// MODBUS,Multi-Instanz
#endif

	ret = paa_register_driver(&slave_drv);			// SLAVE-PAA-DRIVER anmelden ...
	if(ret)	{										// Error	
		goto RESOURCE_RWND_DEVICE;
	}												
	goto RESOURCE_OKAY;

RESOURCE_RWND_DEVICE:								// ***** REWIND on ERROR ************* 
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_DI10_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_DI10_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
RESOURCE_RWND_ERROR_2:
	di_devs_destroy();								// DI-Platform-Devices entfernen!
RESOURCE_RWND_ERROR_1:								// FEHLER: Platform-Devices bauen
	di10_sysfs_drv_del(&di10_drv.driver);			// DI10-Driver-SYSFS entfernen
	platform_driver_unregister(&di10_drv);			// DI10-Platform-Driver abmelden
RESOURCE_RWND_NULL:									// FEHLER: DI10-Platformtreiber anmelden
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",di10_drv_name);
		ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit di10_exit(void)	{
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);		
	paa_unregister_driver(&slave_drv);				// SLAVE-DRV abmelden
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_DI10_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_DI10_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	di_devs_destroy();								// DI-Platform-Device entfernen	
	di10_sysfs_drv_del(&di10_drv.driver);			// DI10-Driver-SYSFS entfernen
	platform_driver_unregister(&di10_drv);			// DI10-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEV_MCT_PAA_DI10_IN_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};

module_init( di10_init );
module_exit( di10_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("(digital in 10 channel) slave paa_driver");

