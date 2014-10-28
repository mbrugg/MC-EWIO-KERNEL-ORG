/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		13.10.2011
 	
 	Description: MODBUS-Interface for mct_paa_dio42-Driver
	
	Dieses Interface erzeugt pro Instanz ein MODBUS-Device im Sys-Filesystem.
	Ein MODBUS-Device unterstützt 1 virtuelles MODBUS-Modul.

	Geräte-Object  mct_if.01.3-6-SLOT.x
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-6]		= MODBUS, Treiberkennung mct_dio42
	[-SLOT]		= 0...5
	.0			= Instanz

 *******************************************************************************/
#ifndef MCT_PAA_DIO42_MODBUS_H_
	#define MCT_PAA_DIO42_MODBUS_H_
#include <linux/platform_device.h>

extern int  modbus_build	(struct platform_driver * 	drv, unsigned char slot);
extern void modbus_destroy	(void);

#endif
