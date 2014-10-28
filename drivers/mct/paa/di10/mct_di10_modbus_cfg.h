/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	05.10.2011

	Description: MODBUS-Configuration for mct_paa_di10-Driver
		     Tracen der MODBUS-Kommandos erfolgt mit:
	
	#define/#undef CONFIG_MODBUS_CMD_TRACE

	MODBUS-Adressen: x=SLOT 0...5:  (unterstützt 1 virtuelles MODBUS-Modul)
			TCP	RTU
			-----------------
			0	1x4	

	Geräte-Object  mct_if.01.3-4-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-4]		= MODBUS, Treiberkennung mct_paa_di10
	[-SLOT]		= 0...5
	.0		= Instanz

*********************************************************************************/
#ifndef __MCT_MODBUS_PAA_DI10_CFG_H_
	#define __MCT_MODBUS_PAA_DI10_CFG_H_	

#define MODBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MODBUS "-" DRIVER_PAA_DI10_IDENT "-"

//#define CONFIG_MODBUS_CMD_TRACE	// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MODBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MODBUS_SLVS		1	// Anzahl MOD-Bus Geräte
#define MODBUS_SLV_0		0	// Index  MOD-Bus Gerät 10 Eingänge

#endif
