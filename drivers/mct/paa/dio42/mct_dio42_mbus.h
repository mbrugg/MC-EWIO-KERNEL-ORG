/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		31.08.2011
 	
 	Description:   MBUS Interface for paa_dio42-Driver
	
	Dieses Interface erzeugt pro Instanz ein MBUS-Device im Sys-Filesystem.
	Ein MBUS-Devive unterstützt 2 virtuelle MBUS-Module. 

 *******************************************************************************/
#ifndef MCT_PAA_DIO42_MBUS_H_
	#define MCT_PAA_DIO42_MBUS_H_
#include <linux/platform_device.h>

extern int  mbus_build		(struct platform_driver * 	drv, unsigned char slot);
extern void mbus_destroy	(void);

#endif
