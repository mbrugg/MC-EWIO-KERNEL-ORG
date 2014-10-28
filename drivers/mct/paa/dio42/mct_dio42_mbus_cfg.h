/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	31.08.2011

	Description: MBUS-Configuration for mct_paa_dio42-Driver
		 	 Tracen der MBUS-Kommandos erfolgt mit:
	
				 #define/#undef CONFIG_MBUS_CMD_TRACE 

	Sec-Adressen:	x=SLOT 0...5: (unterstützt 2 virtuelle MBUS-Module)
					000006x0 == MBUS-Modul (4 dig. Eingänge)
					000006x1 == MBUS-Modul (2 dig. Ausgänge)

	Geräte-Object  mct_if.01.2-6-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.02] 		= 2 Geräte
	[.2-6]		= MBUS, Treiberkennung mct_paa_dio42
	[-SLOT]		= 0...5
	.0			= Instanz

*********************************************************************************/
#ifndef __MCT_PAA_DIO42_MBUS_CFG_H_
	#define __MCT_PAA_DIO42_MBUS_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".02." BUS_MBUS "-" DRIVER_PAA_DIO42_IDENT "-"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MBUS_SLVS			2		// Anzahl M-Bus Geräte
#define MBUS_SLV_IN			0		// Index  M-Bus Gerät Eingang
#define MBUS_SLV_OUT		1		// Index  M-Bus Gerät Ausgänge + Handschaltung
#define MBUS_SLV_OUT_BITS	0x03	// Als Ausgang zugelassen = 1

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e

#endif
