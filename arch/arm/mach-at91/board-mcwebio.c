/*
 * linux./arch/arm/mach-at91/board-mcwebio.c
 *
 *  Copyright (C) 2009 Karl-Heinz Weber, MC Technilogy GmbH
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <asm/hardware.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/arch/board.h>
#include <asm/arch/gpio.h>
*/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
//#include <linux/mtd/physmap.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/at91rm9200_mc.h>


#ifdef CONFIG_ARM_AT91_KS8x93
#define USE_KS8x93
#elif CONFIG_ARM_AT91_ETHER
#define USE_AT91_ETHER
#endif

//#define USE_KS8x93
//#define USE_AT91_ETHER

#ifdef USE_KS8x93
#include <mach/at91_ks8x93_device.h>
#endif

#include "generic.h"


/*
 * Serial port configuration.
 *    0 .. 3 = USART0 .. USART3
 *    4      = DBGU
 */
//static struct at91_uart_config __initdata mcwebio_uart_config = {
//
//	.console_tty	= 0,				/* ttyS0 */
//	.nr_tty		= 2,
//	.tty_map	= { 4, 1, -1, -1, -1 }		/* ttyS0, ..., ttyS4 */
////	.console_tty	= 0,				/* ttyS0 */
////	.nr_tty		= 5,
////	.tty_map	= { 4, 0, 1, 2, 3 }		/* ttyS0, ..., ttyS4 */
//};

static void __init mcwebio_map_io(void)
{
	/* Initialize processor: 12.0 MHz crystal */
	at91rm9200_initialize(12000000, AT91RM9200_PQFP);

	/* Setup the LEDs */
	at91_init_leds(AT91_PIN_PC3, AT91_PIN_PC2);

	/* DGBU on ttyS0. (Rx & Tx only) */
	at91_register_uart(0, 0, 0);

	/* USART0 on ttyS2. (Rx, Tx, CTS) */
	at91_register_uart(AT91RM9200_ID_US0, 2, ATMEL_UART_CTS);

	/* USART1 on ttyS1. (Rx, Tx, CTS, RTS, DTR, DSR, DCD, RI) */
	at91_register_uart(AT91RM9200_ID_US1, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
			   | ATMEL_UART_DTR | ATMEL_UART_DSR | ATMEL_UART_DCD
			   | ATMEL_UART_RI);

	/* set serial console to ttyS0 (ie, DBGU) */
	at91_set_serial_console(0);
	
	//flash bug Adresspin 25 as io
	at91_set_gpio_output(AT91_PIN_PC8,0);
	at91_set_B_periph(AT91_PIN_PA5, 0);		/* TXD3 */
	at91_set_A_periph(AT91_PIN_PA5, 1);		/* NPCS2 */
}




/*
static void __init mcwebio_map_io(void)
{
	// Initialize processor: 12.0 MHz crystal 
	at91rm9200_initialize(12000000, AT91RM9200_PQFP);

	// Setup the LEDs
	at91_init_leds(AT91_PIN_PC3, AT91_PIN_PC2);

	// Setup the serial ports and console 
	at91_init_serial(&mcwebio_uart_config);

	at91_set_gpio_output(AT91_PIN_PC8,0);

at91_set_B_periph(AT91_PIN_PA5, 0);		// TXD3 
at91_set_A_periph(AT91_PIN_PA5, 1);		// NPCS2 
}*/

static void __init mcwebio_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

#ifdef USE_AT91_ETHER
static struct at91_eth_data __initdata mcwebio_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC13,
	.is_rmii	= 0,
};
#endif
#ifdef USE_KS8x93
static struct at91_ks8x93_data __initdata ks8x93_data = {
	.reset_pin	= AT91_PIN_PC15,
	.cfg_sel_pin0	= AT91_PIN_PC14,
	.is_smi		= 1,
	.is_rmii	= 0,
};
#endif

static struct at91_usbh_data __initdata mcwebio_usbh_data = {
	.ports		= 1,
};

static struct at91_udc_data __initdata mcwebio_udc_data = {
	.vbus_pin     = AT91_PIN_PB23,
	.pullup_pin   = AT91_PIN_PB25,
//	.vbus_pin	= AT91_PIN_PC5,
//	.pullup_pin	= AT91_PIN_PB17,
};

static struct at91_mmc_data __initdata mcwebio_mmc_data = {
	.det_pin	= AT91_PIN_PC5,
	.slot_b		= 0,
//	.wire4		= 1,
//	.wp_pin		= AT91_PIN_PC4,
};



// .mode = SPI_MODE_0,
/* MC-Technology */
static struct spi_board_info mcwebio_spi_devices[] __initdata = {
	{	// serial-parallel 3x8bit 
		.modalias = "74hc594_74hc165",
		.bus_num = 0,
		.chip_select = 0,
		.max_speed_hz = 1000 * 1000,
	},
	{	// uart-spi (64byte send/rec-buffer)  
		.modalias = "sc16is750",
		.bus_num = 0,
		.chip_select = 1,
		.max_speed_hz = 4000 * 1000,
		.mode = SPI_MODE_0,
	},
	{	// uart-spi (64byte send/rec-buffer)  
		.modalias = "sc16is740",
		.bus_num = 0,
		.chip_select = 2,
		.max_speed_hz = 4000 * 1000,
		.mode = SPI_MODE_0,
	},
};

static struct i2c_board_info __initdata mcwebio_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pcf8563", 0x51),
	}
};


static void __init mcwebio_board_init(void)
{
	/* Serial */
	at91_add_device_serial();
#ifdef USE_AT91_ETHER
	/* Ethernet */
	at91_add_device_eth(&mcwebio_eth_data);
#endif
#ifdef USE_KS8x93
	/* Ethernet (KS8x93 driver) */
	at91_add_device_ks8x93(&ks8x93_data);
#endif
	/* USB Host */
	at91_add_device_usbh(&mcwebio_usbh_data);
	/* USB Device */
	at91_add_device_udc(&mcwebio_udc_data);
	/* I2C */
	//at91_add_device_i2c(NULL,0);
	at91_add_device_i2c(mcwebio_i2c_devices, ARRAY_SIZE(mcwebio_i2c_devices));
	/* SPI */
	at91_add_device_spi(mcwebio_spi_devices, ARRAY_SIZE(mcwebio_spi_devices));
	/* MMC */
	at91_add_device_mmc(0,&mcwebio_mmc_data);
}

MACHINE_START(MCWEBIO, "MC Technology MCWEBIO")
	/* Maintainer: Karl-Heinz Weber */
//	.phys_io	= AT91_BASE_SYS,
//	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= mcwebio_map_io,
	.init_irq	= mcwebio_init_irq,
	.init_machine	= mcwebio_board_init,
MACHINE_END
