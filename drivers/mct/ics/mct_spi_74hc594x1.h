/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
 	
	Description:

*********************************************************************************/
/*********************************************************************************
 ### OVERWIEV of Hardware Plattform DEVICES ###

	Hardware:	Plattformname:
   	-------------------------------------
	OUT-DEVICE 0	mct_DOP.00-03.0-1-0.0 

	CHIP_0
	------------   
     ---| 74hc594  |
	----------A8
	  |||| ||||

	oBuf[0..1]
	-------------
	obuf[0]	| DUMMY |
	-------------
	obuf[1]	| CHIP_0 |
	-------------
  	
	__________CHIP_0______
   	DO 00-03  4x  Relais
   	DO 04-07  4x  LED's


	Hardware:	Plattformname:
   	-------------------------------------
	IN-DEVICE 0	mct_DI.00-07.0-1-0.0 

	CHIP_0			CHIP_1
	------------   ------------
     ---| 74hc165  |---| 74hc165  |--->
	----------E8   ---------E16
	  |||| ||||     |||| ||||

	iBuf[0..1]
	-------------
	ibuf[0] | CHIP_1	|
	-------------
	ibuf[1] | CHIP_0	|
	-------------

	______CHIP_0__________
   	DI 00-03  4x  Taster
  	DI 04-07  4x  frei

	______CHIP_1__________
   	DI 00-07  8x  Optos
	
*********************************************************************************/
#ifndef MCT_SPI_DIO_74HC594x1_
	#define MCT_SPI_DIO_74HC594x1_

/// OUTPUT-Registers
#define O_REGS		2		// Anzahl ic74hc594
#define O_REG_CHIP_0	1		// o_buf[1]	= digitale relais, LED's
#define O_REG_CHIP_1	0		// o_buf[0] = nicht bestückt!!! 
										
					// zum Register O_REG_CHIP_0
#define O_DREL_CHANNELS	4		// Anzahl digitaler Relais-Ausgänge 
#define O_DREL_REL_CHANNEL_MASK	0x0F	// 0000 1111 (binär)
#define O_DLED_CHANNELS	4		// Anzahl digitaler LED's-Ausgänge
#define O_DLED_CHANNELS_MASK	0xF0

/// INPUT-Registers
#define I_REGS		2		// Anzahl ic74hc165
#define I_REG_CHIP_0	1		// i_buf[1]	= digitale Taster-Eingänge 
#define I_REG_CHIP_1	0		// i_buf[0] = digitale Opto-Eingänge

										// zum Register I_REG_CHIP_0
#define I_DKEY_CHANNELS	4		// Anzahl digitaler Tasten-Eingänge (Initialisierung) 
#define I_DKEY_CHANNEL_MASK	0x0F	// 0000 1111 (binär)
#define I_DKEY_0	0
#define I_DKEY_1	1
#define I_DKEY_2	2
#define I_DKEY_3	3

#define DKEY_WAIT_SHORT_LIMIT 	5	// ab dieser Zeit * 10 ms gilt als SHORT-Press	
#define DKEY_WAIT_MAXI_LIMIT 	200	// maximale Zeitbegrenzung

#define DKEY_IDLE	0
#define DKEY_CLASSIFY 	1
#define DKEY_SHORT 	2
#define DKEY_EXPORT	3

// BIT-MACROS setzen, loeschen, testen auf gesetzt, geloescht
#define _setbit(d,b)	(d|=1<<b)
#define _clrbit(d,b)	(d&=~(1<<b))
#define _togbit(d,b)	((d)^=(1<<b))
#define _bitset(d,b)	(((d)&(1<<b))!=0)
#define _bitclr(d,b)	((d&1<<b)==0)

#endif
