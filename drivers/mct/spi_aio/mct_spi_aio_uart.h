/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#ifndef MCT_SPI_AIO_UART_H_
	#define MCT_SPI_AIO_UART_H_

#include <linux/spi/spi.h>
#include "../ics/mct_spi_sc16is750.h"

/* baud		1.8432Mhz	3.072Mhz	4.19Mhz		14.456Mhz	14.7465Mhz
----------------------------------------------------------------------------------------------
   115200	0x01 (0)	----		----		0x08 (k.A)	0x08 (0.0061)
    57600	0x02 (2.86)	----		----		0x10 (k.A)	0x10 (0.0061)
    38400	0x03 (0)	0x05 (0)	----		0x18 (k.A)	0x18 (0.0069)	
    19200	0x06 (0)	0x10 (0)	----		0x30 (k.A.)	0x30 (k.A.)		
     9600	0x0c (0)	0x20 (0)	0x1B (1.03)	----		0x60 (k.A.)		
*/

/*
// Besonderheiten beim Labormuster (USA-Platine):  
//	1. SPI quarz freuenz = 14.7456Mhz
//	2. IN-Keys Belegung ist anders!
//	3. LED's schalten gegen Masse!
//1.
#define MHZ38400		0x18
#define MHZ19200		0x30
#define MHZ09600		0x60
*/

// Prototype: 4.19Mhz
#define MHZ09600		0x1B

#define UART_RETRIEVE_MAX	10

#define UART_INIT		0
#define UART_INIT_CPL_REQ	1
#define UART_INIT_CPL_ACK	2
#define UART_RETRIEVE_CPL_REQ	3
#define UART_RETRIEVE_CPL_ACK	4
#define UART_PUMP		5
#define UART_PUMP_CPL_REQ	6
#define UART_RUN		7
#define UART_ERROR		8

#define UART_SEQS_MAXI		13
#define UART_PATTERN_SIZE	8

extern unsigned char uart_tlg_init[26];
extern unsigned char uart_tlg_retrieve[22];
extern unsigned char uart_tlg_state[12];
extern unsigned char uart_tlg_error[12];

extern unsigned char uart_reg_pattern[UART_PATTERN_SIZE];

void uart_transfer_preinit( struct spi_transfer TF[], \
	unsigned char tx[],\
	unsigned char rx[],\
	unsigned char cmds[],\
	unsigned int  cmds_size);

#endif
