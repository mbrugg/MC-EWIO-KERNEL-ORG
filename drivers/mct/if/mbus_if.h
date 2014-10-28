/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS Interface for Driver
	
	Dieses Interface erzeugt ein zusätzliches MBUS-Device im Sys-Filesystem. 
	Dadurch kann ein "Treiber" alternativ über das MBUS-Protokoll gesteuert 
	werden. 

	31.10.11	Das Interface arbeitet nur, wenn sich der Treiber im
				Running Mode befindet!
				 
	21.02.12	eth0 - lesen der MAC-Adresse
				Artikel: http://www.linuxjournal.com/article/8110?page=0,2
				Info: http://os1a.cs.columbia.edu/lxr/source/sound/sound_firmware.c

 *******************************************************************************/
#ifndef MCT_MBUS_H_
	#define MCT_MBUS_H_	
	#include <linux/fs.h>				//sys und udev filesystem support
	#include <linux/platform_device.h>

	extern  int  mbus_if_create_bin_file(struct kobject * kobj);
	extern  int  mbus_if_remove_bin_file(struct kobject * kobj);
	extern	long mbus_if_secaddr_by_macaddr(char * secaddr);
#endif