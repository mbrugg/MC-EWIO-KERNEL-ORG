/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
 	
 	Description:

 *******************************************************************************/
#ifndef MCT_SPI_DIO_OBJS_H_
	#define MCT_SPI_DIO_OBJS_H_

#include <linux/platform_device.h>
#include <linux/spi/spi.h>

#include "../ics/mct_spi_74hc594x1.h"		// Hardware support
#include "../mct_types.h"
#include "../mct_json.h"

#define DEVICE_IN_MAX			1	// vom DIO maximal unterstützte IN-Geräte
#define DEVICE_OUT_MAX			1	// vom DIO maximal unterstützte OUT-Geräte

#define DEVICE_IN_0				0	// IN-Gerätenummer
#define DEVICE_OUT_0			0	// OUT-Gerätenummer

#define OUTPUT_ELEMENTS			4	// number of output-elements per device
#define INPUT_ELEMENTS			8	// number of input-elements per device

#define DEVICE_OUT_bits			4	// 4 bit Verarbeitungsbreite
#define DEVICE_IN_bits			8	// 8 bit Verarbeitungsbreite

// Variable: output:
#define OUT8_RELS_SHIFTER			0
#define OUT8_RELS_SHIFTER_PROTECT	4
#define OUT8_RELS_MASK				0x0F 
#define OUT8_RELS_MASK_PROTECT		0xF0 
#define OUT8_REL_0					0	
#define OUT8_REL_1					1
#define OUT8_REL_2					2
#define OUT8_REL_3					3

// Variable: init
#define OUT8_LEDS_SHIFTER			4
#define OUT8_LEDS_MASK				0xF0

struct t_dkey{										// Key-Inputs
	u8  			state;							// interne Statusmaschine
	u8 				tick_h;							// 1-Zähler		
};

///********************
/// CONTROL
///*********************
typedef struct {
	struct spi_device 	*spi;	
	struct timer_list 	timer;
	atomic_t			timer_stop_job;
	struct completion	timer_stop_quit;
	struct spi_message 	msg;
	struct spi_transfer xfer;
	u8 					o_buf[O_REGS];
	u8 					i_buf[I_REGS];
	struct t_dkey		dkeys[I_DKEY_CHANNELS];	//unprotected, do'nt use outside completion!
} dio_control;

typedef struct  {				// obj_value
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_Name;					
	prop_byte 		P_Size;
	prop_byte 		P_Value;
} obj_value;

///****************
/// Device Outputs
///****************
typedef struct  {				// obj_init
	char   *		pname;
	spinlock_t		lock;
	prop_byte 		P_Value;	// 0000 xxxx - channel
	prop_byte 		P_Size;
	prop_byte 		P_Link;		// 0000 xxxx - Verbindung zu S0-Treiber
} obj_init;

struct tdio_dev_outp {			// Output-Object
	char *				pname;
	obj_value  * 		Op_Value;	// digitale Relais-Ausgänge
	obj_init *			Op_Init;	// Initialisierung S0
};

///	Makros: Device Outputs
// Outp.OValue.PValue.value - schreiben
#define OBJS_ODEV_WR_OUTPUT_4Bit(odev,src)	{	\
	spin_lock_bh(&(odev)->Op_Value->lock);	\
	(odev)->Op_Value->P_Value.value = (src & SIZE_MAX_4bit);	\
	spin_unlock_bh(&(odev)->Op_Value->lock);}

// Outp.OValue.PValue.value - lesen
#define OBJS_ODEV_RD_OUTPUT(odev,dst)	{	\
	spin_lock_bh(&(odev)->Op_Value->lock);	\
	dst = (odev)->Op_Value->P_Value.value;	\
	spin_unlock_bh(&(odev)->Op_Value->lock);}

// Outp.OInit.PValue.value - schreiben
#define OBJS_ODEV_WR_INIT_4Bit(odev,src)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	(odev)->Op_Init->P_Value.value=(src & SIZE_MAX_4bit);	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}

// Outp.OInit.PValue.value - lesen
#define OBJS_ODEV_RD_INIT(odev, dst)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	dst = (odev)->Op_Init->P_Value.value;	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}

// Outp.OInit.P_Link.value(chan) auf 1 = setzen
#define OBJS_ODEV_SET_LINK(odev,chan)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	(odev)->Op_Init->P_Link.value |= (1<<(chan));	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}
 
// Outp.OInit.P_Link.value(chan) auf 0 = löschen
#define OBJS_ODEV_CLR_LINK(odev,chan)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	(odev)->Op_Init->P_Link.value &=~(1<<(chan));	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}

// Outp.OInit.P_Link.value(chan) -lesen 
#define OBJS_ODEV_RD_LINK(odev,chan,val)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	val = (odev)->Op_Init->P_Link.value;	\
	val = (val>>chan) & 0x01;	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}

// Outp.OInit.P_Link.value(komplett) - lesen 
#define OBJS_ODEV_RD_LINK_S(odev,val)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	val = (odev)->Op_Init->P_Link.value;	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}

// Outp.OInit.P_Link.value(komplett) -  schreiben
#define OBJS_ODEV_WR_LINK_S(odev,val)	{	\
	spin_lock_bh(&(odev)->Op_Init->lock);	\
	(odev)->Op_Init->P_Link.value = val & SIZE_MAX_4bit;	\
	spin_unlock_bh(&(odev)->Op_Init->lock);}

///***************
/// Device Inputs
///***************
struct tdio_dev_inp {			// Input-Object
	char *				pname;
	obj_value  * 		Op_Value;
};

///	Makros: Device Inputs
// Inp.OValue.value - lesen
#define OBJS_IDEV_RD_INPUT(idev,dst)	{	\
			spin_lock_bh(&(idev)->Op_Value->lock);	\
			dst = (idev)->Op_Value->P_Value.value;	\
			spin_unlock_bh(&(idev)->Op_Value->lock);}

// Inp.OValue.value - schreiben
#define OBJS_IDEV_WR_INPUT(idev,val)	{	\
	spin_lock_bh(&(idev)->Op_Value->lock);	\
	(idev)->Op_Value->P_Value.value = val;	\
	spin_unlock_bh(&(idev)->Op_Value->lock);}

///****************
/// Driver
///****************
typedef struct  {				// obj_physic
	char   *		pname;
	prop_str 		P_Layer0;	// SPI
} obj_physic;


// Zentrale Driverstruktur (only Pointer-SLOT's)
// ACHTUNG: Das Element *obj_control muss immer an der 1. Stelle stehen!
struct tdio_drv {
	obj_control 			* 	Op_Control;		// Slot:  Control-Object
	dio_control 			* 	ct;				// Slot:  globale Steuerung	
	obj_physic 				* 	Op_Physic;		// Slot:  Physic
	struct tdio_dev_inp 	* 	Op_Inputs;		// Slot:  Input-Object
	struct tdio_dev_outp 	*	Op_Outputs;		// Slot:  Output-Object
};

///	Makros: Driver
#define OBJS_DRV_RD_COMMAND(drv,cmd) 	{	\
	spin_lock_bh(&(drv)->Op_Control->lock);	\
	cmd = (drv)->Op_Control->P_Control.value[0];	\
	spin_unlock_bh(&(drv)->Op_Control->lock);}

// OControl.Command - schreiben
#define OBJS_DRV_WR_COMMAND(drv,cmd) 	{	\
	spin_lock_bh(&(drv)->Op_Control->lock);	\
	(drv)->Op_Control->P_Control.value[0] = cmd;	\
	spin_unlock_bh(&(drv)->Op_Control->lock);}

// OControl.State - lesen
#define OBJS_DRV_RD_STATE(drv,state)	{	\
	spin_lock_bh(&(drv)->Op_Control->lock);	\
	state = (drv)->Op_Control->P_Control.value[1];	\
	spin_unlock_bh(&(drv)->Op_Control->lock);}

// OControl.State - schreiben
#define OBJS_DRV_WR_STATE(drv,state)	{	\
	spin_lock_bh(&(drv)->Op_Control->lock);	\
	(drv)->Op_Control->P_Control.value[1] = state;	\
	spin_unlock_bh(&(drv)->Op_Control->lock);}

// .command & .state - lesen syncron Access
#define OBJS_DRV_RD_COMMAND_AND_STATE(drv,cmd, state)	{ 	\
	spin_lock_bh(&(drv)->Op_Control->lock);	\
	cmd = (drv)->Op_Control->P_Control.value[0];	\
	state = (drv)->Op_Control->P_Control.value[1];	\
	spin_unlock_bh(&(drv)->Op_Control->lock);}

// .command & .state - write syncron Access
#define OBJS_DRV_WR_COMMAND_AND_STATE(drv,cmd, state)	{ 	\
	spin_lock_bh(&(drv)->Op_Control->lock);	\
	(drv)->Op_Control->P_Control.value[0] = cmd;	\
	(drv)->Op_Control->P_Control.value[1] = state;	\
	spin_unlock_bh(&(drv)->Op_Control->lock);}

// access = z.B. data, l2->pdata
#define OBJS_CREATE_DRV_PTR(access) struct tdio_drv *drv = (struct tdio_drv *)(access)
#define OBJS_CREATE_IDEV_PTR		struct tdio_dev_inp   * idev = drv->Op_Inputs
#define OBJS_CREATE_ODEV_PTR		struct tdio_dev_outp  * odev = drv->Op_Outputs

// Beschreiber z.B. {"triac 0", "out-0000000-000001000-00000000"} 
struct t_dig {
	char name [PROPERTY_SIZE+1];
	unchar	size;
//	char s_mask[PROPERTY_SIZE+1];
};


///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
extern char	dio_json[JSON_LIMIT_SIZE];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct tdio_drv 		dio_O_Mct;			// Central Driver Object
extern struct tdio_dev_outp dio_O_Outputs;		// Central Device Output Object
extern struct tdio_dev_inp 	dio_O_Inputs;		// Central Device Input Object

///*********************************************************
///  TEMPLATE's
///*********************************************************
extern struct t_dig DOs[OUTPUT_ELEMENTS];
extern struct t_dig	DIs[INPUT_ELEMENTS];

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void 	dio_objects_create(void);
extern void 	dio_objects_generic(void);
extern ssize_t 	dio_objects_json(const char *);

#endif 
