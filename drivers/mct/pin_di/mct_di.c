/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011
 	
 	Description:  Direct/S0-Driver

 *******************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/interrupt.h>	// local_bh_enable

#include "mct_pin_di_objs.h"		// objects
#include "mct_pin_di_sysfs.h"		// sysfs-support
#include "mct_pin_di_timer.h"		// timer-functions
#include "../mct_debug.h"
#include "mct_di.h"

#ifdef MCT_PIN_DI_DEV_IF_MBUS		
	#include "mct_pin_di_mbus.h"		/// MBUS	
#endif

#ifdef MCT_PIN_DI_DEV_IF_MODBUS		
	#include "mct_pin_di_modbus.h"		/// MODBUS	
#endif
static DECLARE_COMPLETION( device_is_free );

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DI-DRIVER (platform) with 1 DEVICE  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DUAL-DRIVER 
static struct platform_driver di_drv = {
		.driver = {		
			.name = "di_drv new???",		// Wert wird erst später gesetzt 
			.bus = &platform_bus_type,		// ... /sys/bus/platform/drivers/DRIVER_NAME
	},										// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};
// INPUT-DEVICE-PTR (platform-device)
static struct platform_device * inputs; 

static void di_dev_release(struct device * dev)	{
	complete(&device_is_free);
}

static int  di_devs_build(struct platform_driver * drv) {
	int ret = 0;
	int lnk_err;
	
	inputs = platform_device_alloc(di_dev_in_name,0); 
	if(inputs) {
		inputs->dev.release = di_dev_release;		// Release-Function
		platform_device_add(inputs);				// Platform-Device anmelden
		platform_set_drvdata(inputs, &di_O_Inputs); // DI-Platform Device den Datenzeiger mitgeben
		inputs->dev.driver = &drv->driver;					
		lnk_err = device_bind_driver(&inputs->dev);	// symbolischen Link
		di_sysfs_dev_add(&inputs->dev);				// DI-Devices-SYSFS erzeugen
	}
	else	{
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",di_dev_in_name);
		ret = -ENOMEM;
	}
	if(ret)	{
		if( inputs) {	
			platform_set_drvdata(inputs,NULL);  // DI-Platform-DEVICE Datenzeiger aufheben	
			platform_device_unregister(inputs); // DI-Platform-DEVICE entfernen
			di_sysfs_dev_del(&inputs->dev);	 	// SYSFS für DI-DEVICES entfernen
		}
	}
	return ret;
}

static void  di_devs_destroy(void)	{
	if(inputs)	{	
		platform_set_drvdata(inputs,NULL);	// DI-Platform-DEVICE Datenzeiger aufheben
		platform_device_unregister(inputs);	// DI-Platform-DEVICE deregistrieren
		di_sysfs_dev_del(&inputs->dev);		// SYSFS für DI-DEVICES entfernen 
	}
};

static int di_timer_create(struct tdi_drv  * drv) {
	unsigned char mode;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	init_completion(&drv->OT.completion);
	init_timer(&drv->OT.timer);
	drv->OT.timer.data = (unsigned long) drv;	
	mode = drv->Op_Inputs->Op_Mode->P_Value.value;
	switch(mode) {
		case MODE_DIRECT:
			drv->OT.timer.function = timer_direct_func;
			drv->OT.granularity = DIRECT_GRANULARITY;
			drv->OT.timer.expires = jiffies + DIRECT_GRANULARITY;
			DEBUG(MCT_DEBUG_LEVEL1, "direct timer!\n");
			return 1;
		break;
		case MODE_S0:
			drv->OT.timer.function = timer_s0_func;
			drv->OT.granularity = S0_GRANULARITY;
			drv->OT.timer.expires = jiffies + S0_GRANULARITY;
			DEBUG(MCT_DEBUG_LEVEL1,"s0 timer!\n");
			return 1;
		break;
		default:
			printk(KERN_ERR "timer mode %d unsupported!\n",mode); 
			complete(&drv->OT.completion);
		break;
	}
	return 0;
}

static int di_timer_delete(struct tdi_drv  * drv) {
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	atomic_set(&drv->OT.atomic, 1);
	wait_for_completion(&drv->OT.completion);
	if( timer_pending( &drv->OT.timer ) )
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer not used!\n", __FILE__,__FUNCTION__);
	if( del_timer_sync( &drv->OT.timer ) ) 
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer active!\n", __FILE__,__FUNCTION__);
	else
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer stop!\n", __FILE__,__FUNCTION__);
	return 0;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Anmeldung:
static int __init di_init(void)	{
	int ret = 0;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	
	/// ÜBERLADEN!
	strcpy((char*)(di_drv.driver.name), di_drv_name);

	ret = platform_driver_register(&di_drv);// DI-Platformtreiber anmelden ...
	if(ret)	{ 								// Error-Message 1
		goto RESOURCE_RWND_NULL;	
	}
	di_sysfs_drv_add(&di_drv.driver);		// DI-Driver-SYSFS anlegen

	ret= di_devs_build(&di_drv);			// DI-Devices und SYSFS erzeugen
	if(ret) {								// Error-Message 2
		goto RESOURCE_RWND_DRIVER_SYSFS_PART;
	}
	
	di_objs_create(&di_O_Mct);				// OBJECTs initialisieren
	
	if(strlen(di_json) != 0)				// modprobe driver json={....}
		di_objs_json(di_json);				// ...JSON-Parameter
	di_objs_generic();						// ...Classic-Parameter			

#ifdef	MCT_PIN_DI_DEV_IF_MBUS
	mbus_build(&di_drv,di_slot);			/// MBUS, Single-Instanz
#endif
#ifdef	MCT_PIN_DI_DEV_IF_MODBUS
	modbus_build(&di_drv,di_slot);			/// MODBUS,Single-Instance
#endif
	if(di_timer_create(&di_O_Mct))	{		/// TIMER
		add_timer(&di_O_Mct.OT.timer);
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n", __FILE__,__FUNCTION__);
		goto RESOURCE_OKAY;
	}

RESOURCE_RWND_DRIVER_SYSFS_PART:			// FEHLER: DO-Platform-Devices bauen
	di_sysfs_drv_del(&di_drv.driver);		// DIO-Driver-SYSFS entfernen
	platform_driver_unregister(&di_drv);	// DIO-Platform-Driver abmelden

RESOURCE_RWND_NULL:						// FEHLER: DIO-Platformtreiber anmelden
		printk(KERN_ERR "%s %s(): %s %s error!\n",__FILE__, __FUNCTION__,"Resource ",di_dev_in_name);
		ret= -EIO;

RESOURCE_OKAY:
	return ret;
}

// Abmeldung:
static void __exit di_exit(void)	{
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);

	di_timer_delete(&di_O_Mct);				/// TIMER
											
#ifdef	MCT_PIN_DI_DEV_IF_MBUS
	mbus_destroy();							/// MBUS
#endif
#ifdef	MCT_PIN_DI_DEV_IF_MODBUS
	modbus_destroy();						/// MODBUS
#endif
	di_devs_destroy();						// DI-Platform-Devices entfernen
	di_sysfs_drv_del(&di_drv.driver);		// DI-Driver-SYSFS entfernen
	platform_driver_unregister(&di_drv);	// DI-Platform-Driver abmelden ...	
}


// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Steffen Kutsche");
MODULE_DESCRIPTION("mct-pin-di driver");

// Modul anmelden
module_init( di_init );
module_exit( di_exit );
