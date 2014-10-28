/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche
	$Id: $
	$Date:$ 	01.09.2011
	
	Description: MBUS-Configuration for mct_paa_ai8-Driver
			 	 Tracen der MBUS-Kommandos erfolgt mit:
	
				 #define/#undef CONFIG_MBUS_CMD_TRACE
 
	Sec-Adressen:	x=SLOT 0...5:  (unterstützt 8 virtuelle MBUS-Module)
					000007x0 == MBUS-Modul (1 analog Eingang)
					000007x1 == s.oben
					000007x2 == s.oben
					000007x3 == s.oben
					000007x4 == s.oben
					000007x5 == s.oben
					000007x6 == s.oben
					000007x7 == s.oben
	
	Geräte-Object  mct_if.08.2-7-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.08] 		= 8 Geräte
	[.2-7]		= MBUS, Treiberkennung mct_ai8
	[SLOT]		= 0...5
	0			= Instanz

*********************************************************************************/
#ifndef __MCT_MBUS_PAA_AI8_CFG_H_
	#define __MCT_MBUS_PAA_AI8_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".08." BUS_MBUS "-" DRIVER_PAA_AI8_IDENT "-"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MBUS_SLVS			8			// Anzahl M-Bus Geräte
#define MBUS_SLV_IN0		0			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN1		1			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN2		2			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN3		3			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN4		4			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN5		5			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN6		6			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN7		7			// Index  M-Bus Gerät Eingang

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e

#endif
