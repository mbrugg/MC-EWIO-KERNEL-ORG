/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

	Analoger-Input/Output DRIVER mit 
	---------------------------------------------------------
	4 Input-Devices	 (je 32 bit)
	---------------------------------------------------------
	0-1 Spannung	(0...10 V), Widerstand (40 - 4M Ohm)
	2-3 Strom		(0...20 mA)	
	
	---------------------------------------------------------
	4 Output-Devices (je 16 bit, 10/11 bit genutzt)
	---------------------------------------------------------
	0-1 Spannung	(0...10.23 V) 	10bit Auflösung
	2-3 Strom		(0...20.47 mA) 	11bit Auflösung


	06.07.2010 - Restart-Feature, wenn die UART Register nach der
			Initialisierung falsch zurückgelesen werden.
			- BugFixing 
			PIC sendet keine Antwort, dann wurde die Meldung
			"driver reset" mehrfach gesendet. Jetzt wird der Status
			_state auf e_start gesetzt! (BugFix 06.07.2010)
	08.07.2010 - Bei der State-Nachricht wurden im SPI-Telegramm
			 fehlerhafter Weise zwei aufeinanderfolgende Schreibzugriffe
			ausgeführt. Da aber nur cmdo[3] mit dem Handmode in
			der Timerroutine belegt wird, gab es alle 10ms einen 
			ca. 20ySek. LOW-Spike. 
			ALT:
			unsigned char  uart_tlg_state[12] = {	\
			(IOSTATE | REG_WR )	,0x00,		\
			(IOSTATE | REG_WR )	,0x00,		\
			(IOSTATE | REG_RD )	,0x00,		\
			.....
			NEU:
			unsigned char  uart_tlg_state[12] = {	\
			(IOSTATE | REG_RD )	,0x00,		\
			(IOSTATE | REG_WR )	,0x00,		\
			(IOSTATE | REG_RD )	,0x00,		\
			....
			Da es sich in dem SPI-Telegramm bei dem 1. Teiltelegramm
			aus Symmetriegründen nur um einen "Platzhalter" handelt
			kann hier auch ein Lesezugriff erfolgen!		
			Wichtig ist lediglich, dass an der Stelle cmdo[3] im
			Buffer der IOStatus der Ein-/Ausgänge aktualisiert wird.	
		
	28.07.2010 - BugFix: Die Handschaltung funktionierte nicht, in
			mct_spi_aio_uart.h war die Zuordnung der Kanäle
			falsch!		
	
	23.01.2012 - Handschaltung entfernt aber die Initialisierung der
			Telegramme beibehalten. (I/O) laufen als Dummy mit. 

	22.03.2012 - BugFix: Die Stomausgänge (bis 20mA) wurden falsch eingestellt,
			d.h. es wurde immer ein doppelter Wert ausgegeben! 
	
	30.05.2012 - BugFix: Die Stomausgänge (bis 20mA) wurden falsch eingestellt, wenn
			bei der Initialsierung bzw. Konfiguration und Erststart des Treibers
			ein Wert > 0 angegeben wurde.
			d.h. es wurde immer ein doppelter Wert ausgegeben! 
			
	07.08.2012 	CONFIG_HZ = 1024, da der Quarz am AT91RM92000 32.768 kHz
			Dadurch ergeben sich 1000 / 1024 = 0,9765625 ms pro Tick
			Werte die nicht Vielfache von  1000ms, 500ms, 250ms, 125ms 
			sind, haben eine Abweichung von 2,34375%. 
			Je 1ms "fehlen" immer 23,4375 ys. Wenn die Summe des Einzelab-
			weichungen > 50% einer Einzeltickzeit wird, dann ist ein 
			zusätzlicher Tick notwendig.

			Um ca. 4ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			4 Ticks *  0,9765625	=    3,90625  ms	-0,09375 ms 

			Um ca. 10 ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			10 Ticks *  0,9765625	=    9,765625  ms	-0,234375 ms 

			
*********************************************************************************/
#ifndef MCT_AIO_H_
	#define MCT_AIO_H_

#include "../mct_versions.h"

#define DRIVER_VERSION_STRING "V 01.00.10"
#define DRIVER_NAME 		"mct_spi_aio"
#define DEVICE_OUT_U_NAME 	"mct_ao.00-09." BUS_INTERN "-" DRIVER_SPI_AIO_IDENT "-0"
#define DEVICE_OUT_I_NAME 	"mct_ao.00-10." BUS_INTERN "-" DRIVER_SPI_AIO_IDENT "-0"
#define DEVICE_IN_NAME 		"mct_ai.00-31." BUS_INTERN "-" DRIVER_SPI_AIO_IDENT "-0"

#define TIMER_SPI_GRANULARITY (HZ/100)	 // adjust 9,765625 ms

// Module-Device-Debugging
#ifdef MCT_SPI_AIO_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#endif
