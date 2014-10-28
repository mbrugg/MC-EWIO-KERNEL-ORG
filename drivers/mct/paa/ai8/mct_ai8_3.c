/*********************************************************************************

 	Copyright MC-Technology GmbH 2010,2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

	 Analoger-Input DRIVER (Gerät)  mit 8 DEVICES (Input Kanälen, je 32 bit
	 float genutzt!)

*********************************************************************************/
#include "../../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PAA_AI8_SLOT	3
#define DRV_MCT_PAA_AI8_NAME 	"mct_paa_ai8_"CAT(DRV_MCT_PAA_AI8_SLOT)""
#define DEV_MCT_PAA_AI8_IN_NAME "mct_ai.00-31." BUS_LBUS "-" DRIVER_PAA_AI8_IDENT "-" CAT(DRV_MCT_PAA_AI8_SLOT)

char * 			ai8_dev_in_name = DEV_MCT_PAA_AI8_IN_NAME;
char * 			ai8_drv_name 	= DRV_MCT_PAA_AI8_NAME;
unsigned char 	ai8_slot		= DRV_MCT_PAA_AI8_SLOT;
