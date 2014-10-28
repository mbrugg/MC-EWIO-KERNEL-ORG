/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011

	Description: MBUS-Configuration for mct_spi_dio-Driver
				 Tracen der MBUS-Kommandos erfolgt mit:
	
				 #define/#undef CONFIG_MBUS_CMD_TRACE 

	Sec-Adressen:	MBUS-Instanz 0: (unterstützt virtuelle 2 MBUS-Module)
					00000100 == MBUS-Modul (dig. Eingänge)
					00000101 == MBUS-Modul (dig. Ausgänge)	

	Geräte-Object  mct_if.02.2-1-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.02] 		= 2 Geräte
	[.2-1]		= MBUS, Treiberkennung mct_spi_dio
	SLOT		= z.Zeit immer 0 
	0			= Instanznummer

*********************************************************************************/
#ifndef __MCT_SPI_DIO_MBUS_CFG_H_
	#define __MCT_SPI_DIO_MBUS_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".02." BUS_MBUS "-" DRIVER_SPI_DIO_IDENT "-0"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MBUS_SLVS			2	// Anzahl M-Bus Geräte
#define MBUS_SLV_IN			0	// Index  M-Bus Gerät Eingänge
#define MBUS_SLV_OUT		1	// Index  M-Bus Gerät Ausgänge + Handschaltung
#define MBUS_SLV_OUT_BITS	0x1fff	// 13 Bit

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e

#endif
