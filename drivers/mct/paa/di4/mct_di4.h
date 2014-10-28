/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

	Description:  mct_paa_di4 CLIENT-DRIVER (L-Bus Hardware)

	Digitaler-Input CLIENT-DRIVER mit 
	---------------------------------------------------------
	1 Input-Device	 (4 bit)
	---------------------------------------------------------

*********************************************************************************/
#ifndef MCT_DI4_H_
	#define MCT_DI4_H_

#include "../../mct_versions.h"

extern char * 			di4_dev_in_name;
extern char * 			di4_drv_name;
extern unsigned char 	di4_slot;


// Module-Device-Debugging
#ifdef MCT_PAA_DI4_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#define DRV_MCT_PAA_DI4_VERSION 		"V 01.00.10\n"

#define TIMER_DI4_GRANULARITY (HZ/40)	 //adjust 50 ms

#endif
