/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

	Description:	paa_di10 Client-Driver

*********************************************************************************/
#ifndef MCT_DI10_H_
	#define MCT_DI10_H_

#include "../../mct_versions.h"

extern char * 			di10_dev_in_name;
extern char * 			di10_drv_name;
extern unsigned char 	di10_slot;

// Module-Device-Debugging
#ifdef MCT_PAA_DI10_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#define DRV_MCT_PAA_DI10_VERSION 		"V 01.00.10\n"

#define TIMER_DI10_GRANULARITY (HZ/40)	 //adjust 50 ms

#endif
