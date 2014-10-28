/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche
	$Id:$
	$Date:$ 	02.09.2011
	
	Description: MBUS-Configuration for mct_paa_ao4-Driver
			 	 Tracen der MBUS-Kommandos erfolgt mit:
	
				 #define/#undef CONFIG_MBUS_CMD_TRACE

	Sec-Adressen:	x=SLOT 0...5:  (unterstützt 4 virtuelle MBUS-Module)
					000008x0 == MBUS-Modul (1 analog Aussgang,und Handmaske)
					000008x1 == s.oben
					000008x2 == s.oben
					000008x3 == s.oben

	Geräte-Object  mct_if.04.2-8-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.04] 		= 4 Geräte
	[.2-8]		= MBUS, Treiberkennung mct_ao4
	[-SLOT]		= 0...5
	.0			= Instanz

	- inclusive Limitüberwachung der analogen Outputs auf 10bit
	
*********************************************************************************/
#ifndef __MCT_MBUS_PAA_AO4_CFG_H_
	#define __MCT_MBUS_PAA_AO4_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".04." BUS_MBUS "-" DRIVER_PAA_AO4_IDENT "-"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MBUS_SLVS			4	// Anzahl M-Bus Geräte
#define MBUS_SLV_OUT0		0	// Index  M-Bus Gerät 1 Ausgang + Handschaltung
#define MBUS_SLV_OUT1		1	// Index  M-Bus Gerät 1 Ausgang + Handschaltung
#define MBUS_SLV_OUT2		2	// Index  M-Bus Gerät 1 Ausgang + Handschaltung
#define MBUS_SLV_OUT3		3	// Index  M-Bus Gerät 1 Ausgang + Handschaltung

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e

#endif
