/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
		
	SPI-UART- Stuff

	13.07.2010	- Wenn Timeout, dann wird jetzt immer danach der nächste Slot
			genommen. Dadurch erfolgt keine Auswertung "alter" empfangener
			Telegramme
			- Gerätekonfiguration und Datenabruf erfolgen mit unterschied-
			lichen Timeout-Werten. Das ist notwendig, da offenbar bei der
			Konfiguration der externen Geräte bis zu 30ms benötigt werden	

	23.07.2010	- Timeout im Datenmodus wird gesondert behandelt. D.H wenn ein
			externes Gerät Eingänge hat, werden die Werte nicht übernommen,
			da diese verfälscht sein können. 
			Hat das Gerät nur Ausgänge bzw. zusätzlich Ausgänge, so werden
			die aktuellen Werte (vom übergeordneten Gerätetreiber	geliefert)
			immer sofort in das neue Telegramm kopiert.
				  
			Bug-Fix
			Telegrammfehler - die kein Timout sind, z.B Länge, CRC ...
			führen jetzt immer zu einer SLOT-Umschaltung. Dadurch wird verhindert,
			das fehlerhafte Empfangs-Telegramme ausgewertet werden!	

 	28.10.2010 CONFIG_HZ = 1024, da der Quarz am AT91RM92000 32.768 kHz
			Dadurch ergeben sich 1000 / 1024 = 0,9765625 ms pro Tick
			Werte die nicht Vielfache von  1000ms, 500ms, 250ms, 125ms 
			sind, haben eine Abweichung von 2,34375%. 
			Je 1ms "fehlen" immer 23,4375 ys. Wenn die Summe des Einzelab-
			weichungen > 50% einer Einzeltickzeit wird, dann ist ein 
			zusätzlicher Tick notwendig.

			Um 6ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			6 Ticks *  0,9765625	=    5,859375  ms	-0,14625 ms 

*********************************************************************************/
#ifndef MCT_PAA_SPI_H_
	#define MCT_PAA_SPI_H_

#include "../mct_types.h"
#include "mct_paa_spi_uart.h"

//#define TIMER_GRANULARITY 	(HZ/170)	// adjust ca. 5.85 ms (old)
#define TIMER_GRANULARITY 	(HZ/340)	// adjust ca. 2.47 ms

//#define TIMER_TIMEOUT_SLOW	20	// Multiplikator * Timergranularity = Zeit in ms 
//#define TIMER_TIMEOUT_FAST	5	// Multiplikator * Timergranularity = Zeit in ms
#define TIMER_TIMEOUT_SLOW	30	// Multiplikator * Timergranularity = Zeit in ms 
#define TIMER_TIMEOUT_FAST	6	// Multiplikator * Timergranularity = Zeit in ms

extern struct spi_driver spi_drv;

extern void spi_timer_function(unsigned long data);

///---------------------------------------------------------------
///	Bottom-PART
///---------------------------------------------------------------
struct lower_device {
	struct spi_device *	spi;	
	struct spi_message 	msg;
	unsigned char		write;		// aktuelle Anzahl 
	unsigned char		read;		// aktuelle Anzahl 
	struct 			spi_transfer	InitX[UART_SEQS_MAXI];
	struct 			spi_transfer	StateX[UART_SEQS_MAXI];
	struct 			spi_transfer	ErrorX[UART_SEQS_MAXI];
	struct 			spi_transfer	RxX;
	struct 			spi_transfer	TxX;
	#define DUAL_PART	SC_16IS750_FIFO_SIZE+1
	#define DUAL_RD_TX	0		// [000]	
	#define DUAL_RD_RX	DUAL_PART	// [065]
	#define DUAL_RD_CMD	DUAL_RD_TX	// [000]	RHR
	#define DUAL_RD_DATA	DUAL_RD_RX+1	// [066...]	
	#define DUAL_WR_TX	2* DUAL_PART	// [130]
	#define DUAL_WR_RX	3* DUAL_PART	// [185...] nicht notwendig!
	#define DUAL_WR_CMD	DUAL_WR_TX	// [130]	THR
	#define DUAL_WR_DATA	DUAL_WR_TX+1	// [131...]
	u8 			dual_buffer[4*DUAL_PART];
								/// UART-Steuerung
	atomic_t		uart;				// Status
	u8 			uart_cmdi [UART_SEQS_MAXI*2];	// Kommando-Antwortpuffer
	u8			*uart_cmdo;			// Kommando-Selector
	unsigned char		uart_init [UART_INIT_SIZE];	// Kommando-Space: INIT
	unsigned char		uart_state[UART_STATE_SIZE];	// Kommando-Space: STATE
	unsigned char		uart_error[UART_ERROR_SIZE];	// Kommando-Space: ERROR
};
#endif
