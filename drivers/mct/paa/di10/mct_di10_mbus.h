/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011
 	
 	Description: MBUS-Interface for mct_paa_di10-Driver
	
	Dieses Interface erzeugt pro Instanz ein MBUS-Device im Sys-Filesystem.
	Ein MBUS-Device unterstützt 1 virtuelles MBUS-Modul.

 *******************************************************************************/
#ifndef MCT_PAA_DI10_MBUS_H_
	#define MCT_PAA_DI10_MBUS_H_
#include <linux/platform_device.h>

extern int  mbus_build		(struct platform_driver * 	drv, unsigned char slot);
extern void mbus_destroy	(void);

#endif
