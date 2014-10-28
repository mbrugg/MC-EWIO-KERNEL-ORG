/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011

*********************************************************************************/
#include "../../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PAA_DI4_SLOT	3
#define DRV_MCT_PAA_DI4_NAME 	"mct_paa_di4_"CAT(DRV_MCT_PAA_DI4_SLOT)""
#define DEV_MCT_PAA_DI4_IN_NAME "mct_di.00-03." BUS_LBUS "-" DRIVER_PAA_DI4_IDENT "-" CAT(DRV_MCT_PAA_DI4_SLOT)

char * 			di4_dev_in_name = DEV_MCT_PAA_DI4_IN_NAME;
char * 			di4_drv_name 	= DRV_MCT_PAA_DI4_NAME;
unsigned char 	di4_slot		= DRV_MCT_PAA_DI4_SLOT;

