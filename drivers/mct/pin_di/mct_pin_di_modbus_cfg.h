/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	06.10.2011

	Description: MODBUS-Configuration for mct_pin_di-Driver
	Tracen der MODBUS-Kommandos erfolgt mit:
	
	#define/#undef CONFIG_MODBUS_CMD_TRACE 

	MODBUS-Adressen:	(x=INSTANCE)
				TCP	RTU
				-----------
				0	20x

	Geräte-Object  mct_if.01.3-0-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-0]		= MODBUS, Treiberkennung mct_pin_di = 0
	[-SLOT]		= 0...3
	.0		= Instanz

*********************************************************************************/
#ifndef __MCT_MODBUS_PIN_DI_CFG_H_
	#define __MCT_MODBUS_PIN_DI_CFG_H_	

#define MODBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MODBUS "-" DRIVER_PIN_DI_IDENT "-"

//#define CONFIG_MODBUS_CMD_TRACE	// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MODBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MODBUS_SLVS		1	// Anzahl MOD-Bus Geräte
#define MODBUS_SLV_0		0	// Index  MOD-Bus Gerät 1 Eingang 

#endif
