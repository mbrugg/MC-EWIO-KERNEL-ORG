/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		21.09.2011
 	
 	Description:  MBUS SLAVE APPLICATION - DEVICE
				

 *******************************************************************************/
#ifndef __MODBUS_APP_H__
	#define __MODBUS_APP_H__

#include "modbus_sl2.h"				// slave layer2 

struct mod_app {
	struct mod_l2 l2;
//	char	(*fake_line)[1000];
};

extern void 			mod_app_task_rec	(struct mod_app * la);
extern int 				mod_app_response	(struct mod_app * la);
#endif // __MODBUS_APP_H__
