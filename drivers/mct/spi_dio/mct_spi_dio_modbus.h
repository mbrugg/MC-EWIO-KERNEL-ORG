/*********************************************************************************
	Copyright MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		06.01.2012

 	Description: MODBUS-Interface for mct_spi_dio-Driver
	
	Dieses Interface erzeugt pro Instanz ein MODBUS-Device im Sys-Filesystem.
	Das MODBUS-Device unterstützt 1 virtuelles MODBUS-Modul

 *******************************************************************************/
#ifndef MCT_SPI_DIO_MODBUS_H_
	#define MCT_SPI_DIO_MODBUS_H_
#include <linux/platform_device.h>

extern int  modbus_build	(struct platform_driver * 	drv, unsigned char slot);
extern void modbus_destroy	(void);

#endif
