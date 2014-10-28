/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

	Description:  mct_paa_di4 CLIENT-DRIVER (L-Bus Hardware)

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"	// debugging
#include "../mct_paa_msg.h"		// paa-messages
#include "mct_di4_objs.h"		// objects
#include "mct_di4_sysfs.h"		// sysfs-support
#include "mct_di4.h"

#ifdef MCT_PAA_DI4_DEV_IF_MBUS		
	#include "mct_di4_mbus.h"		/// MBUS	
#endif

#ifdef MCT_PAA_DI4_DEV_IF_MODBUS		
	#include "mct_di4_modbus.h"		/// MODBUS	
#endif

static DECLARE_COMPLETION( device_is_free );

// process-stati
#define PROC_DI4_INFO		0
#define PROC_DI4_DATA		1
#define PROC_DI4_IDLE		2
unsigned char process;

static struct paa_board_info chip_di4 = {
	.modalias 		= "chip_di4 new???",
	.bus_num 		= PAA_BUS_NUM,
	.slot_select 	= 0,
	.slot_mode 		= 0,
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DI4-DRIVER (on the top of PAA-Core)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//HACK
unsigned char process;

//---------------------------------------------------------------
//	DI4-info-completion
//---------------------------------------------------------------
void di4_info_cpl_fct (void * data)	{
	OBJS_CREATE_DRV_PTR(data);	
	di4_control * ct = (void *) drv->ct;
	unsigned char typ;
	unsigned char len;
	unsigned char buf[PAA_BUFFER_SIZE];

	/// FINAL MESSAGE! 
	if(atomic_read( &ct->msg_stop_job ) ) {
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
			process = PROC_DI4_DATA;
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__, buf,len);
			process = PROC_DI4_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__, PAA_MSG_STR_UNSUPPORTED);
			memcpy(buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			process = PROC_DI4_IDLE;
			break;
	}
	OBJS_DRV_WR_INFO(drv,buf,len);
}

//---------------------------------------------------------------
//	DI4-data-completion
//---------------------------------------------------------------
void di4_data_cpl_fct (void * data)	{
	OBJS_CREATE_DRV_PTR(data);
	OBJS_CREATE_IDEV_PTR;	
	di4_control * ct = (void *) drv->ct;
	unsigned char typ;
	unsigned char len;
	unsigned char buf[PAA_BUFFER_SIZE];
	u8	val;
	static u8 shot;
	
	/// FINAL MESSAGE! 
	if(atomic_read( &ct->msg_stop_job ) ) {
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
			val = buf[0];
				//HACK
			if( shot != val) {
				shot = val;
				DEBUG(MCT_DEBUG_LEVEL3,"%s in:%d\n",__FUNCTION__, val);
				OBJS_IDEV_WR_INPUT(idev,val);
			}
			break;
		case PAA_MSG_ERROR:	/// ABBRUCH
			printk("%s %s len: %d\n",__FUNCTION__,buf,len);
			OBJS_DRV_WR_INFO(drv,buf,len);
			process = PROC_DI4_IDLE;
			break;
		default:
			printk("%s %s\n",__FUNCTION__,PAA_MSG_STR_UNSUPPORTED);
			memcpy(buf,PAA_MSG_STR_UNSUPPORTED,sizeof(PAA_MSG_STR_UNSUPPORTED));
			OBJS_DRV_WR_INFO(drv,buf,len);
			break;
	}
};

//---------------------------------------------------------------
//	DI4-Timer
//---------------------------------------------------------------
static void di4_timer_function(unsigned long data) {
	OBJS_CREATE_DRV_PTR(data);	
	di4_control * ct = (void *) drv->ct;
	enum e_state state;

	ct->timer.expires = jiffies + TIMER_DI4_GRANULARITY;
	if(atomic_read( &ct->timer_stop_job ) ) {
		complete( &ct->timer_stop_quit ); 					// final spi timer call
	} else {
		// ***************************
		// DRIVER-STATE 	
		// ***************************	
		OBJS_DRV_RD_STATE(drv,state);			// driver-status
		switch(state)	{
			case e_stop: 						// Stoppen!
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver stopped!\n", __FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_idle);	// Idle-Mode
				break;				
			case e_start:						// Starten!			
			case e_reset:	
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver started!\n", __FILE__,__FUNCTION__);
					OBJS_DRV_WR_STATE(drv,e_running); 
					process = PROC_DI4_INFO;	
				break;	
			case e_running:
				// PAA-Message
				// printk("di4-TICK\n"); 				
				switch(process) {
					case PROC_DI4_INFO:
						paa_message_init(&ct->msg,&ct->msg_lock,ct->tx_buf,ct->rx_buf);
						ct->msg.tx_typ = PAA_MSG_INFO;
						ct->msg.tx_len = strlen(di4_drv_name)+1;
						strcpy(ct->tx_buf, di4_drv_name); 
						ct->msg.complete = di4_info_cpl_fct;	
						ct->msg.context = (void*) data;			// available to completion
						paa_async(ct->paa, &ct->msg);	
						break;
					case PROC_DI4_DATA:
						paa_message_init(&ct->msg,&ct->msg_lock,ct->tx_buf,ct->rx_buf);
						ct->msg.tx_typ = PAA_MSG_DATA;
						ct->msg.tx_len = 1;
						ct->msg.complete = di4_data_cpl_fct;	
						ct->msg.context = (void*) data;	// available to completion
						paa_async(ct->paa, &ct->msg);	
						break;
					default:
						printk("PROC_DI4_UNKNONW!\n");
					case PROC_DI4_IDLE:
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
	di4_control * ct;
	trace_call_dev(&paa->dev);
	ct = kzalloc(sizeof(di4_control), GFP_ATOMIC);
	if(ct==NULL) {						// kein Speicher
//		kfree(ct);
		printk("kzalloc faild!\n");
		dev_err(&paa->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): PAA slot_select:%d slot_mode:%d Alias:%s \n",__FILE__, __FUNCTION__,  paa->slot_select, paa->slot_mode, paa->modalias);
	
	/// LINK: paa->dev to di4_control and backward
	dev_set_drvdata(&paa->dev,ct);	 			// bereich merken
	ct->paa = paa;	
	
	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);		// completion  object
	init_timer(&ct->timer);						// timer-initialisieren
	
	/// MSG-QUEUE 
	init_completion(&ct->msg_stop_quit);		// completion  object

	/// LINK
	di4_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &di4_O_Mct;
	ct->timer.function = di4_timer_function;
	ct->timer.expires = jiffies + TIMER_DI4_GRANULARITY;
	add_timer( &ct->timer);						// timer-start
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 

#ifdef MODULE
static int slave_remove(struct paa_device *paa)	{	
	di4_control *ct = dev_get_drvdata(&paa->dev);	
	trace_call_dev(&paa->dev);	
	/// DI4-MESSAGE-STOP
	atomic_set( &ct->msg_stop_job, 1);	
	// Message nicht mehr einhängen max. 1 Sekunde warten
	wait_for_completion_timeout(&ct->msg_stop_quit, HZ*3); 
	/// DI4-TIMER-STOP
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
///	SECTION: DI4-DRIVER (platform) with 1 DEVICE  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static struct platform_driver di4_drv = {
		.driver = {		
			.name = "di4_drv new???",		// Wert wird erst später gesetzt 
			.bus = &platform_bus_type,		// ... /sys/bus/platform/drivers/DRIVER_NAME
	},										// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};

// INPUT-DEVICE-PTR (platform-device)
static struct platform_device * inputs; 

static void di_dev_release(struct device * dev)	{
	complete(&device_is_free);
}
static int  di_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;

	inputs  = platform_device_alloc(di4_dev_in_name,0); 
	if(inputs) {
		inputs->dev.release = di_dev_release;		// Release-Function
		platform_device_add(inputs);
		platform_set_drvdata(inputs, &di4_O_Inputs);// dem DI-Platform Device die Datenstruktur mitgeben
		inputs->dev.driver = &drv->driver;
		lnk_err = device_bind_driver(&inputs->dev);			
		di4_sysfs_dev_add(&inputs->dev);			// DI-Devices-SYSFS erzeugen	
	}
	else	{
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",di4_dev_in_name);
		ret = -ENOMEM;
	}
	if(ret)	{
		if( inputs) {	
			platform_set_drvdata(inputs,NULL);		// DI-Platform-Device Datenzeiger aufheben
			platform_device_unregister(inputs);		// DI-Platform-Device entfernen
			di4_sysfs_dev_del(&inputs->dev);		// DI-Devices-SYSFS entfernen
		}
	}
	return ret;
}
static void  di_devs_destroy(void)	{
//	trace_call();
	if(inputs)	{	
		platform_set_drvdata(inputs,NULL);	// DI-Platform-Device Datenzeiger aufheben
		platform_device_unregister(inputs);	// DI-Platform-Device entfernen
		di4_sysfs_dev_del(&inputs->dev);	// DI-Devices-SYSFS entfernen 
	}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init di4_init(void)	{
	int ret = 0;
	struct paa_master * master;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);

	/// ÜBERLADEN!
	strcpy((char*)(di4_drv.driver.name), di4_drv_name);
	strcpy((char*)(chip_di4.modalias), di4_drv_name);
	chip_di4.slot_select = di4_slot;
	strcpy((char*)(slave_drv.driver.name), di4_drv_name);

	ret = platform_driver_register(&di4_drv);		// DI4-Platformtreiber anmelden ...
	if(ret)	{ 										// Error-Message 0
		printk("Err!platform_driver_register\n");		
		goto RESOURCE_RWND_NULL;	
	}
	di4_sysfs_drv_add(&di4_drv.driver);				// DI4-Driver-SYSFS anlegen
	ret = 	di_devs_build(&di4_drv);				// DI-Platform-Device bauen
	if(ret) {										// Error-1
		printk("Err!di_devs_build\n");
		goto RESOURCE_RWND_ERROR_1;
	}
	master =paa_busnum_to_master(PAA_BUS_NUM);		// Master ermitteln
	if(!master) {									// Error-2
		printk("Err!paa_busnum_to_master\n");
		goto RESOURCE_RWND_ERROR_2;
	}
	slave_dev = paa_new_device(master,&chip_di4);	// Geräteslot erzeugen
	if(!slave_dev) {								// EXPAND: paa_board_info
		printk("Err!paa_new_device\n");
		goto RESOURCE_RWND_ERROR_2;					// Error-2
	}

	if(strlen(di4_json) != 0)						// modprobe driver json={....}
		di4_objects_json(di4_json);					// ...JSON-Parameter
	di4_objects_generic();							// ...Classic-Parameter		

#ifdef	MCT_PAA_DI4_DEV_IF_MBUS
	mbus_build(&di4_drv,chip_di4.slot_select);		/// MBUS, Multi-Instanz
#endif
#ifdef	MCT_PAA_DI4_DEV_IF_MODBUS
	modbus_build(&di4_drv,chip_di4.slot_select);	/// MODBUS,Multi-Instanz
#endif

	ret = paa_register_driver(&slave_drv);			// SLAVE-PAA-DRIVER anmelden ...
	if(ret)	{										// Error
		goto RESOURCE_RWND_DEVICE;
	}												
	goto RESOURCE_OKAY;

RESOURCE_RWND_DEVICE:								// ***** REWIND on ERROR ************* 
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_DI4_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_DI4_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
RESOURCE_RWND_ERROR_2:
	di_devs_destroy();								// DI-Platform-Devices entfernen!
RESOURCE_RWND_ERROR_1:								// FEHLER: Platform-Devices bauen
	di4_sysfs_drv_del(&di4_drv.driver);				// DI4-Driver-SYSFS entfernen
	platform_driver_unregister(&di4_drv);			// DI4-Platform-Driver abmelden
RESOURCE_RWND_NULL:									// FEHLER: DI4-Platformtreiber anmelden
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",di4_drv_name);
		ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit di4_exit(void)	{
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);		
	paa_unregister_driver(&slave_drv);				// SLAVE-DRV abmelden
	paa_unregister_device(slave_dev);				// SLAVE-DEV abmelden
#ifdef	MCT_PAA_DI4_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_PAA_DI4_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	di_devs_destroy();								// DI-Platform-Device entfernen	
	di4_sysfs_drv_del(&di4_drv.driver);				// DI4-Driver-SYSFS entfernen
	platform_driver_unregister(&di4_drv);			// DI4-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEV_MCT_PAA_DI4_IN_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};

module_init( di4_init );
module_exit( di4_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("(digital in 4 channel) slave paa_driver");

