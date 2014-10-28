/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

	Description:	paa_do4 Client-Driver

	Digital-Output DRIVER (Gerät)  mit 4 DEVICES (Output Kanälen, ja 1 bit )
 	(Relaisausgänge)

*********************************************************************************/

//*********************************************************************************
// BT-DO4	4x Digitalausgang (Relaisausgang)
//
//  Datentelegramm asymmetrisch: 1 Byte vom Master / 2 Byte zum Master
//
//	Byte0	vom Master	zum Master	
//			Outputs		Outputs								
//	Bit_7	x			x
//	Bit_6	x			x
//	Bit_5	x			x
//	Bit_4	x			x
//	Bit_3	R3-soll		R3-ist
//	Bit_2	R2-soll		R2-ist
//	Bit_1	R1-soll		R1-ist	
//	Bit_0	R0-soll		R0-ist
//	
//	Byte1				zum Master		
//						Handmaske
//	Bit_7				x
//	Bit_6				x
//	Bit_5				x
//	Bit_4				x
//	Bit_3				Hand3-ist
//	Bit_2				Hand2-ist
//	Bit_1				Hand1-ist
//	Bit_0				Hand0-ist	
//
//*********************************************************************************
#ifndef MCT_DO4_H_
	#define MCT_DO4_H_

#include "../../mct_versions.h"

extern char * 			do4_drv_name;
extern char * 			do4_dev_out_name;
extern unsigned char 	do4_slot;

#define DRV_MCT_PAA_DO4_VERSION 		"V 01.00.10\n"

#define TIMER_DO4_GRANULARITY (HZ/50)	 //adjust 20 ms

// Module-Device-Debugging
#ifdef MCT_PAA_DO4_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#endif
