/*
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

#ifndef __LINUX_PAA_H
#define __LINUX_PAA_H

#include <linux/device.h>
// #include <linux/mod_devicetable.h>
#include <linux/slab.h>

#define PAA_BUS_NUM		(u16)(0)
#define PAA_NAME_SIZE		32
#define PAA_BUFFER_SIZE		32	
#define PAA_MODULE_PREFIX 	"paa:"

struct paa_device_id {
	char name[PAA_NAME_SIZE];
	kernel_ulong_t driver_data	/* Data private to the driver */
	__attribute__((aligned(sizeof(kernel_ulong_t))));
};

/*
 * INTERFACES between PAA master-side drivers and PAA infrastructure.
 * (There's no PAA slave support for Linux yet...)
 */
extern struct bus_type paa_bus_type;

/**
 * struct paa_device - Master side proxy for an PAA slave device
 * @dev: Driver model representation of the device.
 * @master: PAA controller used with the device.
 * @max_speed_hz: Maximum clock rate to be used with this chip
 *	(on this board); may be changed by the device's driver.
 *	The paa_transfer.speed_hz can override this for each transfer.
 * @chip_select: Chipselect, distinguishing chips handled by @master.
 * @mode: The paa mode defines how data is clocked out and in.
 *	This may be changed by the device's driver.
 *	The "active low" default for chipselect mode can be overridden
 *	(by specifying PAA_CS_HIGH) as can the "MSB first" default for
 *	each word in a transfer (by specifying PAA_LSB_FIRST).
 * @bits_per_word: Data transfers involve one or more words; word sizes
 *	like eight or 12 bits are common.  In-memory wordsizes are
 *	powers of two bytes (e.g. 20 bit samples use 32 bits).
 *	This may be changed by the device's driver, or left at the
 *	default (0) indicating protocol words are eight bit bytes.
 *	The paa_transfer.bits_per_word can override this for each transfer.
 * @irq: Negative, or the number passed to request_irq() to receive
 *	interrupts from this device.
 * @controller_state: Controller's runtime state
 * @controller_data: Board-specific definitions for controller, such as
 *	FIFO initialization parameters; from board_info.controller_data
 * @modalias: Name of the driver to use with this device, or an alias
 *	for that name.  This appears in the sysfs "modalias" attribute
 *	for driver coldplugging, and in uevents used for hotplugging
 *
 * A @paa_device is used to interchange data between an PAA slave
 * (usually a discrete chip) and CPU memory.
 *
 * In @dev, the platform_data is used to hold information about this
 * device that's meaningful to the device's protocol driver, but not
 * to its controller.  One example might be an identifier for a chip
 * variant with slightly different functionality; another might be
 * information about how this particular board wires the chip's pins.
 */
struct paa_device {
	struct device		dev;
	struct paa_master	*master;
	char			modalias[PAA_NAME_SIZE];
	u8			slot_select;
	u8			slot_mode;
	void			*controller_state;
	void			*controller_data;
};

static inline struct paa_device *to_paa_device(struct device *dev)	{
	return dev ? container_of(dev, struct paa_device, dev) : NULL;
}

/* most drivers won't need to care about device refcounting */
static inline struct paa_device *paa_dev_get(struct paa_device *paa)	{
	return (paa && get_device(&paa->dev)) ? paa : NULL;
}

static inline void paa_dev_put(struct paa_device *paa)	{
	if (paa)
		put_device(&paa->dev);
}

/* ctldata is for the bus_master driver's runtime state */
static inline void *paa_get_ctldata(struct paa_device *paa)	{
	return paa->controller_state;
}

static inline void paa_set_ctldata(struct paa_device *paa, void *state)	{
	paa->controller_state = state;
}

/* device driver data */
static inline void paa_set_drvdata(struct paa_device *paa, void *data)	{
	dev_set_drvdata(&paa->dev, data);
}

static inline void *paa_get_drvdata(struct paa_device *paa)	{
	return dev_get_drvdata(&paa->dev);
}

struct paa_message;

/**
 * struct paa_driver - Host side "protocol" driver
 * @id_table: List of PAA devices supported by this driver
 * @probe: Binds this driver to the paa device.  Drivers can verify
 *	that the device is actually present, and may need to configure
 *	characteristics (such as bits_per_word) which weren't needed for
 *	the initial configuration done during system setup.
 * @remove: Unbinds this driver from the paa device
 * @shutdown: Standard shutdown callback used during system state
 *	transitions such as powerdown/halt and kexec
 * @suspend: Standard suspend callback used during system state transitions
 * @resume: Standard resume callback used during system state transitions
 * @driver: PAA device drivers should initialize the name and owner
 *	field of this structure.
 *
 * This represents the kind of device driver that uses PAA messages to
 * interact with the hardware at the other end of a PAA link.  It's called
 * a "protocol" driver because it works through messages rather than talking
 * directly to PAA hardware (which is what the underlying PAA controller
 * driver does to pass those messages).  These protocols are defined in the
 * specification for the device(s) supported by the driver.
 *
 * As a rule, those device protocols represent the lowest level interface
 * supported by a driver, and it will support upper level interfaces too.
 * Examples of such upper levels include frameworks like MTD, networking,
 * MMC, RTC, filesystem character device nodes, and hardware monitoring.
 */
struct paa_driver {
	const struct paa_device_id *id_table;
	int		(*probe)(struct paa_device *paa);
	int		(*remove)(struct paa_device *paa);
	void		(*shutdown)(struct paa_device *paa);
	struct device_driver	driver;
};

static inline struct paa_driver *to_paa_driver(struct device_driver *drv)	{
	return drv ? container_of(drv, struct paa_driver, driver) : NULL;
}

extern int paa_register_driver(struct paa_driver *sdrv);

/**
 * paa_unregister_driver - reverse effect of paa_register_driver
 * @sdrv: the driver to unregister
 * Context: can sleep
 */
static inline void paa_unregister_driver(struct paa_driver *sdrv)	{
	if (sdrv)
		driver_unregister(&sdrv->driver);
}


/**
 * struct paa_master - interface to PAA master controller
 * @dev: device interface to this driver
 * @bus_num: board-specific (and often SOC-specific) identifier for a
 *	given PAA controller.
 * @num_chipselect: chipselects are used to distinguish individual
 *	PAA slaves, and are numbered from zero to num_chipselects.
 *	each slave has a chipselect signal, but it's common that not
 *	every chipselect is connected to a slave.
 * @dma_alignment: PAA controller constraint on DMA buffers alignment.
 * @mode_bits: flags understood by this controller driver
 * @flags: other constraints relevant to this driver
 * @bus_lock_paanlock: paanlock for PAA bus locking
 * @bus_lock_mutex: mutex for PAA bus locking
 * @bus_lock_flag: indicates that the PAA bus is locked for exclusive use
 * @setup: updates the device mode and clocking records used by a
 *	device's PAA controller; protocol code may call this.  This
 *	must fail if an unrecognized or unsupported mode is requested.
 *	It's always safe to call this unless transfers are pending on
 *	the device whose settings are being modified.
 * @transfer: adds a message to the controller's transfer queue.
 * @cleanup: frees controller-specific state
 *
 * Each PAA master controller can communicate with one or more @paa_device
 * children.  These make a small bus, sharing MOSI, MISO and SCK signals
 * but not chip select signals.  Each device may be configured to use a
 * different clock rate, since those shared signals are ignored unless
 * the chip is selected.
 *
 * The driver for an PAA controller manages access to those devices through
 * a queue of paa_message transactions, copying data between CPU memory and
 * an PAA slave device.  For each such message it queues, it calls the
 * message's completion function when the transaction completes.
 */
struct paa_master {
struct device	dev;
	/* other than negative (== assign one dynamically), bus_num is fully
	 * board-specific.  usually that simplifies to being SOC-specific.
	 * example:  one SOC has three PAA controllers, numbered 0..2,
	 * and one board's schematics might show it using PAA-2.  software
	 * would normally use bus_num=2 for that controller. */
	s16	bus_num;
	/* max supported slots for PAA slaves */
	u16	slots_max;
	/* active slots for PAA slaves on the top of master*/
	u16	slots_top;
	/* called on setup() to build memory provided by PAA master */
	int	(*setup)(struct paa_device *paa);
	int	(*transfer)(struct paa_device *paa, struct paa_message *mesg);
	/* called on release() to free memory provided by PAA master */
	void	(*cleanup)(struct paa_device *paa);
	/* lock and mutex for PAA bus locking */
	//spinlock_t	bus_lock_spinlock;
	//struct mutex	bus_lock_mutex;
	/* flag indicating that the PAA bus is locked for exclusive use */
	//bool		bus_lock_flag;
};

static inline void *paa_master_get_devdata(struct paa_master *master)	{
	return dev_get_drvdata(&master->dev);
}

static inline void paa_master_set_devdata(struct paa_master *master, void *data)	{
	dev_set_drvdata(&master->dev, data);
}

static inline struct paa_master *paa_master_get(struct paa_master *master)	{
	if (!master || !get_device(&master->dev))
		return NULL;
	return master;
}

static inline void paa_master_put(struct paa_master *master){
	if (master)
		put_device(&master->dev);
}


/* the paa driver core manages memory for the paa_master classdev */
extern struct 	paa_master *	paa_alloc_master(struct device *host, unsigned size);
extern int 			paa_register_master(struct paa_master *master);
extern void 			paa_unregister_master(struct paa_master *master);
extern struct 	paa_master *	paa_busnum_to_master(u16 busnum);

/*---------------------------------------------------------------------------*/
/*
 * I/O INTERFACE between PAA controller and protocol drivers
 *
 * Protocol drivers use a queue of paa_messages, each transferring data
 * between the controller and memory buffers.
 * The paa_messages themselves consist of a one of read+write transfer
 * segment.
 * NOTE:  Allocation of paa_message memory is entirely up to the protocol 
 * driver, which guarantees the integrity of both (as
 * well as the data buffers) for as long as the message is queued.
 */
/**
 * struct paa_message - one single-segment PAA transaction
 * @paa: PAA device to which the transaction is queued
 * @complete: called to report transaction completions
 * @context: the argument to complete() when it's called
 * @tx_typ: tx message typ
 * @tx_len: tx message len 
 * @tx_buf*:ptr to tx data or NULL
 * @rx_typ: rx message typ
 * @rx_len: rx message len 
 * @rx_buf*:ptr to rx data or NULL
 * A @paa_message is used to execute an atomic sequence of data transfer
 * The sequence is "atomic" in the sense that no other paa_message may 
 * use that PAA bus until that sequence completes.  
 * On all systems, these messages are queued, and might complete after 
 * transactions to other devices.  Messages sent to a given paa_device 
 * are alway executed in FIFO order.
 * The code that submits an paa_message to the lower layers is responsible 
 * for managing its memory.
 * Zero-initialize every field you don't set up explicitly, to
 * insulate against future API updates.  After you submit a message
 * ignore them until its completion callback.
 */
struct paa_message {
	struct 		paa_device	*paa;
	void		(*complete)(void *context);
	void		*context;
	struct 		list_head	transfer_item;
	spinlock_t	*lock;
	// TX-Part
	u8		tx_typ;
	unsigned	tx_len;
	void		*tx_buf;
	// RX-Part
	u8		rx_typ;
	unsigned	rx_len;
	const void	*rx_buf;
};

static inline void paa_message_init(struct paa_message *m,\
	spinlock_t * lock,\
	u8 *		tx,\
	u8 *		rx) {
	memset(m, 0, sizeof *m);
	m->lock		= lock;
	m->tx_buf 	= tx; 
	m->rx_buf	= rx;
}

/* It's fine to embed message and transaction structures in other data
 * structures so long as you don't free them while they're in use. */
static inline struct paa_message *paa_message_alloc(gfp_t flags, unsigned len) {
	struct paa_message *m;
	m = kzalloc(sizeof(struct paa_message),flags);
	if (m) {
		if(len > PAA_BUFFER_SIZE) {
			len = PAA_BUFFER_SIZE;
		}
		m->tx_buf = kzalloc(len , flags);
		if (!m->tx_buf) {
			return m;
		}
		m->rx_buf = kzalloc(len , flags);
		if (!m->rx_buf) {
			kfree(m->tx_buf);
			kfree(m);
			return m;
		}
	}
	return m;
}


static inline void paa_message_free(struct paa_message *m) {
	kfree(m->tx_buf);
	kfree(m->rx_buf);	
	kfree(m);
}

/**
 * paa_setup - setup PAA mode and clock rate
 * @paa: the device whose settings are being modified
 * Context: can sleep, and no requests are queued to the device
 *
 * PAA protocol drivers may need to update the transfer mode if the
 * device doesn't work with its default.  They may likewise need
 * to update clock rates or word sizes from initial values.  This function
 * changes those settings, and must be called from a context that can sleep.
 */
static inline int paa_setup(struct paa_device *paa) {
	return paa->master->setup(paa);
}


/**
 * paa_async - asynchronous PAA transfer
 * @paa: device with which data will be exchanged
 * @message: describes the data transfer, including completion callback
 * Context: any (irqs may be blocked, etc)
 *
 * This call may be used in_irq and other contexts which can't sleep,
 * as well as from task contexts which can sleep.
 * The completion callback is invoked in a context which can't sleep.
 * Before that invocation, the value of message->status is undefined.
 * When the callback is issued, message->status holds either zero (to
 * indicate complete success) or a negative error code.  After that
 * callback returns, the driver which issued the transfer request may
 * deallocate the associated memory; it's no longer in use by any PAA
 * core or controller driver code.
 * Note that although all messages to a PAA device are handled in
 * FIFO order, messages may go to different devices in other orders.
 * Some device might be higher priority, or have various "hard" access
 * time requirements, for example.
 * On detection of any fault during the transfer, processing of
 * the entire message is aborted, and the device is deselected.
 * Until returning from the associated message completion callback,
 * no other paa_message queued to that device will be processed.
 * (This rule applies equally to all the synchronous transfer calls,
 * which are wrappers around this core asynchronous primitive.)
 */
static inline int paa_async(struct paa_device *paa, struct paa_message *message) {
	message->paa = paa;
	return paa->master->transfer(paa, message);
}


/*
 * INTERFACE between board init code and PAA infrastructure.
 * No PAA driver ever sees these PAA device table segments, but
 * it's how the PAA core (or adapters that get hotplugged) grows
 * the driver model tree.
 *
 * As a rule, PAA devices can't be probed.  Instead, board init code
 * provides a table listing the devices which are present, with enough
 * information to bind and set up the device's driver.  There's basic
 * support for nonstatic configurations too; enough to handle adding
 * parport adapters, or microcontrollers acting as USB-to-PAA bridges.
 */

/**
 * struct paa_board_info - board-specific template for a PAA device
 * @modalias: Initializes paa_device.modalias; identifies the driver.
 * @platform_data: Initializes paa_device.platform_data; the particular
 *	data stored there is driver-specific.
 * @controller_data: Initializes paa_device.controller_data; some
 *	controllers need hints about hardware setup, e.g. for DMA.
 * @bus_num: Identifies which paa_master parents the PAA device; unused
 *	by paanew_device(), and otherwise depends on board wiring.
 * @slot_select: Initializes paa_device.slot_select; depends on how
 *	the board is wired.
 * @mode: Initializes paa_device.mode; based on the SLOW/FAST PAA device
 *
 * When adding new PAA devices to the device tree, these structures serve
 * as a partial device template.  They hold information which can't always
 * be determined by drivers.  Information that probe() can establish (such
 * as the default transfer wordsize) is not included here.
 *
 * These structures are used in two places.  Their primary role is to
 * be stored in tables of board-specific device descriptors, which are
 * declared early in board initialization and then used (much later) to
 * populate a controller's device tree after the that controller's driver
 * initializes.  A secondary (and atypical) role is as a parameter to
 * paa_new_device() call, which happens after those controller drivers
 * are active in some dynamic board configuration models.
 */
struct paa_board_info {
	/* the device name and module name are coupled, like platform_bus;
	 * "modalias" is normally the driver name.
	 * platform_data goes to paa_device.dev.platform_data,
	 * controller_data goes to paa_device.controller_data,
	 */
	char		modalias[PAA_NAME_SIZE];
	const void	*platform_data;
	void		*controller_data;
	/* bus_num is board specific and matches the bus_num of some
	 * paa_master that will probably be registered later. */
	u16		bus_num;
	/* slot_select reflects how this chip is wired to that master */
	u16		slot_select;
	u8		slot_mode;
	/* ... may need additional paa_device config data here. */
};

/* If you're hotplugging an adapter with devices (parport, usb, etc)
 * use paa_new_device() to describe each device.  You can also call
 * paa_unregister_device() to start making that device vanish, but
 * normally that would be handled by paa_unregister_master().
 *
 * You can also use paa_alloc_device() and paa_add_device() to use a two
 * stage registration sequence for each paa_device.  This gives the caller
 * some more control over the paa_device structure before it is registered,
 * but requires that caller to initialize fields that would otherwise
 * be defined using the board info.
 */
extern struct 	paa_device *	paa_alloc_device(struct paa_master *master);
extern int			paa_add_device(struct paa_device *paa);
extern struct 	paa_device *	paa_new_device(struct paa_master *, struct paa_board_info *);

static inline void		paa_unregister_device(struct paa_device *paa)	{
	if (paa)
		device_unregister(&paa->dev);
}

extern const struct paa_device_id * paa_get_device_id(const struct paa_device *sdev);

#endif /* __LINUX_PAA_H */
