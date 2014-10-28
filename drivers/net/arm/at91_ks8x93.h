/*
 * drivers/net/arm/at91_ks8x93.h
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

#ifndef AT91_KS8x93_H_
#define AT91_KS8x93_H_

#define SMI_ID_KS8893 0x882 /* Chip ID0 + Chip ID1 */
#define SMI_ID_KS8993 0x930

#define SMI_CHID0_REG	0
#define SMI_CHID1_REG	1
#define	SMI_GCTL0_REG	2
#define	SMI_GCTL1_REG	3
#define	SMI_GCTL2_REG	4
#define	SMI_GCTL3_REG	5
#define	SMI_GCTL4_REG	6
#define	SMI_GCTL5_REG	7
#define	SMI_GCTL6_REG	8
#define	SMI_GCTL7_REG	9
#define	SMI_GCTL8_REG	10
#define	SMI_GCTL9_REG	11

#define SMI_P1CTL0_REG	16
#define SMI_P1CTL1_REG	17
#define SMI_P1CTL2_REG	18
#define SMI_P1CTL3_REG	19
#define SMI_P1CTL4_REG	20
#define SMI_P1CTL5_REG	21
#define SMI_P1CTL6_REG	22
#define SMI_P1CTL7_REG	23
#define SMI_P1CTL8_REG	24
#define SMI_P1CTL9_REG	25
#define SMI_P1CTL10_REG	26
#define SMI_P1CTL11_REG	27
#define SMI_P1CTL12_REG	28
#define SMI_P1CTL13_REG	29
#define SMI_P1STAT0_REG	30
#define SMI_P1STAT1_REG	31
#define SMI_P2CTL0_REG	32
#define SMI_P2CTL1_REG	33
#define SMI_P2CTL2_REG	34
#define SMI_P2CTL3_REG	35
#define SMI_P2CTL4_REG	36
#define SMI_P2CTL5_REG	37
#define SMI_P2CTL6_REG	38
#define SMI_P2CTL7_REG	39
#define SMI_P2CTL8_REG	40
#define SMI_P2CTL9_REG	41
#define SMI_P2CTL10_REG	42
#define SMI_P2CTL11_REG	43
#define SMI_P2CTL12_REG	44
#define SMI_P2CTL13_REG	45
#define SMI_P2STAT0_REG	46
#define SMI_P2STAT1_REG	47
#define SMI_P3CTL0_REG	48
#define SMI_P3CTL1_REG	49
#define SMI_P3CTL2_REG	50
#define SMI_P3CTL3_REG	51
#define SMI_P3CTL4_REG	52
#define SMI_P3CTL5_REG	53
#define SMI_P3CTL6_REG	54
#define SMI_P3CTL7_REG	55
#define SMI_P3CTL8_REG	56
#define SMI_P3CTL9_REG	57
#define SMI_P3CTL10_REG	58
#define SMI_P3CTL11_REG	59

#define SMI_P3STAT1_REG	63

/* Chip ID1 reg. definitions */
#define CID1_CHIP_ID	15 << 4
#define CID1_REV_IF	 7 << 1
#define CID1_START_SW	 1 << 0

/* Global control reg. 4 definitions */
#define GC4_HD_MODE	1 << 6
#define GC4_FLW_CTL	1 << 5
#define GC4_10BT_MODE	1 << 4
#define GC4_VID		1 << 3
#define GC4_BCAST_RATE	7 << 0

/* Port1/2 control reg. 12 definitions */
#define PC12_ANEG_EN	1 << 7
#define PC12_FC_SPD	1 << 6
#define PC12_FC_DPX	1 << 5
#define PC12_AD_FLWCTL	1 << 4
#define PC12_AD_100_FD	1 << 3
#define PC12_AD_100_HS	1 << 2
#define PC12_AD_10_FD	1 << 1
#define PC12_AD_10_HD	1 << 0

/* Port1/2 status reg. 0 definitions */
#define PST0_MDIX	1 << 7
#define PST0_AN_DONE	1 << 6
#define PST0_LINK	1 << 5
#define PST0_FLWCTL	1 << 4
#define PST0_100_FD	1 << 3
#define PST0_100_HD	1 << 2
#define PST0_10_FD	1 << 1
#define PST0_10_HD	1 << 0




#define MAX_RECV_BUF_SIZE	1000
#define MAX_RECV_FRAME_SIZE	0x600

#define EMAC_DESC_DONE	0x00000001	/* bit for if DMA is done */
#define EMAC_DESC_WRAP	0x00000002	/* bit for wrap */

#define EMAC_BROADCAST	0x80000000	/* broadcast address */
#define EMAC_MULTICAST	0x40000000	/* multicast address */
#define EMAC_UNICAST	0x20000000	/* unicast address */

struct recv_buf_desc {
	u32 addr;
	u32 status;
};

struct recv_buffers {
	struct recv_buf_desc rb_desc[MAX_RECV_BUF_SIZE];
	s8 recv_frame [MAX_RECV_BUF_SIZE][MAX_RECV_FRAME_SIZE];
};

struct ks8x93_private {
	struct net_device_stats stats;
/*	struct mii_if_info mii;		*/	/* ethtool support */
	struct at91_ks8x93_data *board_data;	/* board-specific config */
	struct clk *ks8x93_clk;		/* clock */

	/* Switch */
	u32 switch_type;			/* type of PHY (PHY_ID) */
	spinlock_t lock;			/* lock for MDI/SMI interface */
/*	short phy_media;		*/	/* media interface type */
/*	unsigned short phy_address;	*/	/* 5-bit MDI address */
	struct timer_list check_timer;		/* Poll link status */

	/* Transmit */
	struct sk_buff *sockbuf; /* holds skb until xmit interrupt completes */
	dma_addr_t skb_physaddr; /* phys addr from pci_map_single */
	s32 sockbuf_len;	/* saved skb length for pci_unmap_single
*/

	/* Receive */
	s32 rxBuffIndex;		/* index into receive descriptor list */
	struct recv_buffers *rbufs_base;	/* descriptor list address */
	struct recv_buffers *rbufs_phys_base; /* descriptor list physical adr.
*/
};

#endif /* AT91_KS8x93_H_ */
