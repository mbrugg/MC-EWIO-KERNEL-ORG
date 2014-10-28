/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
 	
	Description:
	
 *******************************************************************************/
#include <linux/fs.h>				// sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>		// local_bh_enable
#include <linux/spi/spi.h>

#include "../mct_json.h"			// json-support
#include "../mct_debug.h"
#include "mct_spi_dio_objs.h"		// objects
#include "mct_spi_dio_sysfs.h"		// sysfs-support
#include "mct_dio.h"

#ifdef MCT_SPI_DIO_DEV_IF_MBUS		
	#include "mct_spi_dio_mbus.h"	/// MBUS	
#endif

#ifdef MCT_SPI_DIO_DEV_IF_MODBUS		
	#include "mct_spi_dio_modbus.h"	/// MODBUS	
#endif

static DECLARE_COMPLETION( device_is_free );

extern void icom_svr_set_cli_init(unsigned char client, unsigned char init);
extern void icom_svr_get_cli_info(void);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DIO-DRIVER (on the top of SPI-Core)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------
//	DIO-async-completion
//---------------------------------------------------------------
void dio_complete_function (void * data)	{
	unchar	ch=0;
	unchar o_out;	
	unchar o_leds;
	unchar i_keys;
	unchar i_opto;
	unchar links;
	enum e_state state;
	OBJS_CREATE_DRV_PTR(data);		
	OBJS_CREATE_ODEV_PTR;
	OBJS_CREATE_IDEV_PTR;

	struct 	t_dkey *dkey;
	dio_control * ct = drv->ct;
	o_out  = ct->o_buf[O_REG_CHIP_0];// Ausgänge: REL's/LED's lesen ..		
	o_leds = o_out;
	o_leds 	= ((o_leds & OUT8_LEDS_MASK) >> OUT8_LEDS_SHIFTER); 
	i_keys 	= ct->i_buf[I_REG_CHIP_0];// Eingänge: Taster lesen (Initialisierung)
	i_opto 	= ct->i_buf[I_REG_CHIP_1];// Eingänge: Optokoppler lesen

	OBJS_DRV_RD_STATE(drv, state);
	switch(state) {
		case e_start:
				icom_svr_get_cli_info(); 		
				OBJS_DRV_WR_STATE(drv,e_init);	// alle Infos von den Clients abrufen
				return;							// und merken!
				break;

		case e_shutdown:
				o_leds = 0; 
				OBJS_ODEV_WR_INIT_4Bit(odev,o_leds);		// in der init-Variable sichern
				OBJS_DRV_WR_STATE(drv,e_byebye);	// Clients			
				return;
				break;

		default: 
				break;
	}
	
	// ***********************************
	// DIGITAL CHANNELS
	// ***********************************
	for(ch=0; ch < I_DKEY_CHANNELS;	ch++)	{
		dkey = &ct->dkeys[ch] ;
		if(_bitset(i_keys,ch))		{	 	// Taster = 1 (gedrückt)
			switch(dkey->state)	{
				case DKEY_IDLE:
					dkey->tick_h = 1;
					dkey->state= DKEY_CLASSIFY;
					break;	
				case DKEY_CLASSIFY:					// H-Signale einsammeln ...
					if(dkey->tick_h < DKEY_WAIT_MAXI_LIMIT)		
						dkey->tick_h++;				
					break;		
				default:
					break; 
			}
		}
		else	{									// Taster = 0 (nicht gedrückt)
			switch(dkey->state)	{
				case DKEY_CLASSIFY:	// Auswertung ...
					if(dkey->tick_h < DKEY_WAIT_SHORT_LIMIT ){	// Zu Kurz rausfiltern
						dkey->tick_h = 0;
						dkey->state = DKEY_IDLE;
						break;
					}	
					dkey->tick_h = 0;
					dkey->state = DKEY_SHORT;
					break;

				case DKEY_SHORT:
					OBJS_ODEV_RD_LINK(odev,ch,links);
					if(!links) {
						//printk("!Links %d\n",links);
						_clrbit(o_leds,ch);
						dkey->state = DKEY_IDLE;
					}
					else {
						//printk("Links %d\n",links);
						_togbit(o_leds,ch);		// bit-umschalten	
						dkey->state = DKEY_EXPORT; // Tastendruck exportieren ...
					}
					DEBUG(MCT_DEBUG_LEVEL4,"KEY:ch %d o_leds: %d\n",ch, o_leds);
					// Init-LED's merken (wird erst in der nächsten 
					// Timerloop am Port ausgegeben!)
					OBJS_ODEV_WR_INIT_4Bit(odev,o_leds);	// in der init-Variable sichern
					break;
					
				case DKEY_EXPORT:
					OBJS_ODEV_RD_INIT(odev,o_leds);		/// INTERCOM
					icom_svr_set_cli_init(ch,o_leds);	// Tastendruck exportieren an
					dkey->state = DKEY_IDLE;			// den Client
					break;

				default:
					break;
			}	
		}
	}
	// Opto Eingänge merken
	OBJS_IDEV_WR_INPUT(idev,i_opto);
}

//---------------------------------------------------------------
///	DIO-Timer
//---------------------------------------------------------------
static void dio_timer_function(unsigned long data) {
	OBJS_CREATE_DRV_PTR(data);
	OBJS_CREATE_ODEV_PTR;

	dio_control * _ct = drv->ct;
	enum e_state state;
	u8 o_rels = 0;
	u8 o_leds = 0;

	_ct->timer.expires = jiffies + TIMER_SPI_GRANULARITY;
	if(atomic_read( &_ct->timer_stop_job ) ) {
		complete( &_ct->timer_stop_quit );		// final spi timer call
	} else {

		OBJS_DRV_RD_STATE(drv,state);
		// ***************************
		// STATUS-Auswertung
		// ***************************		
		switch(state)	{
			case e_stop: 		// erfolgt keine Abfrage der Ports mehr!
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver stopped!\n", __FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_shutdown);// siehe Completion-Funktion!
				break;		

			case e_byebye:
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver idle!\n", __FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_idle);
				break;
	
			case e_idle:
				goto RELOAD_TIMER_NO_SPI;		// Der Timer wird aber zyklisch weiter aktiviert. 
				break;							// aber die Ports werden nicht mehr ausgewertet

			case e_start:
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver started!\n", __FILE__,__FUNCTION__);
				break;

			case e_init:
				OBJS_DRV_WR_STATE(drv,e_running);
				break;

			default:							// e_idle, e_running, e_error ... 
				break;	
		}

		// Variable: init lesen und nach o_leds
		OBJS_ODEV_RD_INIT(odev,o_leds);		
		o_leds <<= OUT8_LEDS_SHIFTER;			// LED's xxxx 0000
		// Variable: output lesen und nach o_rels
		OBJS_ODEV_RD_OUTPUT(odev,o_rels);		// REL's 0000 yyyy
												// OUT   xxxx yyyy
		_ct->o_buf[O_REG_CHIP_0] = (o_rels | o_leds); 
		_ct->o_buf[O_REG_CHIP_1] = 0;			// dummy
		
		// SPI-Message 
		spi_message_init(&_ct->msg);
		_ct->msg.complete = dio_complete_function;
		_ct->msg.context = (void*) data;		// available to completion
		spi_message_add_tail(&_ct->xfer, &_ct->msg);
		spi_async(_ct->spi, &_ct->msg);	
RELOAD_TIMER_NO_SPI:
		add_timer( &_ct->timer);				// Timer aufziehen
	} // end else
};

//---------------------------------------------------------------
/// SECTION: SPI-DUPLEX-Driver (Input and Output)
//---------------------------------------------------------------
//---------------------------------------------------------------
//	Duplex-Probe
//---------------------------------------------------------------
static int duplex_probe(struct spi_device *spi)	{	
	dio_control * ct;
	trace_call_dev(&spi->dev);
	ct = kzalloc(sizeof(dio_control), GFP_KERNEL);
	if(!ct) {								// kein Speicher
		kfree(ct);
		dev_err(&spi->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): SPI Hz:%d Mode:%d Alias:%s\n",__FILE__, __FUNCTION__, spi->max_speed_hz,spi->mode,spi->modalias);

	dev_set_drvdata(&spi->dev,ct);	 		// bereich merken
	ct->spi = spi;							// an context übergeben	
	
	// leere AMessage bauen ...	
	memset(&ct->o_buf,0x00,O_REGS);
	memset(&ct->i_buf,0x00,I_REGS);

	ct->xfer.tx_buf = &ct->o_buf;
	ct->xfer.rx_buf = &ct->i_buf;
	ct->xfer.len = 2;
	ct->xfer.cs_change = 0;
	ct->msg.complete = dio_complete_function;

	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);	// completion  object
	init_timer(&ct->timer);					// timer-initialisieren
	dio_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &dio_O_Mct;
	ct->timer.function = dio_timer_function;
	ct->timer.expires = jiffies + TIMER_SPI_GRANULARITY;
	add_timer( &ct->timer);	
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 
//---------------------------------------------------------------
//	Duplex-Remove
//---------------------------------------------------------------
#ifdef MODULE
static int duplex_remove(struct spi_device *spi)	{	
	dio_control *ct = dev_get_drvdata(&spi->dev);	
	trace_call_dev(&spi->dev);
	/// SPI-TIMER-STOP
	atomic_set( &ct->timer_stop_job, 1);
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

//---------------------------------------------------------------
///	Duplex Driver (spi)
//---------------------------------------------------------------
//MODULE_ALIAS("platform:74hc594_74hc165");

static struct spi_driver duplex_drv = {
	.driver = {
		.name = "74hc594_74hc165",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe 	= duplex_probe,
	.remove = __exit_p(duplex_remove),
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: DIO-DRIVER (platform) with 2 DEVICES  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DUAL-DRIVER 
static struct platform_driver dio_drv = {
		.driver = {		
			.name = DRIVER_NAME,			// ... siehe oben, per define! 
			.bus = &platform_bus_type,		// ... /sys/bus/platform/drivers/DRIVER_NAME
	},										// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};
// OUTPUT-DEVICE-PTR (platform-device)
static struct platform_device * outputs; 
// INPUT-DEVICE-PTR (platform-device)
static struct platform_device * inputs; 

static void do_dev_release(struct device * dev)	{
	complete(&device_is_free);
}
static int  do_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;

	//	platform_set_drvdata(pdev,obj);
	outputs = platform_device_alloc(DEVICE_OUT_NAME,0); 			
	if(outputs) {
		outputs->dev.release = do_dev_release;		// Release-Function
		platform_device_add(outputs);
		platform_set_drvdata(outputs, &dio_O_Outputs); 	// DO-Platform-Device den Datenzeiger mitgeben
		outputs->dev.driver = &drv->driver;
		lnk_err= device_bind_driver(&outputs->dev);			
		dio_do_sysfs_dev_add(&outputs->dev);			// DO-Devices-SYSFS erzeugen				
	}
	else	{
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s" DEVICE_OUT_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource",0);
		ret = -ENOMEM;
	}
	if(ret)	{
		if( outputs) {	
			platform_device_unregister(outputs);	// DO-Platform-Device entfernen
			dio_do_sysfs_dev_del(&outputs->dev);	// DO-Devices-SYSFS entfernen
		}
	}
	return ret;
};
static void  do_devs_destroy(void)	{

	if(outputs)	{	
		platform_set_drvdata(outputs,NULL);			// DO-Platform-Device Datenzeiger aufheben			
		platform_device_unregister(outputs);		// DO-Platform-Device entfernen
		dio_do_sysfs_dev_del(&outputs->dev);		// DO-Devices-SYSFS entfernen 
	}
};

static void di_dev_release(struct device * dev)	{
	complete(&device_is_free);
}
static int  di_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;

	inputs  = platform_device_alloc(DEVICE_IN_NAME,0); 			
	if(inputs) {
		inputs->dev.release = di_dev_release;		// Release-Function
		platform_device_add(inputs);
		platform_set_drvdata(inputs, &dio_O_Inputs);	// dem DI-Platform Device die Datenstruktur mitgeben
		inputs->dev.driver = &drv->driver;
		lnk_err = device_bind_driver(&inputs->dev);			
		dio_di_sysfs_dev_add(&inputs->dev);			// DI-Devices-SYSFS erzeugen	
	}
	else	{
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s" DEVICE_IN_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource",0);
		ret = -ENOMEM;
	}
	if(ret)	{
		if( inputs) {	
			platform_set_drvdata(inputs,NULL);		// DI-Platform-Device Datenzeiger aufheben
			platform_device_unregister(inputs);		// DI-Platform-Device entfernen
			dio_di_sysfs_dev_del(&inputs->dev);		// DI-Devices-SYSFS entfernen
		}
	}
	return ret;
}
static void  di_devs_destroy(void)	{
//	trace_call();
	if(inputs)	{	
		platform_set_drvdata(inputs,NULL);			// DI-Platform-Device Datenzeiger aufheben			
		platform_device_unregister(inputs);			// DI-Platform-Device entfernen
		dio_di_sysfs_dev_del(&inputs->dev);			// DI-Devices-SYSFS entfernen 
	}
};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init dio_init(void)	{
	int ret = 0;


	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);

	ret = platform_driver_register(&dio_drv);		// DIO-Platformtreiber anmelden ...
	if(ret)	{ 										// Error-Message 1
		goto RESOURCE_RWND_NULL;	
	}
	dio_sysfs_drv_add(&dio_drv.driver);				// DIO-Driver-SYSFS anlegen

	ret = 	do_devs_build(&dio_drv);				// DO-Platform-Devices bauen
	if(ret) {										// Error-Message 2
		goto RESOURCE_RWND_DRIVER_SYSFS_PART;
	}
	ret = 	di_devs_build(&dio_drv);				// DI-Platform-Devices bauen
	if(ret) {										// Error-Message 3
		goto RESOURCE_RWND_DRIVER_DODEVS_PART;
	}
	
	dio_objects_create();							// OBJECTs initialisieren

	if(strlen(dio_json) != 0)						// modprobe driver json={....}
		dio_objects_json(dio_json);					// ...JSON-Parameter
	dio_objects_generic();	
													// ...Classic-Parameter
#ifdef	MCT_SPI_DIO_DEV_IF_MBUS
	mbus_build(&dio_drv,0);							/// MBUS, Single-Instance
#endif
#ifdef	MCT_SPI_DIO_DEV_IF_MODBUS
	modbus_build(&dio_drv,0);						/// MODBUS,Single-Instance
#endif

	ret = spi_register_driver(&duplex_drv);			// DUPLEX-SPI-DRIVER anmelden ...
	if(ret)	{										// Error-Message 3
		goto RESOURCE_RWND_OUTPUT_DEVICES_PART;
	}												// alle Resourcen erhalten!
	goto RESOURCE_OKAY;
													// ***** REWIND on ERROR ************* 
RESOURCE_RWND_OUTPUT_DEVICES_PART:					// FEHLER: DUPLEX-SPI-Driver registrieren 
#ifdef	MCT_SPI_DIO_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_SPI_DIO_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	di_devs_destroy();								// DI-Platform-Devices entfernen!

RESOURCE_RWND_DRIVER_DODEVS_PART:					// FEHLER: DI-Platform-Devices bauen	
	do_devs_destroy();								// DO-Platform-Devices entfernen!

RESOURCE_RWND_DRIVER_SYSFS_PART:					// FEHLER: DO-Platform-Devices bauen
	dio_sysfs_drv_del(&dio_drv.driver);				// DIO-Driver-SYSFS entfernen
	platform_driver_unregister(&dio_drv);			// DIO-Platform-Driver abmelden

RESOURCE_RWND_NULL:									// FEHLER: DIO-Platformtreiber anmelden
		// Resource-Meldung!
		printk(KERN_ERR "%s %s(): %s" DRIVER_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource",0);
		ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit dio_exit(void)	{
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);		

	spi_unregister_driver(&duplex_drv);				// DUPLEX-SPI-Treiber abmelden
#ifdef	MCT_SPI_DIO_DEV_IF_MBUS	
	mbus_destroy();									/// MBUS
#endif
#ifdef	MCT_SPI_DIO_DEV_IF_MODBUS
	modbus_destroy();								/// MODBUS
#endif
	di_devs_destroy();								// DI-Platform-Devices entfernen	
	do_devs_destroy();								// DO-Platform-Devices entfernen
	dio_sysfs_drv_del(&dio_drv.driver);				// DIO-Driver-SYSFS entfernen
	platform_driver_unregister(&dio_drv);			// DIO-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEVICE_IN_MAX + DEVICE_OUT_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};


module_init( dio_init );
module_exit( dio_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("SPI digital input/output driver (3x8bit)");

