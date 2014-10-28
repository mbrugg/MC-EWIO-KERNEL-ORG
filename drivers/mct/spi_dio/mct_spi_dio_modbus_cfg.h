/*********************************************************************************
	Copyright MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		06.01.2012

	Description: MODBUS-Configuration for mct_spi_dio-Driver

	Tracen der MODBUS-Kommandos erfolgt mit:
	
	#define/#undef CONFIG_MODBUS_CMD_TRACE 

	MODBUS-Adressen:TCP	RTU
			------------
			0	210

	Geräte-Object  mct_if.01.3-1-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-1]		= MODBUS, Treiberkennung mct_spi_dio
	[-SLOT]		= 0
	.0		= Instanz

	Input : 8bit Optokoppler
	Output:	4bit Relais
	
	Register:

*********************************************************************************/
#ifndef __MCT_SPI_DIO_MODBUS_CFG_H_
	#define __MCT_SPI_DIO_MODBUS_CFG_H_	

#define MODBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MODBUS "-" DRIVER_SPI_DIO_IDENT "-"

//#define CONFIG_MODBUS_CMD_TRACE	// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MODBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MODBUS_SLVS		1	// Anzahl MOD-Bus Geräte
#define MODBUS_SLV_0		0	// Index  MOD-Bus Gerät Ausgänge + Handschaltung

#endif
