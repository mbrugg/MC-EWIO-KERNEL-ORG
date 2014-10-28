/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		22.01.2012

 	Description: MBUS-Configuration for mct_spi_aio-Driver
				 Tracen der MBUS-Kommandos erfolgt mit:
	
				 #define/#undef CONFIG_MBUS_CMD_TRACE 

	Sec-Adressen:	MBUS-Instanz 0:		(unterstützt 8 virtuelle MBUS-Module)
					00000200 == MBUS-Modul (analoger Eingang)
					00000201 == MBUS-Modul (analoger Eingang)
					00000202 == MBUS-Modul (analoger Eingang)
					00000203 == MBUS-Modul (analoger Eingang)
					00000204 == MBUS-Modul (analoger Ausgang)
					00000205 == MBUS-Modul (analoger Ausgang)
					00000206 == MBUS-Modul (analoger Ausgang)
					00000207 == MBUS-Modul (analoger Ausgang)

	Geräte-Object  mct_if.08.2-2-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.08] 		= 8 Geräte (4 analoge In, 4 analoge Out)
	[.2-2]		= MBUS, Treiberkennung mct_spi_aio
	SLOT		= z.Zeit immer 0 
	0			= Instanznummer

*********************************************************************************/
#ifndef __MCT_MBUS_SPI_AIO_CFG_H_
	#define __MCT_MBUS_SPI_AIO_CFG_H_	

#define MBUS_DEVICE_INTERFACE	"mct_" DRIVER_INTERFACE ".08." BUS_MBUS "-" DRIVER_SPI_AIO_IDENT "-0"

//#define CONFIG_MBUS_CMD_TRACE		// AN:  Schalter - tracen der Kommandos
#undef CONFIG_MBUS_CMD_TRACE		// AUS: Schalter - tracen der Kommandos

#define MBUS_SLVS			8			// Anzahl M-Bus Geräte
#define MBUS_SLV_IN0		0			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN1		1			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN2		2			// Index  M-Bus Gerät Eingang
#define MBUS_SLV_IN3		3			// Index  M-Bus Gerät Eingang

#define MBUS_SLV_OUT0		4			// Index  M-Bus Gerät Ausgang
#define MBUS_SLV_OUT1		5			// Index  M-Bus Gerät Ausgang
#define MBUS_SLV_OUT2		6			// Index  M-Bus Gerät Ausgang
#define MBUS_SLV_OUT3		7			// Index  M-Bus Gerät Ausgang

#define CHANNEL_IN_OFFSET 	MBUS_SLV_IN0
#define CHANNEL_OUT_OFFSET 	MBUS_SLV_OUT0

#define MBUS_MANU_HEX		0x33,0x4f
#define MBUS_GEN_HEX		0x0a
#define MBUS_MED_HEX		0x0e

#endif
