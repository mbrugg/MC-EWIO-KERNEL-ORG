/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$


*********************************************************************************/
#ifndef MCT_PAA_SPI_UART_H_
	#define MCT_PAA_SPI_UART_H_

#include <linux/platform_device.h>
#include <linux/spi/spi.h>	

#include "../ics/mct_spi_sc16is750.h"

// Besonderheiten beim Labormuster (USA-Platine): 
//	1. SPI quarz frequenz = 14.7456Mhz
#define MHZ09600	0x60
#define MHZ19200	0x30
#define MHZ38400	0x18
#define MHZ57600	0x10
#define MHZ115200	0x08


// GEHT nicht!
// Prototype: 4.19Mhz
// #define MHZ09600	0x1B
// 4.190.000 : (115200 * 16) = 2,2732204861...
// Fehlertol. (2,2732204861 * 100) : 2 = 113,661024 dh 13% Abweichung!
//#define MHZ115200	0x02


#define UART_RETRIEVE_MAX	10	

#define UART_INIT		0
#define UART_INIT_CPL_REQ	1
#define UART_INIT_CPL_ACK	2
#define UART_PUMP		3
#define UART_PUMP_CPL_REQ	4
#define UART_RUN		5
#define UART_ERROR		7

#define UART_SEQS_MAXI		11

#define	UART_INIT_SIZE		22
#define	UART_STATE_SIZE		8
#define	UART_ERROR_SIZE		8

extern unsigned char paa_spi_uart_init[UART_INIT_SIZE];
extern unsigned char paa_spi_uart_state[UART_STATE_SIZE];
extern unsigned char paa_spi_uart_error[UART_ERROR_SIZE];

extern void paa_spi_uart_transfer_preinit( struct spi_transfer TF[], \
		unsigned char cmdo[],\
		unsigned int  cmdo_size,\
		unsigned char rx[]);

#endif
