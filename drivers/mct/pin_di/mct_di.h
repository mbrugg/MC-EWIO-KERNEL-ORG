/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011
 	
 	Description:  Direct/S0-Driver

	 28.10.2010 	CONFIG_HZ = 1024, da der Quarz am AT91RM92000 32.768 kHz
			Dadurch ergeben sich 1000 / 1024 = 0,9765625 ms pro Tick
			Werte die nicht Vielfache von  1000ms, 500ms, 250ms, 125ms 
			sind, haben eine Abweichung von 2,34375%. 
			Je 1ms "fehlen" immer 23,4375 ys. Wenn die Summe des Einzelab-
			weichungen > 50% einer Einzeltickzeit wird, dann ist ein 
			zusätzlicher Tick notwendig.

			Um ca. 4ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			4 Ticks *  0,9765625	=    3,90625  ms	-0,09375 ms 

			Um ca. 10ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			10 Ticks *  0,9765625	=    9,765625  ms	-0,234375 ms 

	
	31.05.2011	M-BUS-Interface als zusätzliches DEVICE implementiert
					
	23.06.2011	BugFix:
			Formatanweisung von %lld auf %llu für 64bit-Werte
			im Counter gab es dadurch negtive Werte! 

*********************************************************************************/
#ifndef MCT_DI_H_
	#define MCT_DI_H_

#include "../mct_versions.h"

extern char * 		di_dev_in_name;
extern char * 		di_drv_name;
extern unsigned char 	di_slot;

extern unsigned char	di_pin;
extern char	*	di_pin_name;

// Module-Device-Debugging
#ifdef MCT_PIN_DI_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#define DRIVER_VERSION_STRING "V 01.00.10\n"


// Direkter Eingang
#define DIRECT_GRANULARITY 	(HZ/102)	// adjust 10ms

// Impulserkennung, wenn laut DIN 43 864 mindestens 
// 25ms high-Pegel und 25ms low-Pegel bei 90% Amplitude und
// Flankenanstieg tA< 5ms (Amplitude 0.0 auf 0.9)
// Flankenabfall tF< 5ms (Amplitude 0.9 auf 0.0)
#define S0_GRANULARITY 		(HZ/256)	// adjust 4ms
#define S0_H_SAMPLE		6		// 24ms
#define S0_L_SAMPLE		6		// 24ms

// HARDWARE
// Pull-Up / Pull-Down 
#define TRI_PULL_DOWN		0
#define TRI_PULL_UP		1


#endif
