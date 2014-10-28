/*
 * linux./arch/arm/mach-at91/at91_ks8x93_device.c
 *
 * Driver for Micrel's Switches KS8993 and KS8893.
 * Based on at91_ether ethernet driver.
 * MII interface to send/receive data, SPI to manage switch
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
/*
#include <linux/platform_device.h>

#include <asm/hardware.h>
#include <asm/arch/board.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_ks8x93_device.h>
*/
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>

#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/at91rm9200.h>
#include <mach/at91rm9200_mc.h>


#include "generic.h"


#if defined(CONFIG_ARM_AT91_KS8x93) || defined(CONFIG_ARM_AT91_KS8x93_MODULE)
#include <mach/at91_ks8x93_device.h>
static u64 eth_dmamask = 0xffffffffUL;
static struct at91_ks8x93_data eth_data;

static struct resource at91_ks8x93_resources[] = {
	[0] = {
		.start	= AT91_VA_BASE_EMAC,
		.end	= AT91_VA_BASE_EMAC + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91RM9200_ID_EMAC,
		.end	= AT91RM9200_ID_EMAC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91_ks8x93_device = {
	.name		= "at91_ks8x93",
	.id		= -1,
	.dev		= {
				.dma_mask		= &eth_dmamask,
				.coherent_dma_mask	= 0xffffffff,
				.platform_data		= &eth_data,
	},
	.resource	= at91_ks8x93_resources,
	.num_resources	= ARRAY_SIZE(at91_ks8x93_resources),
};

//AT91RM9200
/*
 * called from board specific initialization function
 */
void __init at91_add_device_ks8x93(struct at91_ks8x93_data *data)
{
	if (!data)
		return;

	if (data->cfg_sel_pin0)
		at91_set_gpio_output(data->cfg_sel_pin0, 1);

	if (data->reset_pin)
		at91_set_gpio_output(data->reset_pin, 1);

	at91_set_A_periph(AT91_PIN_PA16, 0);	/* EMDIO */
	at91_set_A_periph(AT91_PIN_PA15, 0);	/* EMDC */
	at91_set_A_periph(AT91_PIN_PA14, 0);	/* ERXER */
	at91_set_A_periph(AT91_PIN_PA13, 0);	/* ERX1 */
	at91_set_A_periph(AT91_PIN_PA12, 0);	/* ERX0 */
	at91_set_A_periph(AT91_PIN_PA11, 0);	/* ECRS_ECRSDV */
	at91_set_A_periph(AT91_PIN_PA10, 0);	/* ETX1 */
	at91_set_A_periph(AT91_PIN_PA9, 0);	/* ETX0 */
	at91_set_A_periph(AT91_PIN_PA8, 0);	/* ETXEN */
	at91_set_A_periph(AT91_PIN_PA7, 0);	/* ETXCK_EREFCK */

	if (!data->is_rmii) {
		at91_set_B_periph(AT91_PIN_PB19, 0);	/* ERXCK */
		at91_set_B_periph(AT91_PIN_PB18, 0);	/* ECOL */
		at91_set_B_periph(AT91_PIN_PB17, 0);	/* ERXDV */
		at91_set_B_periph(AT91_PIN_PB16, 0);	/* ERX3 */
		at91_set_B_periph(AT91_PIN_PB15, 0);	/* ERX2 */
		at91_set_B_periph(AT91_PIN_PB14, 0);	/* ETXER */
		at91_set_B_periph(AT91_PIN_PB13, 0);	/* ETX3 */
		at91_set_B_periph(AT91_PIN_PB12, 0);	/* ETX2 */
	}

	eth_data = *data;
	printk("at91_ks8x93: Adding device ... ");
	platform_device_register(&at91_ks8x93_device);
	printk("added\n");
}
#else
void __init at91_add_device_ks8x93(struct at91_ks8x93_data *data) {}
#endif

