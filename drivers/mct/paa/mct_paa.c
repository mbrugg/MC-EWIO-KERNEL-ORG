/*
 * paa.c - PAA init/core code
 *
 * Copyright (C) 2005 David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/cache.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/slab.h>
//#include <linux/mod_devicetable.h>
//#include <linux/paa/paa.h>
//#include <linux/of_paa.h>

#include "../mct_debug.h"			// debug-support
#include "mct_paa.h"

/* PAA bustype and paa_master class are registered after board init code
 * provides the PAA device tables, ensuring that both are present by the
 * time controller driver registration causes paa_devices to "enumerate".
 */
static void paadev_release(struct device *dev)	{
	struct paa_device	*paa = to_paa_device(dev);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/* paa masters may cleanup for released devices */
	if (paa->master->cleanup)
		paa->master->cleanup(paa);
	paa_master_put(paa->master);
	kfree(paa);
}

static ssize_t modalias_show(struct device *dev, struct device_attribute *a, char *buf)	{
	const struct paa_device	*paa = to_paa_device(dev);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	return sprintf(buf, "%s\n", paa->modalias);
}

static ssize_t slot_mode_show(struct device *dev, struct device_attribute *a, char *buf) {
	const struct paa_device	*paa = to_paa_device(dev);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	return sprintf(buf, "%u\n", paa->slot_mode);
}

static struct device_attribute paa_dev_attrs[] = {
	__ATTR_RO(modalias),
	__ATTR_RO(slot_mode),
	__ATTR_NULL,
};

/* modalias support makes "modprobe $MODALIAS" new-style hotplug work,
 * and the sysfs version makes coldplug work too.
 */
static const struct paa_device_id *paa_match_id(const struct paa_device_id *id,
						const struct paa_device *sdev)	{
	while (id->name[0]) {
		if (!strcmp(sdev->modalias, id->name))
			return id;
		id++;
	}
	return NULL;
}

const struct paa_device_id *paa_get_device_id(const struct paa_device *sdev)	{
	const struct paa_driver *sdrv = to_paa_driver(sdev->dev.driver);
	return paa_match_id(sdrv->id_table, sdev);
}
EXPORT_SYMBOL_GPL(paa_get_device_id);

static int paa_match_device(struct device *dev, struct device_driver *drv)	{
	const struct paa_device	*paa = to_paa_device(dev);
	const struct paa_driver	*sdrv = to_paa_driver(drv);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/* Attempt an OF style match */
	if (of_driver_match_device(dev, drv))
		return 1;
	if (sdrv->id_table)
		return !!paa_match_id(sdrv->id_table, paa);
	return strcmp(paa->modalias, drv->name) == 0;
}

static int paa_uevent(struct device *dev, struct kobj_uevent_env *env)	{
	const struct paa_device		*paa = to_paa_device(dev);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	add_uevent_var(env, "MODALIAS=%s%s", PAA_MODULE_PREFIX, paa->modalias);
	return 0;
}

struct bus_type paa_bus_type = {
	.name		= "paa",
	.dev_attrs	= paa_dev_attrs,
	.match		= paa_match_device,
	.uevent		= paa_uevent,
};
EXPORT_SYMBOL_GPL(paa_bus_type);


static int paa_drv_probe(struct device *dev)	{
	const struct paa_driver		*sdrv = to_paa_driver(dev->driver);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	return sdrv->probe(to_paa_device(dev));
}

static int paa_drv_remove(struct device *dev)	{
	const struct paa_driver		*sdrv = to_paa_driver(dev->driver);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	return sdrv->remove(to_paa_device(dev));
}

static void paa_drv_shutdown(struct device *dev){
	const struct paa_driver		*sdrv = to_paa_driver(dev->driver);
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	sdrv->shutdown(to_paa_device(dev));
}

/**
 * paa_register_driver - register a PAA driver
 * @sdrv: the driver to register
 * Context: can sleep
 */
int paa_register_driver(struct paa_driver *sdrv)
{
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	sdrv->driver.bus = &paa_bus_type;
	if (sdrv->probe)
		sdrv->driver.probe = paa_drv_probe;
	if (sdrv->remove)
		sdrv->driver.remove = paa_drv_remove;
	if (sdrv->shutdown)
		sdrv->driver.shutdown = paa_drv_shutdown;
	return driver_register(&sdrv->driver);
}
EXPORT_SYMBOL_GPL(paa_register_driver);

/*-------------------------------------------------------------------------*/

/* PAA devices should normally not be created by PAA device drivers; that
 * would make them board-specific.  Similarly with PAA master drivers.
 * Device registration normally goes into like arch/.../mach.../board-YYY.c
 * with other readonly (flashable) information about mainboard devices.
 */

struct boardinfo {
	struct list_head	list;
	unsigned		n_board_info;
	struct paa_board_info	board_info[0];
};

//static LIST_HEAD(board_list);
//static DEFINE_MUTEX(board_lock);

/**
 * paa_alloc_device - Allocate a new PAA device
 * @master: Controller to which device is connected
 * Context: can sleep
 *
 * Allows a driver to allocate and initialize a paa_device without
 * registering it immediately.  This allows a driver to directly
 * fill the paa_device with device parameters before calling
 * paa_add_device() on it.
 *
 * Caller is responsible to call paa_add_device() on the returned
 * paa_device structure to add it to the PAA master.  If the caller
 * needs to discard the paa_device without adding it, then it should
 * call paa_dev_put() on it.
 *
 * Returns a pointer to the new device, or NULL.
 */
struct paa_device *paa_alloc_device(struct paa_master *master)
{
	struct paa_device	*paa;
	struct device		*dev = master->dev.parent;

	if (!paa_master_get(master))
		return NULL;

	paa = kzalloc(sizeof *paa, GFP_KERNEL);
	if (!paa) {
		dev_err(dev, "cannot alloc paa_device\n");
		paa_master_put(master);
		return NULL;
	}

	paa->master = master;
	paa->dev.parent = dev;
	paa->dev.bus = &paa_bus_type;
	paa->dev.release = paadev_release;
	device_initialize(&paa->dev);
	return paa;
}
EXPORT_SYMBOL_GPL(paa_alloc_device);

/**
 * paa_add_device - Add paa_device allocated with paa_alloc_device
 * @paa: paa_device to register
 *
 * Companion function to paa_alloc_device.  Devices allocated with
 * paa_alloc_device can be added onto the paa bus with this function.
 *
 * Returns 0 on success; negative errno on failure
 */
int paa_add_device(struct paa_device *paa)
{
	static DEFINE_MUTEX(paa_add_lock);
	struct device *dev = paa->master->dev.parent;
	struct device *d;
	int status;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	/* Chipselects are numbered 0..max; validate. */
/*
	if (paa->chip_select >= paa->master->num_chipselect) {
		dev_err(dev, "cs%d >= max %d\n",
			paa->chip_select,
			paa->master->num_chipselect);
		return -EINVAL;
	}
*/
	if (paa->slot_select >= paa->master->slots_max) {
		dev_err(dev, "cs%d >= max %d\n",paa->slot_select,paa->master->slots_max);
		return -EINVAL;
	}


	/* Set the bus ID string */
	dev_set_name(&paa->dev, "%s.%u", dev_name(&paa->master->dev),paa->slot_select);


	/* We need to make sure there's no other device with this
	 * chipselect **BEFORE** we call setup(), else we'll trash
	 * its configuration.  Lock against concurrent add() calls.
	 */
	mutex_lock(&paa_add_lock);

	d = bus_find_device_by_name(&paa_bus_type, NULL, dev_name(&paa->dev));
	if (d != NULL) {
		dev_err(dev, "slot_select %d already in use\n", paa->slot_select);
		put_device(d);
		status = -EBUSY;
		goto done;
	}

	/* Drivers may modify this initial i/o setup, but will
	 * normally rely on the device being setup.  Devices
	 * using PAA_CS_HIGH can't coexist well otherwise...
	 */
	status = paa_setup(paa);
	if (status < 0) {
		dev_err(dev, "can't %s %s, status %d\n","setup", dev_name(&paa->dev), status);
		goto done;
	}

	/* Device may be bound to an active driver when this returns */
	status = device_add(&paa->dev);
	if (status < 0)
		dev_err(dev, "can't %s %s, status %d\n", "add", dev_name(&paa->dev), status);
	else
		dev_dbg(dev, "registered child %s\n", dev_name(&paa->dev));

done:
	mutex_unlock(&paa_add_lock);
	return status;
}
EXPORT_SYMBOL_GPL(paa_add_device);

/**
 * paa_new_device - instantiate one new PAA device
 * @master: Controller to which device is connected
 * @chip: Describes the PAA device
 * Context: can sleep
 *
 * On typical mainboards, this is purely internal; and it's not needed
 * after board init creates the hard-wired devices.  Some development
 * platforms may not be able to use paa_register_board_info though, and
 * this is exported so that for example a USB or parport based adapter
 * driver could add devices (which it would learn about out-of-band).
 *
 * Returns the new device, or NULL.
 */
struct paa_device *paa_new_device(struct paa_master *master,
				  struct paa_board_info *chip)
{
	struct paa_device	*proxy;
	int			status;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/* NOTE:  caller did any chip->bus_num checks necessary.
	 *
	 * Also, unless we change the return value convention to use
	 * error-or-pointer (not NULL-or-pointer), troubleshootability
	 * suggests syslogged diagnostics are best here (ugh).
	 */

	proxy = paa_alloc_device(master);
	if (!proxy)
		return NULL;

	WARN_ON(strlen(chip->modalias) >= sizeof(proxy->modalias));
/*
	proxy->chip_select = chip->chip_select;
	proxy->max_speed_hz = chip->max_speed_hz;
	proxy->mode = chip->mode;
	proxy->irq = chip->irq;
*/
	proxy->slot_select 	= chip->slot_select;
	proxy->slot_mode 	= chip->slot_mode;

	strlcpy(proxy->modalias, chip->modalias, sizeof(proxy->modalias));
	proxy->dev.platform_data = (void *) chip->platform_data;
	proxy->controller_data = chip->controller_data;
	proxy->controller_state = NULL;

	status = paa_add_device(proxy);
	if (status < 0) {
		paa_dev_put(proxy);
		return NULL;
	}

	return proxy;
}
EXPORT_SYMBOL_GPL(paa_new_device);



/*-------------------------------------------------------------------------*/

static void paa_master_release(struct device *dev)
{
	struct paa_master *master;

	master = container_of(dev, struct paa_master, dev);
	kfree(master);
}

static struct class paa_master_class = {
	.name		= "paa_master",
	.owner		= THIS_MODULE,
	.dev_release	= paa_master_release,
};


/**
 * paa_alloc_master - allocate PAA master controller
 * @dev: the controller, possibly using the platform_bus
 * @size: how much zeroed driver-private data to allocate; the pointer to this
 *	memory is in the driver_data field of the returned device,
 *	accessible with paa_master_get_devdata().
 * Context: can sleep
 *
 * This call is used only by PAA master controller drivers, which are the
 * only ones directly touching chip registers.  It's how they allocate
 * an paa_master structure, prior to calling paa_register_master().
 *
 * This must be called from context that can sleep.  It returns the PAA
 * master structure on success, else NULL.
 *
 * The caller is responsible for assigning the bus number and initializing
 * the master's methods before calling paa_register_master(); and (after errors
 * adding the device) calling paa_master_put() to prevent a memory leak.
 */
struct paa_master *paa_alloc_master(struct device *dev, unsigned size)
{
	struct paa_master	*master;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	if (!dev)
		return NULL;

	master = kzalloc(size + sizeof *master, GFP_KERNEL);
	if (!master)
		return NULL;

	device_initialize(&master->dev);
	master->dev.class = &paa_master_class;
	master->dev.parent = get_device(dev);
	paa_master_set_devdata(master, &master[1]);

	return master;
}
EXPORT_SYMBOL_GPL(paa_alloc_master);

/**
 * paa_register_master - register PAA master controller
 * @master: initialized master, originally from paa_alloc_master()
 * Context: can sleep
 *
 * PAA master controllers connect to their drivers using some non-PAA bus,
 * such as the platform bus.  The final stage of probe() in that code
 * includes calling paa_register_master() to hook up to this PAA bus glue.
 *
 * PAA controllers use board specific (often SOC specific) bus numbers,
 * and board-specific addressing for PAA devices combines those numbers
 * with chip select numbers.  Since PAA does not directly support dynamic
 * device identification, boards need configuration tables telling which
 * chip is at which address.
 *
 * This must be called from context that can sleep.  It returns zero on
 * success, else a negative error code (dropping the master's refcount).
 * After a successful return, the caller is responsible for calling
 * paa_unregister_master().
 */
int paa_register_master(struct paa_master *master)
{
	static atomic_t		dyn_bus_id = ATOMIC_INIT((1<<15) - 1);
	struct device		*dev = master->dev.parent;
	int					status = -ENODEV;
	int					dynamic = 0;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);

	if (!dev)
		return -ENODEV;
	/* even if it's just one always-selected device, there must  be at least one slot */
	if (master->slots_max == 0)
		return -EINVAL;
	/* convention:  dynamically assigned bus IDs count down from the max */
	if (master->bus_num < 0) {
		/* FIXME switch to an IDR based scheme, something like
		 * I2C now uses, so we can't run out of "dynamic" IDs
		 */
		master->bus_num = atomic_dec_return(&dyn_bus_id);
		dynamic = 1;
	}

//	spin_lock_init(&master->bus_lock_spinlock);
//	mutex_init(&master->bus_lock_mutex);
//	master->bus_lock_flag = 0;

	/* register the device, then userspace will see it.
	 * registration fails if the bus ID is in use.
	 */
	dev_set_name(&master->dev, "paa%u", master->bus_num);
	status = device_add(&master->dev);
	if (status < 0)
		goto done;
	dev_dbg(dev, "registered master %s%s\n", dev_name(&master->dev),
			dynamic ? " (dynamic)" : "");

	/* populate children from any paa device tables */
	//scan_boardinfo(master);
	status = 0;

	/* Register devices from the device tree */
	// of_register_paa_devices(master);
done:
	return status;
}
EXPORT_SYMBOL_GPL(paa_register_master);


static int __unregister(struct device *dev, void *null)	{
	paa_unregister_device(to_paa_device(dev));
	return 0;
}

/**
 * paa_unregister_master - unregister PAA master controller
 * @master: the master being unregistered
 * Context: can sleep
 *
 * This call is used only by PAA master controller drivers, which are the
 * only ones directly touching chip registers.
 *
 * This must be called from context that can sleep.
 */
void paa_unregister_master(struct paa_master *master)	{
	int dummy;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	dummy = device_for_each_child(&master->dev, NULL, __unregister);
	device_unregister(&master->dev);
}
EXPORT_SYMBOL_GPL(paa_unregister_master);

static int __paa_master_match(struct device *dev, void *data)	{
	struct paa_master *m;
	u16 *bus_num = data;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	m = container_of(dev, struct paa_master, dev);
	return m->bus_num == *bus_num;
}

/**
 * paa_busnum_to_master - look up master associated with bus_num
 * @bus_num: the master's bus number
 * Context: can sleep
 *
 * This call may be used with devices that are registered after
 * arch init time.  It returns a refcounted pointer to the relevant
 * paa_master (which the caller must release), or NULL if there is
 * no such master registered.
 */
struct paa_master *paa_busnum_to_master(u16 bus_num)	{
	struct device		*dev;
	struct paa_master	*master = NULL;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	dev = class_find_device(&paa_master_class, NULL, &bus_num,__paa_master_match);
	if (dev)
		master = container_of(dev, struct paa_master, dev);
	/* reference got in class_find_device */
	return master;
}
EXPORT_SYMBOL_GPL(paa_busnum_to_master);


/*-------------------------------------------------------------------------*/

static int __init paa_init(void) {
	int	status;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	status = bus_register(&paa_bus_type);
	if (status < 0)
		goto ERR0;
	status = class_register(&paa_master_class);
	if (status < 0)
		goto ERR1;
	return 0;

ERR1:
	bus_unregister(&paa_bus_type);
ERR0:
	return status;
}

static void __exit paa_exit(void) {
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	class_unregister(&paa_master_class);
	bus_unregister(&paa_bus_type);
}


/* board_info is normally registered in arch_initcall(),
 * but even essential drivers wait till later
 * REVISIT only boardinfo really needs static linking. the rest (device and
 * driver registration) _could_ be dynamically linked (modular) ... costs
 * include needing to have boardinfo data structures be much more public.
 */
module_init( paa_init );
module_exit( paa_exit );

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("paa_core");
