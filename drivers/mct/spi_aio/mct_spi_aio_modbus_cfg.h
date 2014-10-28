/*********************************************************************************
	Copyright	MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		15.02.2012

	Description: MODBUS-Configuration for mct_spi_aio-Driver
	
	Tracen der MODBUS-Kommandos erfolgt mit:
	
	#define/#undef CONFIG_MODBUS_CMD_TRACE 

	MODBUS-Adressen:TCP	RTU
			-----------------
			0	220

	Geräte-Object  mct_if.01.3-2-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-2]		= MODBUS, Treiberkennung mct_spi_aio
	SLOT		= z.Zeit immer 0 
	.0		= Instanz

	Input : 4xKanal 32bit float (0-1 = Spannung/Widerstand, 2-3 = Strom) 
	Output: 4xKanal 16bit (0-1 = Spannung, 2-3 = Strom)
	
	Register:

*********************************************************************************/
#ifndef __MCT_MODBUS_SPI_AIO_CFG_H_
	#define __MCT_MODBUS_SPI_AIO_CFG_H_	

#define MODBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".01." BUS_MODBUS "-" DRIVER_SPI_AIO_IDENT "-"

//#define CONFIG_MODBUS_CMD_TRACE	// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MODBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MODBUS_SLVS		1	// Anzahl MOD-Bus Geräte
#define MODBUS_SLV_0		0	// Index  MOD-Bus Gerät 4 analoge Eingänge
					// 4 analoge Ausgänge	
#endif
