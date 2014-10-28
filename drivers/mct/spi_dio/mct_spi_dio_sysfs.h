/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011

 	Description:

*********************************************************************************/
#ifndef MCT_SPI_DIO_SYSFS_H_
	#define MCT_SPI_DIO_SYSFS_H_

extern void dio_sysfs_ini(void);
extern void dio_sysfs_drv_add(struct device_driver *drv);
extern void dio_sysfs_drv_del(struct device_driver *drv);
extern void dio_do_sysfs_dev_add(struct device *dev);
extern void dio_do_sysfs_dev_del(struct device *dev);
extern void dio_di_sysfs_dev_add(struct device *dev);
extern void dio_di_sysfs_dev_del(struct device *dev);

#endif
