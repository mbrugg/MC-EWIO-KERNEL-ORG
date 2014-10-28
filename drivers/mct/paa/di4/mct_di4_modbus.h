/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		06.10.2011
 	
 	Description: MODBUS-Interface for mct_paa_di4-Driver
	
	Dieses Interface erzeugt pro Instanz ein MODBUS-Device im Sys-Filesystem.
	Ein MODBUS-Device unterstützt 1 virtuelles MODBUS-Modul.	

	Geräte-Object  mct_if.01.3-3-SLOT.0
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-3]		= MODBUS, Treiberkennung mct_di4
	[-SLOT]		= 0...5
	.0			= Instanz

 *******************************************************************************/
#ifndef MCT_PAA_DI4_MODBUS_H_
	#define MCT_PAA_DI4_MODBUS_H_
#include <linux/platform_device.h>

extern int  modbus_build	(struct platform_driver * 	drv, unsigned char slot);
extern void modbus_destroy	(void);

#endif
