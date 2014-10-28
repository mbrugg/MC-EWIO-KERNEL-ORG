/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#ifndef MCT_SPI_AIO_SYSFS_H_
	#define MCT_SPI_AIO_SYSFS_H_

extern void aio_sysfs_ini(void);
extern void aio_sysfs_attr_add(struct device_driver *drv);
extern void aio_sysfs_attr_del(struct device_driver *drv);
extern void aio_ao_sysfs_attr_add(struct device *dev);
extern void aio_ao_sysfs_attr_del(struct device *dev);
extern void aio_ai_sysfs_attr_add(struct device *dev);
extern void aio_ai_sysfs_attr_del(struct device *dev);

#endif
