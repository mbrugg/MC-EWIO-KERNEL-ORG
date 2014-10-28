/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011
	
*********************************************************************************/
#include "../../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PAA_DO4_SLOT	5
#define DRV_MCT_PAA_DO4_NAME 	"mct_paa_do4_"CAT(DRV_MCT_PAA_DO4_SLOT)""
#define DEV_MCT_PAA_DO4_OUT_NAME "mct_do.00-03." BUS_LBUS "-" DRIVER_PAA_DO4_IDENT "-" CAT(DRV_MCT_PAA_DO4_SLOT)

char * 			do4_drv_name 		= DRV_MCT_PAA_DO4_NAME;
char * 			do4_dev_out_name 	= DEV_MCT_PAA_DO4_OUT_NAME;
unsigned char 	do4_slot			= DRV_MCT_PAA_DO4_SLOT;
