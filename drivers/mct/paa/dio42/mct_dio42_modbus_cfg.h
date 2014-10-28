/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	13.10.2011

	Description:  MODBUS-Configuration for mct_paa_dio42-Driver
		      Tracen der MODBUS-Kommandos erfolgt mit:
	
	#define/#undef CONFIG_MODBUS_CMD_TRACE

	MODBUS-Adressen:	x=SLOT 0...5:  (unterstützt 1 virtuelles MODBUS-Modul)
				TCP		RTU
				-----------------
				0		1x6	

	Geräte-Object  mct_if.01.3-6-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-6]		= MODBUS, Treiberkennung mct_paa_dio42
	[-SLOT]		= 0...5
	.0		= Instanz

*********************************************************************************/
#ifndef __MCT_PAA_DIO42_MODBUS_CFG_H_
	#define __MCT_PAA_DIO42_MODBUS_CFG_H_	

#define MODBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MODBUS "-" DRIVER_PAA_DIO42_IDENT "-"

//#define CONFIG_MODBUS_CMD_TRACE	// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MODBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MODBUS_SLVS		1	// Anzahl MOD-Bus Geräte
#define MODBUS_SLV_0		0	// Index  MOD-Bus Gerät Ausgänge + Handschaltung

#endif
