/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		12.10.2011
 	
 	Description: MODBUS-Interface for mct_paa_ai8-Driver
	
	Dieses Interface erzeugt pro Instanz ein MODBUS-Device im Sys-Filesystem.
	Ein MODBUS-Device unterstützt 1 virtuelles MODBUS-Modul.	

 *******************************************************************************/
#ifndef MCT_PAA_AI8_MODBUS_H_
	#define MCT_PAA_AI8_MODBUS_H_
#include <linux/platform_device.h>

extern int  modbus_build	(struct platform_driver * 	drv, unsigned char slot);
extern void modbus_destroy	(void);

#endif
