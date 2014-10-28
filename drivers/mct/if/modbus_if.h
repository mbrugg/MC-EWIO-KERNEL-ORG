/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		21.09.2011
 	
 	Description:  MODBUS-LAYER: "INTERFACE" (only SLAVE) 

	Dieses Interface erzeugt ein zusätzliches MODBUS-Device im Sys-Filesystem. 
	Dadurch kann ein "Treiber" alternativ über das MODBUS-Protokoll gesteuert 
	werden. 

 *******************************************************************************/
#ifndef MCT_MODBUS_H_
	#define MCT_MODBUS_H_	
	#include <linux/fs.h>				//sys und udev filesystem support
	#include <linux/platform_device.h>
	extern  int  modbus_if_create_bin_file(struct kobject * kobj);
	extern  int  modbus_if_remove_bin_file(struct kobject * kobj);
#endif