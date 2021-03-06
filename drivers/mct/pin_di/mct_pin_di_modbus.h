/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		16.01.2012
 	
 	Description: MODBUS-Interface for pin_di-Driver
	
	Dieses Interface erzeugt pro Instanz ein MODBUS-Device im Sys-Filesystem.
	Ein	MODBUS-Device unterstützt 1 virtuelles MODBUS-Modul. 

 *******************************************************************************/
#ifndef MCT_PIN_DI_MODBUS_H_
	#define MCT_PIN_DI_MODBUS_H_
#include <linux/platform_device.h>

extern int  modbus_build	(struct platform_driver * 	drv, unsigned char slot);
extern void modbus_destroy	(void);

#endif
