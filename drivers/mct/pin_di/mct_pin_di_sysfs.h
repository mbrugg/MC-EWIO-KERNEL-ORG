/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011
 	
 	Description:

 *******************************************************************************/
#ifndef MCT_PIN_DI_SYSFS_H_
	#define MCT_PIN_DI_SYSFS_H_

#include <linux/device.h>

extern void di_sysfs_ini(void);
extern void di_sysfs_drv_add(struct device_driver *drv);
extern void di_sysfs_drv_del(struct device_driver *drv);
extern void di_sysfs_dev_add(struct device *dev);
extern void di_sysfs_dev_del(struct device *dev);

#endif
