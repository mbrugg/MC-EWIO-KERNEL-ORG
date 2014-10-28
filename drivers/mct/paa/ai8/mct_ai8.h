/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$	

	Description:	paa_ai8 Client-Driver

	 Analoger-Input DRIVER (Gerät)  mit 8 DEVICES (Input Kanälen, je 32 bit
	 float genutzt!)


*********************************************************************************/

//*********************************************************************************
// BT-AI8	8x Analoger Eingang
//
//  Datentelegramm asymmetrisch: 0Byte vom Master / 32Byte zum Master
//
//					zum Master
//					Inputs
//	Byte28-32		AI7 ist
//	Byte24-27		AI6 ist
//	Byte20-23		AI5 ist
//	Byte16-19		AI4 ist
//	Byte12-15		AI3 ist
//	Byte08-11		AI2 ist
//	Byte04-07		AI1 ist
//	Byte00-03		AI0 ist
//*********************************************************************************
#ifndef MCT_AI8_H_
	#define MCT_AI8_H_
#include "../../mct_versions.h"

extern char * 			ai8_drv_name;
extern char * 			ai8_dev_in_name;
extern unsigned char 	ai8_slot;

#define DRV_MCT_PAA_AI8_VERSION 			"V 01.00.10\n"

#define TIMER_AI8_GRANULARITY (HZ/5)	 	//adjust 200 ms

// Module-Device-Debugging
#ifdef MCT_PAA_AI8_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#endif
