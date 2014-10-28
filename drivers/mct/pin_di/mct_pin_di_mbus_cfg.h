/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		25.08.2011
	
	Description:  MBUS-Configuration for mct_pin_di-Driver
		      Tracen der MBUS-Kommandos erfolgt mit:
	
	 #define/#undef CONFIG_MBUS_CMD_TRACE 

	Sec-Adressen:	x=MBUS-Instanz 0...3:	(unterstützt 1 virtuelles MBUS-Modul)
			000000x0 == MBUS-Modul 	(SO- oder digitaler Eingang)

	Geräte-Object mct_if.01.2-0-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.2-0]		= MBUS, Driverkennung mct_pin_di
	[-SLOT]		= 0...3
	.0		= Instanz

*********************************************************************************/
#ifndef __MCT_MBUS_PIN_DI_CFG_H_
	#define __MCT_MBUS_PIN_DI_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MBUS "-" DRIVER_PIN_DI_IDENT "-"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos


#define MBUS_SLVS		1	// Anzahl M-Bus Geräte
#define MBUS_SLV_IN		0	// Index  M-Bus Gerät (1xS0 oder 1xEingang)

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e


#endif
