/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		26.08.2011	

	Description:	paa_di42 Client-Driver

*********************************************************************************/
#ifndef MCT_DIO42_H_
	#define MCT_DIO42_H_

#include "../../mct_versions.h"

extern char * 			dio42_drv_name;
extern char * 			dio42_dev_in_name;
extern char * 			dio42_dev_out_name;
extern unsigned char 	dio42_slot;

#define DRV_MCT_PAA_DIO42_VERSION 		"V 01.00.10\n"

#define TIMER_DIO42_GRANULARITY (HZ/40)	 	//adjust 50 ms

// Module-Device-Debugging
#ifdef MCT_PAA_DIO42_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#endif
