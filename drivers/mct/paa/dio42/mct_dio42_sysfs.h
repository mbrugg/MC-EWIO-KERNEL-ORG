/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

*********************************************************************************/
#ifndef MCT_DIO42_SYSFS_H_
	#define MCT_DIO42_SYSFS_H_

extern void dio42_sysfs_ini(void);
extern void dio42_sysfs_drv_add(struct device_driver *drv);
extern void dio42_sysfs_drv_del(struct device_driver *drv);
extern void dio42_do_sysfs_dev_add(struct device *dev);
extern void dio42_do_sysfs_dev_del(struct device *dev);
extern void dio42_di_sysfs_dev_add(struct device *dev);
extern void dio42_di_sysfs_dev_del(struct device *dev);
#endif
