/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#include <linux/platform_device.h>
#include "mct_spi_aio_uart.h"	

#define DLL_INIT		MHZ09600
#define DLM_INIT		0x00
#define SPR_INIT		0x55
#define EFR_INIT		0x10
#define IER_INIT		0x00
#define IOCTRL_INIT		0x00
#define IOINTMSK_INIT	0x00
#define IODIR_INIT		0x00 	// 0=Eingang, 1= Ausgang

/// SC16IS750-HARDWARE
/// UART-INIT-TEMPLATE
unsigned char 	uart_tlg_init[26] = {		\
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
	(IOCTRL | REG_WR),	IOCTRL_INIT,\
	(IOINTMSK | REG_WR),IOINTMSK_INIT,	\
	(IODIR | REG_WR),	IODIR_INIT,	\
};

/// UART-RETRIEVE-TEMPLATE
// zurücklesen der Einstellungen!
unsigned char 	uart_tlg_retrieve[22] = {		\
	(LCR | REG_WR),		DLAB,		\
	(DLL | REG_RD),		0x00,	\
	(DLM | REG_RD),		0x00,		\
	(LCR | REG_WR),		0xBF,		\
	(EFR | REG_RD),		0x00,		\
	(LCR | REG_WR),		0x03,		\
	(SPR | REG_RD),		0x00,		\
	(IER | REG_RD),		0x00,		\
	(IOCTRL | REG_RD),	0x00,		\
	(IOINTMSK | REG_RD),0x00,		\
	(IODIR | REG_RD),	0x00,		\
};

/// Vergleichsmuster der Register nach einer UART-INIT/RETRIEVE
unsigned char uart_reg_pattern[UART_PATTERN_SIZE] = {	\
	(DLL_INIT),\
	(DLM_INIT),\
	(EFR_INIT),\
	(SPR_INIT),\
	(IER_INIT),\
	(IOCTRL_INIT),\
	(IOINTMSK_INIT),\
	(IODIR_INIT),\
};

// BugFix 08.07.2010
/// UART-STATE-TEMPLATE
unsigned char  uart_tlg_state[12] = {	\
	(IOSTATE | REG_RD )	,0x00,		\
	(IOSTATE | REG_WR )	,0x00,		\
	(IOSTATE | REG_RD )	,0x00,		\
	(LSR | REG_RD )		,0x00,		\
	RXFIFO				,0x00,		\
	TXFIFO				,0x00,		\
};

/// UART-ERROR-TEMPLATE
unsigned char  uart_tlg_error[12] = {	\
	(FCR | REG_WR )		,0x00,		\
	(IOSTATE | REG_WR )	,0x00,		\
	(IOSTATE | REG_RD )	,0x00,		\
	(LSR | REG_RD )		,0x00,		\
	RXFIFO				,0x00,		\
	TXFIFO				,0x00,		\
};


void uart_transfer_preinit( struct spi_transfer TF[], unsigned char tx[], unsigned char rx[], unsigned char cmds[], unsigned int cmds_size) {
	unsigned int _i;
	for(_i = 0; _i< (cmds_size/2); _i++) {
		TF[_i].len = 2;							// sequence-länge
		TF[_i].cs_change = 1;
		TF[_i].tx_buf = &tx[_i*2];				// Bufferverweise
		TF[_i].rx_buf = &rx[_i*2];
	}
	TF[(cmds_size/2)-1].cs_change = 0;
}
