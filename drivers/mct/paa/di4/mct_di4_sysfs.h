/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

*********************************************************************************/
#ifndef MCT_DI4_SYSFS_H_
	#define MCT_DI4_SYSFS_H_

extern void di4_sysfs_ini(void);
extern void di4_sysfs_drv_add(struct device_driver *drv);
extern void di4_sysfs_drv_del(struct device_driver *drv);
extern void di4_sysfs_dev_add(struct device *dev);
extern void di4_sysfs_dev_del(struct device *dev);

#endif
