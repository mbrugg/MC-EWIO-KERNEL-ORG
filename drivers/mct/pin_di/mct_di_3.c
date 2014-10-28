/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011

 *******************************************************************************/
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/irq.h>	
#include <mach/board.h>
#include <mach/gpio.h>
#include "mct_pin_di_objs.h"
#include "../mct_versions.h"

#define _CAT(x) #x
#define CAT(x) _CAT(x)

#define DRV_MCT_PIN_DI_SLOT		3
#define DRV_MCT_PIN_DI_NAME 	"mct_pin_di_"CAT(DRV_MCT_PIN_DI_SLOT)""
#define DEV_MCT_PIN_DI_IN_NAME 	"mct_di.01." BUS_INTERN "-" DRIVER_PIN_DI_IDENT "-" CAT(DRV_MCT_PIN_DI_SLOT)
#define DEV_PIN_NAME			"Pulse 4 (S04/4-)"

char *			di_drv_name		= DRV_MCT_PIN_DI_NAME;
char *			di_dev_in_name 	= DEV_MCT_PIN_DI_IN_NAME;
unsigned char 	di_slot			= DRV_MCT_PIN_DI_SLOT;

unsigned char	di_pin			= AT91_PIN_PB9;
char *			di_pin_name		= DEV_PIN_NAME;

///*********
/// INTERCOM
///*********
/// Server setzt im Client 3 das Init-Bit
void icom_svr_set_cli3_init(unsigned char init) {
	unsigned long long u64 = 0;
	unsigned char mode = 0;
	OBJS_IDEV_RD_MODE(&di_O_Inputs,mode);
	if (mode == MODE_S0) {
//		 printk("%s() init:%d\n",__FUNCTION__,init);
		if(init) {	// init=1, dann aktuellen Inputzähler auf Freezezähler
			OBJS_IDEV_RD_VALUE(&di_O_Inputs,u64);
			OBJS_IDEV_WR_FREEZE(&di_O_Inputs,u64);
		}
		OBJS_IDEV_WR_INIT(&di_O_Inputs,init); // Init-Mode aktualisieren!}
	}
}

/// Server holt vom Client 3 das Init-Bit
// -1 wenn nicht im S0-Mode, sonst 0 oder 1
int icom_svr_get_cli3_init(void) {
	int init = -1;
	unsigned char mode = 0;
	OBJS_IDEV_RD_MODE(&di_O_Inputs,mode);
	if (mode == MODE_S0) {
		OBJS_IDEV_RD_INIT(&di_O_Inputs,init); // Init-Mode aktualisieren!}
//		printk("%s() init:%d\n",__FUNCTION__,init);
	}
	return init;
}

EXPORT_SYMBOL_GPL( icom_svr_set_cli3_init );
EXPORT_SYMBOL_GPL( icom_svr_get_cli3_init );
