/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

*********************************************************************************/
#include "../../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PAA_DI10_SLOT	5
#define DRV_MCT_PAA_DI10_NAME 	"mct_paa_di10_"CAT(DRV_MCT_PAA_DI10_SLOT)""
#define DEV_MCT_PAA_DI10_IN_NAME "mct_di.00-09." BUS_LBUS "-" DRIVER_PAA_DI10_IDENT "-" CAT(DRV_MCT_PAA_DI10_SLOT)

char * 			di10_dev_in_name = DEV_MCT_PAA_DI10_IN_NAME;
char * 			di10_drv_name 	= DRV_MCT_PAA_DI10_NAME;
unsigned char 	di10_slot		= DRV_MCT_PAA_DI10_SLOT;
