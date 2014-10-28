/*********************************************************************************

 	Copyright MC-Technology GmbH 2010,2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

	 Analoger-Output DRIVER (Gerät)  mit 4 DEVICES (Output Kanälen, je 16 bit
	 davon 10bit genutzt!)

*********************************************************************************/
#include "../../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PAA_AO4_SLOT	2
#define DRV_MCT_PAA_AO4_NAME 	"mct_paa_ao4_"CAT(DRV_MCT_PAA_AO4_SLOT)""
#define DEV_MCT_PAA_AO4_OUT_NAME "mct_ao.00-09." BUS_LBUS "-" DRIVER_PAA_AO4_IDENT "-" CAT(DRV_MCT_PAA_AO4_SLOT)

char * 			ao4_drv_name 	= DRV_MCT_PAA_AO4_NAME;
char * 			ao4_dev_out_name = DEV_MCT_PAA_AO4_OUT_NAME;
unsigned char 	ao4_slot		= DRV_MCT_PAA_AO4_SLOT;


