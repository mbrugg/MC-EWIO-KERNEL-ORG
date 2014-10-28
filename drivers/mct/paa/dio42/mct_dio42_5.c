/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		26.08.2011	

*********************************************************************************/
#include "../../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PAA_DIO42_SLOT		5
#define DRV_MCT_PAA_DIO42_NAME 		"mct_paa_dio42_"CAT(DRV_MCT_PAA_DIO42_SLOT)""
#define DEV_MCT_PAA_DIO42_IN_NAME 	"mct_di.00-03." BUS_LBUS "-" DRIVER_PAA_DIO42_IDENT "-" CAT(DRV_MCT_PAA_DIO42_SLOT)
#define DEV_MCT_PAA_DIO42_OUT_NAME 	"mct_do.00-01." BUS_LBUS "-" DRIVER_PAA_DIO42_IDENT "_" CAT(DRV_MCT_PAA_DIO42_SLOT)

char * 			dio42_drv_name		= DRV_MCT_PAA_DIO42_NAME;
char * 			dio42_dev_in_name 	= DEV_MCT_PAA_DIO42_IN_NAME;
char * 			dio42_dev_out_name 	= DEV_MCT_PAA_DIO42_OUT_NAME;
unsigned char 	dio42_slot			= DRV_MCT_PAA_DIO42_SLOT;
