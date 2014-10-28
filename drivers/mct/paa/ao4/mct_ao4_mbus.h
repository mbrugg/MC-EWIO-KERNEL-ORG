/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.09.2011
 	
 	Description: MBUS-Interface for mct_paa_ao4-Driver
	
	Dieses Interface erzeugt pro Instanz ein MBUS-Device im Sys-Filesystem.
	Ein MBUS-Device unterstützt 4 virtuelle MBUS-Module.
	
 *******************************************************************************/
#ifndef MCT_PAA_AO4_MBUS_H_
	#define MCT_PAA_AO4_MBUS_H_
#include <linux/platform_device.h>

extern int  mbus_build		(struct platform_driver * 	drv, unsigned char slot);
extern void mbus_destroy	(void);

#endif
