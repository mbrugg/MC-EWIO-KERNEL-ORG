/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

*********************************************************************************/
#include "mct_paa_spi_uart.h"

/* baud			1.8432Mhz		3.072Mhz	4.19Mhz		14.456Mhz		14.7465Mhz
-----------------------------------------------------------------------------------
   115200		0x01 (0)		----		----		0x08 (k.A)		0x08 (0.0061)
	57600		0x02 (2.86)		----		----		0x10 (k.A)		0x10 (0.0061)
	38400		0x03 (0)		0x05 (0)	----		0x18 (k.A)		0x18 (0.0069)	
	19200		0x06 (0)		0x10 (0)	----		0x30 (k.A.)		0x30 (k.A.)		
	 9600		0x0c (0)		0x20 (0)	0x1B (1.03)	----			0x60 (k.A.)		
*/

#define DLL_INIT		MHZ115200
#define DLM_INIT		0x00
#define SPR_INIT		0x55
#define EFR_INIT		0x10
#define IER_INIT		0x00
#define EFCR_INIT		(EFCR_RTSCON_BIT | EFCR_RTSINVER_BIT) 

/// SC16IS750-HARDWARE
/// UART-INIT-TEMPLATE
unsigned char 	paa_spi_uart_init[] = {		\
	(LCR | REG_WR),		DLAB,		\
	(DLL | REG_WR),		DLL_INIT,	\
	(DLM | REG_WR),		DLM_INIT,	\
	(LCR | REG_WR),		0xBF,		\
	(EFR | REG_WR),		EFR_INIT,	\
	(LCR | REG_WR),		0x03,		\
	(FCR | REG_WR),		0x06,		\
	(FCR | REG_WR),		0x01,		\
	(SPR | REG_WR),		SPR_INIT,	\
	(IER | REG_WR),		IER_INIT,	\
	(EFCR| REG_WR),		EFCR_INIT,	\
};

/// UART-STATE-TEMPLATE
unsigned char  paa_spi_uart_state[] = {	\
	(IIR | REG_RD )		,0x00,		\
	(LSR | REG_RD )		,0x00,		\
	RXFIFO				,0x00,		\
	TXFIFO				,0x00,		\
};

/// UART-ERROR-TEMPLATE
unsigned char  paa_spi_uart_error[] = {	\
	(FCR | REG_WR )		,0x00,		\
	(LSR | REG_RD )		,0x00,		\
	RXFIFO				,0x00,		\
	TXFIFO				,0x00,		\
};


void paa_spi_uart_transfer_preinit( \
	struct spi_transfer TF[], \
	unsigned char cmdo[], \
	unsigned int cmdo_size, \
	unsigned char rx[]) {
	unsigned int _i;
	
	for(_i = 0; _i< (cmdo_size/2); _i++) {
		TF[_i].len = 2;							// sequence-länge
		TF[_i].cs_change = 1;		
		TF[_i].tx_buf = &cmdo[_i*2];			// Bufferverweise
		TF[_i].rx_buf = &rx[_i*2];
	}
	TF[(cmdo_size/2)-1].cs_change = 0;
}
