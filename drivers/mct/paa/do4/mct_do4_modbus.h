/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		21.09.2011
 	
 	Description: MODBUS-Interface for mct_paa_do4-Driver
	
	Dieses Interface erzeugt pro Instanz ein MODBUS-Device im Sys-Filesystem.
	Das MODBUS-Device unterstützt 1 virtuelles MODBUS-Modul.

	Geräte-Object  mct_if.01.3-5-SLOT.x
	[mct] 		= MC Technology
	[if]  		= Interface
	[.01] 		= 1 Gerät
	[.3-5]		= MODBUS, Treiberkennung mct_do4
	[-SLOT]		= 0...5
	.0			= Instanz

 *******************************************************************************/
#ifndef MCT_PAA_DO4_MODBUS_H_
	#define MCT_PAA_DO4_MODBUS_H_
#include <linux/platform_device.h>
extern int  modbus_build	(struct platform_driver * 	drv, unsigned char slot);
extern void modbus_destroy	(void);
#endif
