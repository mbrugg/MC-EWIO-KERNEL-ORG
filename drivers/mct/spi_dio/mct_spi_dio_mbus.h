/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011

 	Description:  MBUS-Interface for mct_spi_dio-Driver
	
	Dieses Interface erzeugt ein MBUS-Device im Sys-Filesystem.
	Das MBUS-Device unterstützt 2 virtuelle MBUS-Module. 

 *******************************************************************************/
#ifndef MCT_SPI_DIO_MBUS_H_
	#define MCT_SPI_DIO_MBUS_H_
#include <linux/platform_device.h>

extern int  mbus_build		(struct platform_driver * 	drv, unsigned char instance);
extern void mbus_destroy	(void);

#endif
