/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	21.09.2011

	Description: MODBUS-Configuration for mct_paa_do4-Driver
			Tracen der MODBUS-Kommandos erfolgt mit:
	
      #define/#undef CONFIG_MODBUS_CMD_TRACE

	MODBUS-Adressen:	x=SLOT 0...5:  (unterstützt 1 virtuelles MODBUS-Modul)
				TCP	RTU
				-----------
				0	1x5

	Geräte-Object  mct_if.01.3-5-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-5]		= MODBUS, Treiberkennung mct_do4
	[-SLOT]		= 0...5
	.0		= Instanz

// Anordnung bei Read Input Coils
// Bit 0..3: Tatsächlicher Relais-Zustand, 0: Aus, 1: Ein
// Bit 4..7: Ursache des Relais-Zustands, 0: Bus, 1: Schalter

*********************************************************************************/
#ifndef __MCT_MODBUS_PAA_DO4_CFG_H_
	#define __MCT_MODBUS_PAA_DO4_CFG_H_	

#define MODBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MODBUS "-" DRIVER_PAA_DO4_IDENT "-"

//#define CONFIG_MODBUS_CMD_TRACE	// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MODBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MODBUS_SLVS		1	// Anzahl MOD-Bus Geräte
#define MODBUS_SLV_0		0	// Index  MOD-Bus Gerät Ausgänge + Handschaltung

#endif
