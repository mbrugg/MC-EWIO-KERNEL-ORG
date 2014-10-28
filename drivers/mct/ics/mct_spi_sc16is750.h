/*********************************************************************************

 Copyright MC-Technology GmbH 2009

 Autor:	Dipl.-Ing. Steffen Kutsche		

*********************************************************************************/
/// SC16IS750-HARDWARE
#ifndef MCT_SPI_SC16IS750_
	#define MCT_SPI_SC16IS750_

#define SC_16IS750_FIFO_SIZE	0x40
#define TX_FIFO_SIZE		SC_16IS750_FIFO_SIZE
#define RX_FIFO_SIZE		SC_16IS750_FIFO_SIZE
#define	REG_RD			0x80			// to read register
#define	REG_WR			0x00			// to write register
#define DLAB			0x80			// Way to swap mem banks

// internal registers
#define THR		 	(0x00<<3)		// Transmit FIFO	W
#define RHR			((0x00<<3)| REG_RD)	// Receive FIFO		R
#define IER			(0x01<<3)		// Interrupt Enable	R/W
#define FCR			(0x02<<3)		// FIFO control		W
#define IIR			((0x02<<3)| REG_RD)	// Interrupt Status	R
#define LCR			(0x03<<3)		// Line Control		R/W
#define MCR			(0x04<<3)		// Modem Control	R/W

// LSR-Register
#define LSR			(0x05<<3)		// Line Status		R/W
#define LSR_ERR_GENERAL		(1<<7)			// 1= FiFo data error (parity,overrun,frame,break)
#define LSR_TX_DATA		(1<<6)			// 1= transm. hold (THR) and shift reg. (TSR) empty
#define LSR_TX_HOLD		(1<<5)			// 1= transmit hold (THR) not empty
#define LSR_BREAK_INTERRUPT	(1<<4)			// 1= break codition occured
#define LSR_ERR_FRAME		(1<<3)			// 1= RX-FiFo frame error (no valid stop-bit)
#define LSR_ERR_PARITY		(1<<2)			// 1= RX-FiFo parity error
#define LSR_ERR_OVERRUN		(1<<1)			// 1= overrun error	
#define LSR_RX_DATA		(1<<0)			// 1= data in RX-FiFo
// LSR-ERR-Bits
#define LSR_ERR_FLAGS		(LSR_ERR_GENERAL | LSR_ERR_FRAME | LSR_ERR_PARITY | LSR_ERR_OVERRUN)

#define MSR			(0x06<<3)		// Modem Status	 	R/W
#define SPR			(0x07<<3)		// Scratch Pad	 	R/W
#define TXFIFO			((0x08<<3) | REG_RD)	// TX FIFO	 	R
#define RXFIFO			((0x09<<3) | REG_RD)	// RX FIFO		R

// IODIR-Register
#define IODIR			(0x0A<<3)		// IO Direction Control R/W
#define IODIR_CHANNELS		8
#define IODIR_CHAN_7		(1<<7)
#define IODIR_CHAN_6		(1<<6)
#define IODIR_CHAN_5		(1<<5)
#define IODIR_CHAN_4		(1<<4)
#define IODIR_CHAN_3		(1<<3)
#define IODIR_CHAN_2		(1<<2)
#define IODIR_CHAN_1		(1<<1)
#define IODIR_CHAN_0		(1<<0)
#define IOSTATE			(0x0B<<3)		// IO State		R/W
#define IOINTMSK		(0x0C<<3)		// IO Interrupt Mask	R/W
#define IOCTRL			(0x0E<<3)		// IO Control 		R/W

// EFCR-Register
#define EFCR			(0x0F<<3)		// Enhanced Function Control	R/W
#define EFCR_IRDA_MODE_BIT	(1<<7)			// see doc: irda-Mode 
#define EFCR_RTSINVER_BIT	(1<<5)			// see doc: invert RTS-signal in RS485 Mode	
#define EFCR_RTSCON_BIT		(1<<4)			// see doc: enable transm. to control RTS-pin	
#define EFCR_TXDISABLE_BIT	(1<<2)			// see doc: disable transm.
#define EFCR_RXDISABLE_BIT	(1<<1)			// see doc:	disable receiver
#define EFCR_9BIT_MODE_BIT	(1<<0)			// see doc: enable 9bit-Mode or Multidrop (RS485)

// special registers, LCR.7 must be set to 1 to R/W to these registers 
#define DLL			(0x00<<3)		// Baud Rate Divisor, LSB	R/W
#define DLM			(0x01<<3)		// Baud Rate Divisor, MSR	R/W
#define EFR			(0x02<<3)		// Enhanced Function		R/W
#define XON1			(0x04<<3)		// flow control
#define XON2			(0x05<<3)		// flow control
#define XOFF1			(0x06<<3)		// flow control
#define XOFF2			(0x07<<3)		// flow control


#define LSR_TX_EMPTY(reg_lsr)	(reg_lsr & LSR_TX_DATA)		// 1=leer
#define LSR_RX_DATAS(reg_lsr)	(reg_lsr & LSR_RX_DATA)		// 1=full
#define LSR_ERROR(reg_lsr)	(reg_lsr & LSR_ERR_FLAGS)	// 0= Kein Fehler

#endif
