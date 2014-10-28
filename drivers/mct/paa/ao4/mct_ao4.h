/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$	

	Description:	paa_ao4 Client-Driver

	Analoger-Output DRIVER (Gerät)  mit 4 DEVICES (Output Kanälen, je 16 bit
	davon 10bit genutzt!)


*********************************************************************************/

//*********************************************************************************
// BT-AO4	4x Analoger Ausgang
//
//  Datentelegramm symmetrisch: 8Byte vom Master / 8Byte zum Master
//
//					vom Master	zum Master		
//					Outputs		Outputs
//	Byte06-07		AO3 soll	AO3 (Kopie)
//	Byte04-05		AO2 soll	AO3 (Kopie)
//	Byte02-03		AO1 soll	AO3 (Kopie)
//	Byte00-01		AO0 soll	AO3 (Kopie)
//*********************************************************************************

#ifndef MCT_AO4_H_
	#define MCT_AO4_H_
#include "../../mct_versions.h"

extern char * 			ao4_drv_name;
extern char * 			ao4_dev_out_name;
extern unsigned char 	ao4_slot;

#define DRV_MCT_PAA_AO4_VERSION 		"V 01.00.10\n"

#define TIMER_AO4_GRANULARITY (HZ/5)	 	//adjust 200 ms

// Module-Device-Debugging
#ifdef MCT_PAA_AO4_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif


#endif
