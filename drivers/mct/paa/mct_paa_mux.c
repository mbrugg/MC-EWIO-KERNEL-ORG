/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

*********************************************************************************/
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/platform_device.h>

#include "../mct_debug.h"			// debug-support
#include "mct_paa.h"
#include "mct_paa_mux.h"
#include "mct_paa_mux_objs.h"
#include "mct_paa_mux_attr.h"

#include "mct_paa_lbus_mstr.h"
#include "mct_paa_spi.h"

static DECLARE_COMPLETION(paa_thread_exited);

static struct platform_device paa_mux_device = {
	.name		= PLATFORM_DEVICE_NAME,
	.id			= PAA_BUS_NUM,
	.dev		= {
	},
};

static int paa_mux_timer_create(struct paa_mux * as) {
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	init_completion(&as->OT.completion);
	init_timer(&as->OT.timer);
	as->OT.timer.data = (unsigned long) as;
	as->OT.timer.function = spi_timer_function;
	as->OT.granularity = TIMER_GRANULARITY;
	as->OT.timer.expires = jiffies + Op_PaaMux->OT.granularity;
	return 0;
}

static int paa_mux_timer_delete(struct paa_mux * as) {
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	atomic_set( &as->OT.atomic, 1);
	wait_for_completion(&as->OT.completion);
	if( timer_pending( &as->OT.timer ) )
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer not used!\n", __FILE__,__FUNCTION__);
	if( del_timer_sync( &as->OT.timer ) ) 
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer active!\n", __FILE__,__FUNCTION__);
	else
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer stop!\n", __FILE__,__FUNCTION__);
	return 0;
}

///**********************
/// MESSAGE-TRANSFER-PUMP
///**********************
static int paa_mux_transfer(struct paa_device *paa, struct paa_message *msg)
{
	struct paa_mux		*as;
	unsigned long		flags;
//	struct device		*controller = paa->master->dev.parent;
	as = paa_master_get_devdata(paa->master);
	//HACK
	// dev_dbg(controller, "new message %p submitted for %s\n", msg, paa->dev.bus_id);
	// printk("new message %p submitted for %s\n", msg, paa->dev.bus_id);
	if (as->stopping)
		return -ESHUTDOWN;
	//buffer ??
	if (!(msg->tx_buf || msg->rx_buf)) {
		printk(KERN_ERR "%s %s(): missing rx or tx buf\n",__FILE__, __FUNCTION__);
		return -EINVAL;
	}
	// HACK
	// printk("  xfer %p: len %u tx %p rx %p\n", msg, msg->len, msg->tx_buf, msg->rx_buf);
	// msg->status = -EINPROGRESS;
	// msg->actual_length = 0;
	
	///	NEXT-MESSAGE
	spin_lock_irqsave(&as->transfer_lock, flags);
	list_add_tail(&msg->transfer_item, &as->transfer_list);
	spin_unlock_irqrestore(&as->transfer_lock, flags);
	return 0;
}

static int paa_mux_setup(struct paa_device *paa) {
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	return 0;
}

static void paa_mux_cleanup(struct paa_device *paa){
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
}

static int __devinit paa_mux_probe(struct platform_device *pdev)	{
	struct  paa_master	*master;
	struct  paa_mux		*as;
	int			ret;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);

	/* setup spi core then atmel-specific driver state */
	master = paa_alloc_master(&pdev->dev, sizeof *as);
	if (!master) {
		ret = -ENOMEM;
		goto master_error;
	}
	master->bus_num 		= pdev->id;
	master->slots_max	 	= LBUS_DEF_SLOTS_MAX;
	master->setup 			= paa_mux_setup;
	master->transfer 		= paa_mux_transfer;
	master->cleanup 		= paa_mux_cleanup;
	platform_set_drvdata(pdev, master);
	/// AS-STUFF		
	as = paa_master_get_devdata(master);
	as->pdev = pdev;
	ret = paa_register_master(master);
	if (ret)
		goto master_error;
	return 0;
master_error:
	printk(KERN_ERR "%s %s(): MASTER ERROR\n",__FILE__, __FUNCTION__);
	paa_master_put(master);	
	return ret;
}

static int __exit paa_mux_remove(struct platform_device *pdev) {
	struct paa_master	*master = platform_get_drvdata(pdev);
	struct paa_mux		*as 	= paa_master_get_devdata(master);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/// THREAD-END
	atomic_inc(&as->transfer_stop);
	paa_unregister_master(master);
	return 0;
}

static struct platform_driver paa_mux_driver = {
	.driver		= {
		.name	= PLATFORM_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= paa_mux_probe,
	.remove		= __exit_p(paa_mux_remove),
};

static int __init paa_mux_init(void) {
	struct paa_master	*master;
	struct paa_mux		*as;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	platform_driver_register(&paa_mux_driver);	// Platform: MUX-Treiber registrieren
	platform_device_register(&paa_mux_device);	// Platform: MUX-Gerät registrieren

	paa_mux_attr_add(&paa_mux_driver.driver);
	master = paa_busnum_to_master(PAA_BUS_NUM);	// Master per Busnummer ermitteln	
	as = paa_master_get_devdata(master);		// allokierten paa_mux Bereich holen
	
	Op_PaaMux = as;								/// Global Merken 
	/// MUX-STUFF
	paa_mux_objects_create(as);
	
	if(strlen(mux_json) != 0)					// modprobe driver json={....}
		paa_mux_objects_json(mux_json);			// ...JSON-Parameter
	paa_mux_objects_generic(as);				// ...Classic-Parameter		
	
	/// MUX-TRANSFER
	spin_lock_init(&as->transfer_lock);
	atomic_set(&as->transfer_stop,0);
	INIT_LIST_HEAD(&as->transfer_list);
	/// LBUS 	paa_mux->lbus wird initialisiert
	paa_lbus_mstr_init(&as->lbus,as->Op_LBus->P_Slots.value);
	/// TIMER
	paa_mux_timer_create(as);
	/// LDEV	paa_mux->ldev wird initialisiert!
	spi_register_driver(&spi_drv);
	add_timer( &as->OT.timer);	\
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n", __FILE__,__FUNCTION__);
	return 0;
}

static void __exit paa_mux_exit(void) {
	struct paa_master	*master;
	struct paa_mux		*as;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	master = paa_busnum_to_master(PAA_BUS_NUM);	// Master per Busnummer ermitteln	
	as = paa_master_get_devdata(master);		// allokierten paa_mux Bereich ermitteln	
	paa_mux_timer_delete(as);
	paa_lbus_mstr_delete(&as->lbus);			/// LBUS-MASTER
	spi_unregister_driver(&spi_drv);
	paa_mux_attr_del(&paa_mux_driver.driver);
	platform_device_unregister(&paa_mux_device);
	platform_driver_unregister(&paa_mux_driver);
}

module_init(paa_mux_init);
module_exit(paa_mux_exit);

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("paa_master");
MODULE_ALIAS("mct_paa_mux");

