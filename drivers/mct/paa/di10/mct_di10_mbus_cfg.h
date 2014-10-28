/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

	Description: MBUS-Configuration for mct_paa_di10-Driver
			 	 Tracen der MBUS-Kommandos erfolgt mit:
	
				 #define/#undef CONFIG_MBUS_CMD_TRACE 

	Sec-Adressen:	x=SLOT 0...5:  (unterstützt 1 virtuelles MBUS-Modul)
					000004x0 == MBUS-Modul (10 digitale Eingänge)

	Geräte-Object  mct_if.01.2-4-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.2-4]		= MBUS, Treiberkennung mct_paa_di10
	[-SLOT]		= 0...5
	.0			= Instanz

*********************************************************************************/
#ifndef __MCT_MBUS_PAA_DI10_CFG_H_
	#define __MCT_MBUS_PAA_DI10_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MBUS "-" DRIVER_PAA_DI10_IDENT "-"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MBUS_SLVS			1		// Anzahl M-Bus Geräte
#define MBUS_SLV_IN			0		// Index  M-Bus Gerät Eingänge

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e

#endif