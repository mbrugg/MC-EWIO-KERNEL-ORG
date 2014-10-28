/*
 * linux/drivers/net/arm/at91_ks8x93.c
 *
 * Driver for Micrel's Switches KS8993 and KS8893.
 * Based on at91_ether ethernet driver.
 * 
 * MII or RMII interface to send/receive data
 * SMI or SPI interface manage switch (SPI via user
 * application that handles the SPI driver)
 *
 * (C) Copyright 2006
 * Forschungs- und Transferzentrum (FTZ) Leipzig
 * Mirco Fuchs <fuchs@ftz-leipzig.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/init.h>

//#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
//#include <asm/uaccess.h>

#include <asm/arch/at91rm9200_emac.h>
#include <asm/arch/gpio.h>
//#include <asm/arch/board.h>
#include <asm/arch/at91_ks8x93_device.h>

#include "at91_ks8x93.h"

#define DRV_NAME	"at91_ks8x93"
#define DRV_VERSION	"0.1"

#define LINK_POLL_INTERVAL	(HZ)

#if 0
if (!data->is_smi) {
		/* spi interface */
		nur interface über port setzen, wenn entpr- port pin angegeben
		wurde, ansonsten nichts aktiv setzen und von richtiger hardware
		konfiguration ausgehen
		dieser treiber darf kein neuer spi driver werden, sondern er
		muss sich als spi_device registrieren!?
		
		- spi über user-space treiber oder applikation
		- soll spi genutzt werden, ist system auf mii umzustellen
		- wichtig: spi muss dann von user verwaltet werden, insbes.
		  muss in board-xxx.c das spi-gerät initialisiert und
		  angemeldet werden -> diesbezüglich könnte es notwendig sein,
		  das spi-device vor dem ks8x9x-device anzumelden
		- es geht hier also nur darum, auf MII umzuschalten (diese
		  zugriffe müssen natürlich auch in diesem treiber realisiert
		  werden)
	}
#endif

void (*read_switch)  (u8 address, u32 *value, ...);
void (*write_switch) (u8 address, u32 value, ...);

/*
 * Read from a EMAC register.
 */
static inline unsigned long at91_emac_read(u32 reg)
{
	void __iomem *emac_base = (void __iomem *)AT91_VA_BASE_EMAC;

	return __raw_readl(emac_base + reg);
}

/*
 * Write to a EMAC register.
 */
static inline void at91_emac_write(u32 reg, u32 value)
{
	void __iomem *emac_base = (void __iomem *)AT91_VA_BASE_EMAC;

	__raw_writel(value, emac_base + reg);
}

/*
 * Wait until the SWITCH operation is complete.
 */
static inline void at91_switch_wait(void)
{
	unsigned long timeout = jiffies + 2;

	while (!(at91_emac_read(AT91_EMAC_SR) & AT91_EMAC_SR_IDLE)) {
		if (time_after(jiffies, timeout)) {
			printk("at91_ks8x93: MIO timeout\n");
			break;
		}
		cpu_relax();
	}
}

/* ........................... CONFIG INTERFACE ........................... */

/*
 * Enable the MDIO bit in MAC control register
 * When not called from an interrupt-handler, access to the PHY must be
 *  protected by a spinlock.
 */
static void enable_mdi(void)
{
	u32 ctl;

	ctl = at91_emac_read(AT91_EMAC_CTL);
	at91_emac_write(AT91_EMAC_CTL, ctl | AT91_EMAC_MPE);
}

/*
 * Disable the MDIO bit in the MAC control register
 */
static void disable_mdi(void)
{
	u32 ctl;

	ctl = at91_emac_read(AT91_EMAC_CTL);
	at91_emac_write(AT91_EMAC_CTL, ctl & ~AT91_EMAC_MPE);
}

/*
 * Write value to the a SWITCH register, use MIIM interface
 * Note: MDI interface is assumed to already have been enabled.
 */
static void write_miim(u8 address, u32 value, u8 phy_addr)
{
	at91_emac_write(AT91_EMAC_MAN, AT91_EMAC_MAN_802_3 | AT91_EMAC_RW_W
		| ((phy_addr & 0x1f) << 23) | (address << 18) | (value &
		AT91_EMAC_DATA));

	/* Wait until IDLE bit in Network Status register is cleared */
	at91_switch_wait();
}

/*
 * Read value stored in a SWITCH register, use MIIM interface
 * Note: MDI interface is assumed to already have been enabled.
 */
static void read_miim(u8 address, u32 *value, u8 phy_addr)
{
	at91_emac_write(AT91_EMAC_MAN, AT91_EMAC_MAN_802_3 | AT91_EMAC_RW_R
		| ((phy_addr & 0x1f) << 23) | (address << 18));

	/* Wait until IDLE bit in Network Status register is cleared */
	at91_switch_wait();

	*value = at91_emac_read(AT91_EMAC_MAN) & AT91_EMAC_DATA;
}

/*
 * Write value to the a SWITCH register, use MIIM interface
 * Note: MDI interface is assumed to already have been enabled.
 */
static void write_smi_ks8893(u8 address, u32 value)
{
	u8 phy_addr = 0;

	phy_addr = ((address >> 5) & 0x07);

	at91_emac_write(AT91_EMAC_MAN, (AT91_EMAC_MAN_802_3 | AT91_EMAC_RW_W |
		((phy_addr & 0x1f) << 23) | ((address & 0x1f) << 18) |
		(value & AT91_EMAC_DATA)) & ~AT91_EMAC_RW);

	/* Wait until IDLE bit in Network Status register is cleared */
	at91_switch_wait();
}

/*
 * Read value stored in a SWITCH register, use MIIM interface
 * Note: MDI interface is assumed to already have been enabled.
 */
static void read_smi_ks8893(u8 address, u32 *value)
{
	u8 phy_addr = 0;

	phy_addr = ((address >> 5) & 0x07) | 0x10;
	
	at91_emac_write(AT91_EMAC_MAN, (AT91_EMAC_MAN_802_3 | AT91_EMAC_RW_R |
		((phy_addr & 0x1f) << 23) | ((address & 0x1f) << 18) |
		AT91_EMAC_DATA) & ~AT91_EMAC_RW);

	/* Wait until IDLE bit in Network Status register is cleared */
	at91_switch_wait();

	*value = at91_emac_read(AT91_EMAC_MAN) & AT91_EMAC_DATA;
}

/*
 * Write value to the a SWITCH register, use MIIM interface
 * Note: MDI interface is assumed to already have been enabled.
 */
static void write_smi_ks8993(u8 address, u32 value)
{
	u8 phy_addr = 0;

	phy_addr = ((address >> 2) & 0x18) | 0x04;

	at91_emac_write(AT91_EMAC_MAN, AT91_EMAC_MAN_802_3 | AT91_EMAC_RW_W |
		((phy_addr & 0x1f) << 23) | ((address & 0x1f ) << 18) |
		(value & AT91_EMAC_DATA));

	/* Wait until IDLE bit in Network Status register is cleared */
	at91_switch_wait();
}

/*
 * Read value stored in a SWITCH register, use MIIM interface
 * Note: MDI interface is assumed to already have been enabled.
 */
static void read_smi_ks8993(u8 address, u32 *value)
{
	u8 phy_addr = 0;

	phy_addr = ((address >> 2) & 0x18) | 0x04;

	at91_emac_write(AT91_EMAC_MAN, AT91_EMAC_MAN_802_3 | AT91_EMAC_RW_R |
		((phy_addr & 0x1f) << 23) | ((address & 0x1f) << 18) |
		AT91_EMAC_DATA);

	/* Wait until IDLE bit in Network Status register is cleared */
	at91_switch_wait();

	*value = at91_emac_read(AT91_EMAC_MAN) & AT91_EMAC_DATA;
}


/* ........................... Switch MANAGEMENT .......................... */

/*
 * Access the PHY to determine the current link speed and mode, and update the
 * MAC accordingly.
 * If no link or auto-negotiation is busy, then no changes are made.
 */
static void update_linkspeed(struct net_device *netdev, s32 silent)
{
//	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;
//	struct at91_ks8x93_data *board_data = pp->board_data;
	u32 p1stat, p1ctl, p2stat, p2ctl, smireg;
	u32 speed, duplex, mac_cfg;

	/* Link up, or auto-negotiation still in progress */
	read_switch (SMI_GCTL4_REG, &smireg);
	
	speed = (smireg & GC4_10BT_MODE) ? SPEED_10 : SPEED_100;
	duplex = (smireg & GC4_HD_MODE) ? DUPLEX_HALF : DUPLEX_FULL;
	
	/* disabling full duplex because at91 does not support it */
	if (smireg & GC4_FLW_CTL)
		write_switch (SMI_GCTL4_REG, (smireg & ~GC4_FLW_CTL));
	
	/* Update the MAC */
	mac_cfg = at91_emac_read(AT91_EMAC_CFG) & ~(AT91_EMAC_SPD |
		AT91_EMAC_FD);

	mac_cfg |= (speed == SPEED_100) ? AT91_EMAC_SPD : 0;
	mac_cfg |= (duplex == DUPLEX_FULL) ? AT91_EMAC_FD : 0;
	at91_emac_write(AT91_EMAC_CFG, mac_cfg);
	
	/* check link and autonegotiation on port 1 */
	read_switch (SMI_P1CTL12_REG, &p1ctl);
	read_switch (SMI_P1STAT0_REG, &p1stat);
	read_switch (SMI_P2CTL12_REG, &p2ctl);
	read_switch (SMI_P2STAT0_REG, &p2stat);

	/* check for a link on at least one port */
	if (!(p1stat & PST0_LINK) && !(p2stat & PST0_LINK)) {
		netif_carrier_off(netdev);
		if (!silent)
			printk(KERN_INFO "%s: Link down.\n",
				netdev->name);
		return;
	} else if ((p1stat & PST0_LINK) && !(p2stat & PST0_LINK)) {
		if ((p1ctl & PC12_ANEG_EN) && !(p1stat & PST0_AN_DONE))
			return;
	} else if (!(p1stat & PST0_LINK) && (p2stat & PST0_LINK)) {
		if ((p2ctl & PC12_ANEG_EN) && !(p2stat & PST0_AN_DONE))
			return;
	} else {
		if (((p1ctl & PC12_ANEG_EN) && !(p1stat & PST0_AN_DONE)) &&
			((p2ctl & PC12_ANEG_EN) && !(p2stat & PST0_AN_DONE)))
			return;
	}

	if (!silent) {
		printk(KERN_INFO "%s: Link now %i-%s\n", netdev->name, speed,
			(duplex == DUPLEX_FULL) ? "FullDuplex" : "HalfDuplex");
	}
	netif_carrier_on(netdev);
}

static void ks8x93_check_link(ulong dev_id)
{
	struct net_device *netdev = (struct net_device *) dev_id;
	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;
	ulong flags;
	
	spin_lock_irqsave(&pp->lock, flags);
	enable_mdi();
	update_linkspeed(netdev, 1);
	disable_mdi();
	spin_unlock_irqrestore(&pp->lock, flags);

	pp->check_timer.expires = jiffies + LINK_POLL_INTERVAL;
	add_timer(&pp->check_timer);
}

/* ......................... ADDRESS MANAGEMENT ........................ */

/*
 * NOTE: Your bootloader must always set the MAC address correctly before
 * booting into Linux.
 *
 * - It must always set the MAC address after reset, even if it doesn't
 *   happen to access the Ethernet while it's booting.  Some versions of
 *   U-Boot on the at91-DK do not do this.
 *
 */

static short __init unpack_mac_address(struct net_device *dev, u32 hi, u32 lo)
{
	u8 addr[6];

	addr[0] = (lo & 0x000000ff);
	addr[1] = (lo & 0x0000ff00) >> 8;
	addr[2] = (lo & 0x00ff0000) >> 16;
	addr[3] = (lo & 0xff000000) >> 24;
	addr[4] = (hi & 0x000000ff);
	addr[5] = (hi & 0x0000ff00) >> 8;

	if (is_valid_ether_addr(addr)) {
		memcpy(dev->dev_addr, &addr, 6);
		return 1;
	}
	return 0;
}

/*
 * Set the ethernet MAC address in dev->dev_addr
 */
static void __init get_mac_address(struct net_device *dev)
{
	/* Check Specific-Address 1 */
	if (unpack_mac_address(dev, at91_emac_read(AT91_EMAC_SA1H),
		at91_emac_read(AT91_EMAC_SA1L)))
		return;
	/* Check Specific-Address 2 */
	if (unpack_mac_address(dev, at91_emac_read(AT91_EMAC_SA2H),
		at91_emac_read(AT91_EMAC_SA2L)))
		return;
	/* Check Specific-Address 3 */
	if (unpack_mac_address(dev, at91_emac_read(AT91_EMAC_SA3H),
		at91_emac_read(AT91_EMAC_SA3L)))
		return;
	/* Check Specific-Address 4 */
	if (unpack_mac_address(dev, at91_emac_read(AT91_EMAC_SA4H),
		at91_emac_read(AT91_EMAC_SA4L)))
		return;
	printk(KERN_ERR "at91_ks8x93: Your bootloader did not configure a MAC "
		"address.\n");
}

/*
 * Program the hardware MAC address from dev->dev_addr.
 */
static void update_mac_address(struct net_device *dev)
{
	at91_emac_write(AT91_EMAC_SA1L, (dev->dev_addr[3] << 24) |
		(dev->dev_addr[2] << 16) | (dev->dev_addr[1] << 8) |
		(dev->dev_addr[0]));
	at91_emac_write(AT91_EMAC_SA1H, (dev->dev_addr[5] << 8) |
		(dev->dev_addr[4]));

	at91_emac_write(AT91_EMAC_SA2L, 0);
	at91_emac_write(AT91_EMAC_SA2H, 0);
}

/*
 * Store the new hardware address in dev->dev_addr, and update the MAC.
 */
static int set_mac_address(struct net_device *dev, void* addr)
{
	struct sockaddr *address = addr;

	if (!is_valid_ether_addr(address->sa_data))
		return -EADDRNOTAVAIL;
	memcpy(dev->dev_addr, address->sa_data, dev->addr_len);
	update_mac_address(dev);

	printk("%s: Setting MAC address to %02x:%02x:%02x:%02x:%02x:%02x\n",
		dev->name, dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

	return 0;
}

/* ................................ MAC ................................ */

/*
 * Initialize and start the Receiver and Transmit subsystems
 */
static void ks8x93_start(struct net_device *netdev)
{
	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;
	struct recv_buffers *rbufs_base, *rbufs_phys_base;
	s32 i;
	u32 ctl;

	rbufs_base = pp->rbufs_base;
	rbufs_phys_base = pp->rbufs_phys_base;

	for (i = 0; i < MAX_RECV_BUF_SIZE; i++) {
		rbufs_base->rb_desc[i].addr =
			(u32) &rbufs_phys_base->recv_frame[i][0];
		rbufs_base->rb_desc[i].status = 0;
	}

	/* Set the Wrap bit on the last descriptor */
	rbufs_base->rb_desc[i-1].addr |= EMAC_DESC_WRAP;

	/* Reset buffer index */
	pp->rxBuffIndex = 0;

	/* Program address of descriptor list in Rx Buffer Queue register */
	at91_emac_write(AT91_EMAC_RBQP, (u32) rbufs_phys_base);

	/* Enable Receive and Transmit */
	ctl = at91_emac_read(AT91_EMAC_CTL);
	at91_emac_write(AT91_EMAC_CTL, ctl | AT91_EMAC_RE | AT91_EMAC_TE);
}

/*
 * Open the ethernet interface
 */
static int ks8x93_open(struct net_device *netdev)
{
	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;
	u32 ctl, smi_reg;
	ulong flags;
	u32 test;

	if (!is_valid_ether_addr(netdev->dev_addr))
		return -EADDRNOTAVAIL;

	clk_enable(pp->ks8x93_clk);		/* Re-enable Peripheral clock */

	/* Clear internal statistics */
	ctl = at91_emac_read(AT91_EMAC_CTL);
	at91_emac_write(AT91_EMAC_CTL, ctl | AT91_EMAC_CSR);

	/* Update the MAC address (incase user has changed it) */
	update_mac_address(netdev);

	/* setting up time for link state changes */
	pp->check_timer.expires = jiffies + LINK_POLL_INTERVAL;
	add_timer(&pp->check_timer);

	/* Enable MAC interrupts */
	at91_emac_write(AT91_EMAC_IER, AT91_EMAC_RCOM | AT91_EMAC_RBNA |
		AT91_EMAC_TUND | AT91_EMAC_RTRY | AT91_EMAC_TCOM |
		AT91_EMAC_ROVR | AT91_EMAC_ABT);

	/* Start Switch and determine current link speed */
	spin_lock_irqsave(&pp->lock, flags);
	enable_mdi();
	read_switch(SMI_CHID1_REG, &smi_reg);
	write_switch(SMI_CHID1_REG, (smi_reg | CID1_START_SW));
	spin_unlock_irqrestore(&pp->lock, flags);
	
	test = jiffies + 500;
	while(time_before(jiffies, test)){
		cpu_relax();
	}
	

	spin_lock_irqsave(&pp->lock, flags);
	update_linkspeed(netdev, 0);
	disable_mdi();
	spin_unlock_irqrestore(&pp->lock, flags);

	ks8x93_start(netdev);
	netif_start_queue(netdev);
	return 0;
}

/*
 * Transmit packet.
 */
static int ks8x93_tx(struct sk_buff *sockbuf, struct net_device *netdev)
{
	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;

	if (at91_emac_read(AT91_EMAC_TSR) & AT91_EMAC_TSR_BNQ) {
		netif_stop_queue(netdev);

		/* Store packet information (to free when Tx completed) */
		pp->sockbuf = sockbuf;
		pp->sockbuf_len = sockbuf->len;
		pp->skb_physaddr = dma_map_single(NULL, sockbuf->data,
			sockbuf->len, DMA_TO_DEVICE);
		pp->stats.tx_bytes += sockbuf->len;
		/* Set address of the data in the Transmit Address register */
		at91_emac_write(AT91_EMAC_TAR, pp->skb_physaddr);
		/* Set length of the packet in the Transmit Control register */
		at91_emac_write(AT91_EMAC_TCR, sockbuf->len);
		netdev->trans_start = jiffies;
	} else {
		printk(KERN_ERR "at91_ks8x93: ks8x93_tx() called, but device "
			"is busy!\n");
		return 1;
		/* if we return anything but zero, dev.c:1055 calls
		 * kfree_skb(skb) on this skb, he also reports -ENETDOWN and
		 * printk's, so either we free and return(0) or don't free and
		 * return 1 
		 */
	}

	return 0;
}

/*
 * Extract received frame from buffer descriptors and sent to upper layers.
 * (Called from interrupt context)
 */
static void ks8x93_rx(struct net_device *netdev)
{
	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;
	struct recv_buffers *rbufs_base;
	u8 *p_recv;
	struct sk_buff *sockbuf;
	u32 pktlen;

	rbufs_base = pp->rbufs_base;
	while (rbufs_base->rb_desc[pp->rxBuffIndex].addr & EMAC_DESC_DONE) {
		p_recv = rbufs_base->recv_frame[pp->rxBuffIndex];
		pktlen = rbufs_base->rb_desc[pp->rxBuffIndex].status & 0x7ff;
		/* Length of frame including FCS */
		sockbuf = dev_alloc_skb(pktlen + 2);
		if (sockbuf != NULL) {
			skb_reserve(sockbuf, 2);
			memcpy(skb_put(sockbuf, pktlen), p_recv, pktlen);
			sockbuf->dev = netdev;
			sockbuf->protocol = eth_type_trans(sockbuf, netdev);
			netdev->last_rx = jiffies;
			pp->stats.rx_bytes += pktlen;
			netif_rx(sockbuf);
		}
		else {
			pp->stats.rx_dropped += 1;
			printk(KERN_NOTICE "%s: Memory squeeze, dropping "
				"packet.\n", netdev->name);
		}

		if (rbufs_base->rb_desc[pp->rxBuffIndex].status &
			EMAC_MULTICAST) 
			pp->stats.multicast++;

		rbufs_base->rb_desc[pp->rxBuffIndex].addr &= ~EMAC_DESC_DONE;
		/* reset ownership bit */
		if (pp->rxBuffIndex == MAX_RECV_BUF_SIZE - 1) {
		/* wrap after last buffer */
			pp->rxBuffIndex = 0;
		} else {
			pp->rxBuffIndex++;
		}
	}
}

/*
 * MAC interrupt handler
 */
static irqreturn_t ks8x93_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct net_device *netdev = (struct net_device *) dev_id;
	struct ks8x93_private *pp = (struct ks8x93_private *) netdev->priv;
	u32 intstatus, ctl;

	/* MAC Interrupt Status register indicates what interrupts are pending.
	   It is automatically cleared once read. */
	intstatus = at91_emac_read(AT91_EMAC_ISR);

 	if (intstatus & AT91_EMAC_RCOM) /* Receive complete */
		ks8x93_rx(netdev);
	
	if (intstatus & AT91_EMAC_TCOM) {	/* Transmit complete */
		/* The TCOM bit is set even if the transmission failed. */
		if (intstatus & (AT91_EMAC_TUND | AT91_EMAC_RTRY))
			pp->stats.tx_errors += 1;

		if (pp->sockbuf) {
			dev_kfree_skb_irq(pp->sockbuf);
			pp->sockbuf = NULL;
			dma_unmap_single(NULL, pp->skb_physaddr,
				pp->sockbuf_len, DMA_TO_DEVICE);
		}
		netif_wake_queue(netdev);
	}

	/* Work-around for Errata #11 */
	if (intstatus & AT91_EMAC_RBNA) {
		ctl = at91_emac_read(AT91_EMAC_CTL);
		at91_emac_write(AT91_EMAC_CTL, ctl & ~AT91_EMAC_RE);
		at91_emac_write(AT91_EMAC_CTL, ctl | AT91_EMAC_RE);
	}

	if (intstatus & AT91_EMAC_ROVR)
		printk("%s: ROVR error\n", netdev->name);

	return IRQ_HANDLED;
}

static int __init detect_switch_miim (void)
{
	read_switch  = (void*) read_miim;
	write_switch = (void*) write_miim;

	printk(KERN_ERR "at91_ks8x93: MIIM interface currently not "
		"supported\n");
	return 0;
}

/*
 * Change read/write functions to access Switch
 */
static int __init detect_switch_smi (void)
{
	u32 switchid0, switchid1;
	u32 switchid;
	
	/* first we check if a KS8993 is connected */

	read_switch  = (void*) read_smi_ks8993;
	write_switch = (void*) write_smi_ks8993;

	enable_mdi();
	read_switch(SMI_CHID0_REG, &switchid0);
	read_switch(SMI_CHID1_REG, &switchid1);
	disable_mdi();

	/* if this failes, we try KS8893 */
	if ((switchid0 == 0xffff) && (switchid1 = 0xffff)) {
		read_switch  = (void*) read_smi_ks8893;
		write_switch = (void*) write_smi_ks8893;
		enable_mdi();
		read_switch(SMI_CHID0_REG, &switchid0);
		read_switch(SMI_CHID1_REG, &switchid1);
		disable_mdi();
	}

	switchid = (switchid0 << 4) | ((switchid1 & 0xf0) >> 4);
	switch (switchid) {
		case SMI_ID_KS8893:
			printk("at91_ks8x93: Detected KS8893 ethernet "
				"switch\n");
			return SMI_ID_KS8893;
		case SMI_ID_KS8993:
			printk("at91_ks8x93: Detected KS8993 ethernet "
				"switch\n");
			return SMI_ID_KS8993;
		default:
			return 0;
	}
}







/*
 * Update the current statistics from the internal statistics registers.
 */
//static struct net_device_stats *at91ether_stats(struct net_device *dev)
//{
//	struct at91_private *lp = netdev_priv(dev);
//	int ale, lenerr, seqe, lcol, ecol;
//
//	if (netif_running(dev)) {
//		lp->stats.rx_packets += at91_emac_read(AT91_EMAC_OK);		/* Good frames received */
	//	ale = at91_emac_read(AT91_EMAC_ALE);
	//	lp->stats.rx_frame_errors += ale;				/* Alignment errors */
	//	lenerr = at91_emac_read(AT91_EMAC_ELR) + at91_emac_read(AT91_EMAC_USF);
	//	lp->stats.rx_length_errors += lenerr;				/* Excessive Length or Undersize Frame error */
	//	seqe = at91_emac_read(AT91_EMAC_SEQE);
	//	lp->stats.rx_crc_errors += seqe;				/* CRC error */
	//	lp->stats.rx_fifo_errors += at91_emac_read(AT91_EMAC_DRFC);	/* Receive buffer not available */
	//	lp->stats.rx_errors += (ale + lenerr + seqe
	//		+ at91_emac_read(AT91_EMAC_CDE) + at91_emac_read(AT91_EMAC_RJB));
//
	//	lp->stats.tx_packets += at91_emac_read(AT91_EMAC_FRA);		/* Frames successfully transmitted */
	//	lp->stats.tx_fifo_errors += at91_emac_read(AT91_EMAC_TUE);	/* Transmit FIFO underruns */
	//	lp->stats.tx_carrier_errors += at91_emac_read(AT91_EMAC_CSE);	/* Carrier Sense errors */
	//	lp->stats.tx_heartbeat_errors += at91_emac_read(AT91_EMAC_SQEE);/* Heartbeat error */
//
	//	lcol = at91_emac_read(AT91_EMAC_LCOL);
	//	ecol = at91_emac_read(AT91_EMAC_ECOL);
	//	lp->stats.tx_window_errors += lcol;			/* Late collisions */
	//	lp->stats.tx_aborted_errors += ecol;			/* 16 collisions */
//
	//	lp->stats.collisions += (at91_emac_read(AT91_EMAC_SCOL) + at91_emac_read(AT91_EMAC_MCOL) + lcol + ecol);
//	}
//	return &lp->stats;
//}




/*
 * Update the current statistics from the internal statistics registers.
 */
static struct net_device_stats *at91ether_stats(struct net_device *dev)
{
	struct ks8x93_private *lp;
	lp = netdev_priv(dev);
	int ale, lenerr, seqe, lcol, ecol;

//	if (netif_running(dev)) {
		//lp->stats.rx_packets = 0;		/* Good frames received */
//lp->stats.rx_packets += at91_emac_read(AT91_EMAC_OK);		/* Good frames received */
//		lp->stats.rx_frame_errors = 0;				/* Alignment errors */
//		lp->stats.rx_length_errors = 0;				/* Excessive Length or Undersize Frame error */
//		lp->stats.rx_crc_errors = 0;				/* CRC error */
//		lp->stats.rx_fifo_errors = 0;	/* Receive buffer not available */
//		lp->stats.rx_errors = 0;
//		lp->stats.tx_packets = 0;		/* Frames successfully transmitted */
//		lp->stats.tx_fifo_errors = 0;	/* Transmit FIFO underruns */
//		lp->stats.tx_carrier_errors = 0;	/* Carrier Sense errors */
//		lp->stats.tx_heartbeat_errors = 0;/* Heartbeat error */
//		lp->stats.tx_window_errors = 0;			/* Late collisions */
//		lp->stats.tx_aborted_errors = 0;			/* 16 collisions */
//		lp->stats.collisions = 0;
//	}
	if (netif_running(dev)) {
		lp->stats.rx_packets += at91_emac_read(AT91_EMAC_OK);		/* Good frames received */
		ale = at91_emac_read(AT91_EMAC_ALE);
		lp->stats.rx_frame_errors += ale;				/* Alignment errors */
		lenerr = at91_emac_read(AT91_EMAC_ELR) + at91_emac_read(AT91_EMAC_USF);
		lp->stats.rx_length_errors += lenerr;				/* Excessive Length or Undersize Frame error */
		seqe = at91_emac_read(AT91_EMAC_SEQE);
		lp->stats.rx_crc_errors += seqe;				/* CRC error */
		lp->stats.rx_fifo_errors += at91_emac_read(AT91_EMAC_DRFC);	/* Receive buffer not available */
		lp->stats.rx_errors += (ale + lenerr + seqe
			+ at91_emac_read(AT91_EMAC_CDE) + at91_emac_read(AT91_EMAC_RJB));

		lp->stats.tx_packets += at91_emac_read(AT91_EMAC_FRA);		/* Frames successfully transmitted */
		lp->stats.tx_fifo_errors += at91_emac_read(AT91_EMAC_TUE);	/* Transmit FIFO underruns */
		lp->stats.tx_carrier_errors += at91_emac_read(AT91_EMAC_CSE);	/* Carrier Sense errors */
		lp->stats.tx_heartbeat_errors += at91_emac_read(AT91_EMAC_SQEE);/* Heartbeat error */
		lcol = at91_emac_read(AT91_EMAC_LCOL);
		ecol = at91_emac_read(AT91_EMAC_ECOL);
		lp->stats.tx_window_errors += lcol;			/* Late collisions */
		lp->stats.tx_aborted_errors += ecol;			/* 16 collisions */

		lp->stats.collisions += (at91_emac_read(AT91_EMAC_SCOL) + at91_emac_read(AT91_EMAC_MCOL) + lcol + ecol);
	}
	return &lp->stats;
}





static int __init ks8x93_alloc_resources ( 
		struct platform_device *pdev, u32 switchid,
		struct clk *ks8x93_clk)
{
	struct at91_ks8x93_data *board_data = pdev->dev.platform_data;
	struct net_device *netdev;
	struct resource *res;
	struct ks8x93_private *pp;
	s32 ret;
	ulong flags;

	netdev = alloc_etherdev(sizeof(struct ks8x93_private));
	if (!netdev)
		return -ENOMEM;

	/* Get I/O base address and IRQ */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		free_netdev(netdev);
		return -ENODEV;
	}
	netdev->base_addr = res->start;
	netdev->irq = platform_get_irq(pdev, 0);
	
//	SET_MODULE_OWNER(netdev);

	/* Install the interrupt handler */
	if (request_irq(netdev->irq, ks8x93_interrupt, 0, netdev->name, 
		netdev)) {
		free_netdev(netdev);
		return -EBUSY;
	}

	/* Allocate memory for DMA Receive descriptors */
	pp = (struct ks8x93_private *) netdev->priv;
	pp->rbufs_base = (struct recv_buffers *) dma_alloc_coherent (
		NULL, sizeof(struct recv_buffers),
		(dma_addr_t *) &pp->rbufs_phys_base, GFP_KERNEL);
	if (pp->rbufs_base == NULL) {
		free_irq(netdev->irq, netdev);
		free_netdev(netdev);
		return -ENOMEM;
	}
	
	pp->board_data = board_data;
	pp->ks8x93_clk = ks8x93_clk;
	platform_set_drvdata(pdev, netdev);

	spin_lock_init(&pp->lock);
	
	ether_setup(netdev);
	netdev->open = ks8x93_open;
	netdev->stop = NULL;
	netdev->hard_start_xmit = ks8x93_tx;
	netdev->get_stats = at91ether_stats;
	netdev->set_multicast_list = NULL;
	netdev->set_mac_address = set_mac_address;
	netdev->ethtool_ops = NULL;
	netdev->do_ioctl = NULL;

	SET_NETDEV_DEV(netdev, &pdev->dev);

	/* Get ethernet address and store it in dev->dev_addr */
	get_mac_address(netdev); 
	update_mac_address(netdev); /* Program ethernet address into MAC */

	at91_emac_write(AT91_EMAC_CTL, 0);

	if (board_data->is_rmii) {
		at91_emac_write(AT91_EMAC_CFG, AT91_EMAC_CLK_DIV32 |
			AT91_EMAC_BIG | AT91_EMAC_RMII);
	} else {
		at91_emac_write(AT91_EMAC_CFG, AT91_EMAC_CLK_DIV32 |
			AT91_EMAC_BIG);
	}

	pp->switch_type = switchid;

	/* Register the network interface */
	ret = register_netdev(netdev);
	if (ret) {
		free_irq(netdev->irq, netdev);
		free_netdev(netdev);
		dma_free_coherent(NULL, sizeof(struct recv_buffers),
			pp->rbufs_base, (dma_addr_t) pp->rbufs_phys_base);
		return ret;
	}

	/* Determine current link speed */
	spin_lock_irqsave(&pp->lock, flags);
	enable_mdi();
	update_linkspeed(netdev, 1);
	disable_mdi();
	spin_unlock_irqrestore(&pp->lock, flags);
	netif_carrier_off(netdev); /* will be enabled in open() */

	/* Use a timer to poll the Switch */
	init_timer(&pp->check_timer);
	pp->check_timer.data = (u32) netdev;
	pp->check_timer.function = ks8x93_check_link;

	/* Display ethernet banner */
	printk(KERN_INFO "%s: AT91 ethernet at 0x%08x int=%d %s%s "
		"(%02x:%02x:%02x:%02x:%02x:%02x)\n",
		netdev->name, (u32) netdev->base_addr, netdev->irq,
		at91_emac_read(AT91_EMAC_CFG) & AT91_EMAC_SPD ? "100-" : "10-",
		at91_emac_read(AT91_EMAC_CFG) & AT91_EMAC_FD ? "FullDuplex" :
		"HalfDuplex",
		netdev->dev_addr[0], netdev->dev_addr[1], netdev->dev_addr[2],
		netdev->dev_addr[3], netdev->dev_addr[4], netdev->dev_addr[5]);
	
	return 0;
}

/*
 * Detect Switch and initialize configuration interface
 */
static int __init at91_ks8x93_probe(struct platform_device *pdev)
{
	struct at91_ks8x93_data *board_data = pdev->dev.platform_data;
	ulong delay;
	u32 switchid;
	s32 detected = -1;
	struct clk *ks8x93_clk;

	ks8x93_clk = clk_get(&pdev->dev, "ether_clk");
	if (IS_ERR(ks8x93_clk)) {
		printk(KERN_ERR "at91_ks8x93: no clock defined\n");
		return -ENODEV;
	}
	clk_enable(ks8x93_clk);	/* Enable Peripheral clock */

	/* selecting configuration interface (SMI or MIIM)*/
	if (board_data->is_smi && board_data->cfg_sel_pin0) {
		at91_set_gpio_value(board_data->cfg_sel_pin0, 1);
	} else if (!board_data->is_smi && board_data->cfg_sel_pin0) {
		at91_set_gpio_value(board_data->cfg_sel_pin0, 0);
	}
	
	/* resetting switch */
	if (board_data->reset_pin) {
		at91_set_gpio_value(board_data->reset_pin, 0);
		delay = jiffies + 5;
		while(time_before(jiffies, delay)){
			cpu_relax();
		}
		at91_set_gpio_value(board_data->reset_pin, 1);
		delay = jiffies + HZ/5;
		while(time_before(jiffies, delay)){
			cpu_relax();
		}
	} else {
		printk(KERN_WARNING "at91_ks8x93: Cannot reset switch\n");
	}

	if (board_data->is_smi) {
		switchid = detect_switch_smi();
	} else {
		switchid = detect_switch_miim();
	}
	
	if (!switchid) {
		printk(KERN_ERR "at91_ks8x93: Unknown ethernet device!\n");
		clk_disable(ks8x93_clk);
		return detected;
	}

	detected = ks8x93_alloc_resources(pdev, switchid, ks8x93_clk);

	clk_disable(ks8x93_clk);

	return detected;
}


#define at91_ks8x93_remove	NULL
#define at91_ks8x93_suspend	NULL
#define at91_ks8x93_resume	NULL

static struct platform_driver at91_ks8x93_driver = {
	.probe		= at91_ks8x93_probe,
	.remove		= __devexit_p(at91_ks8x93_remove),
	.suspend	= at91_ks8x93_suspend,
	.resume		= at91_ks8x93_resume,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init at91_ks8x93_init(void)
{
	return platform_driver_register(&at91_ks8x93_driver);
}

static void __exit at91_ks8x93_exit(void)
{ 
	platform_driver_unregister(&at91_ks8x93_driver);	
}

module_init(at91_ks8x93_init)
module_exit(at91_ks8x93_exit)

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KS8x93 driver for AT91 machines");
MODULE_AUTHOR("Mirco Fuchs, FTZ Leipzig");
