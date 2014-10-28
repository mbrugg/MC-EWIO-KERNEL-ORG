/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
	
	Description:
	
	07.08.2012 	CONFIG_HZ = 1024, da der Quarz am AT91RM92000 32.768 kHz
			Dadurch ergeben sich 1000 / 1024 = 0,9765625 ms pro Tick
			Werte die nicht Vielfache von  1000ms, 500ms, 250ms, 125ms 
			sind, haben eine Abweichung von 2,34375%. 
			Je 1ms "fehlen" immer 23,4375 ys. Wenn die Summe des Einzelab-
			weichungen > 50% einer Einzeltickzeit wird, dann ist ein 
			zusätzlicher Tick notwendig.

			Um ca. 4ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			4 Ticks *  0,9765625	=    3,90625  ms	-0,09375 ms 

			Um ca. 10 ms über Timerticks in Abhängigkeit von HZ zu realisieren:
			10 Ticks *  0,9765625	=    9,765625  ms	-0,234375 ms 
	
 *******************************************************************************/
#ifndef MCT_DIO_H_
	#define MCT_DIO_H_

#include "../mct_versions.h"

#define DRIVER_VERSION_STRING "V 01.00.10\n"

#define DRIVER_NAME 		"mct_spi_dio"
#define DEVICE_OUT_NAME 	"mct_dop.00-03." BUS_INTERN "-" DRIVER_SPI_DIO_IDENT "-0"
#define DEVICE_IN_NAME 		"mct_di.00-07." BUS_INTERN "-" DRIVER_SPI_DIO_IDENT "-0"

#define TIMER_SPI_GRANULARITY (HZ/100)	 	//adjust 9,765625 ms

// Module-Device-Debugging
#ifdef MCT_SPI_DIO_DEV_DEBUG
	#define trace_call_dev(dev) dev_printk(KERN_INFO , dev , "%s()\n", __FUNCTION__ )
#else
	#define trace_call_dev(dev)
#endif

#endif
