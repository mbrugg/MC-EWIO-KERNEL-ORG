/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

*********************************************************************************/
#ifndef MCT_DI10_OBJS_H_
	#define MCT_DI10_OBJS_H_

#include <linux/platform_device.h>
#include "../mct_paa.h"

#include "../../mct_types.h"
#include "../../mct_json.h"

#define DEV_MCT_PAA_DI10_IN_MAX			1			// IN-Geräte
#define DEV_MCT_PAA_DI10_IN_CHAN_MAX	10			// IN-Kanäle pro IN-Gerät

#define DEV_MCT_PAA_DI10_IN_CHAN_0		0
#define DEV_MCT_PAA_DI10_IN_CHAN_1		1
#define DEV_MCT_PAA_DI10_IN_CHAN_2		2
#define DEV_MCT_PAA_DI10_IN_CHAN_3		3
#define DEV_MCT_PAA_DI10_IN_CHAN_4		4
#define DEV_MCT_PAA_DI10_IN_CHAN_5		5
#define DEV_MCT_PAA_DI10_IN_CHAN_6		6
#define DEV_MCT_PAA_DI10_IN_CHAN_7		7
#define DEV_MCT_PAA_DI10_IN_CHAN_8		8
#define DEV_MCT_PAA_DI10_IN_CHAN_9		9

///********************
/// CONTROL
///*********************
typedef struct  {
	struct paa_device *	paa;	
	
	struct timer_list 	timer;
	atomic_t			timer_stop_job;
	struct completion	timer_stop_quit;

	atomic_t			msg_stop_job;			// Message Stop
	struct completion	msg_stop_quit;
	spinlock_t			msg_lock;				// Message-Protection
	struct paa_message 	msg;
	u8					tx_req;
	u8					tx_len;
	u8 					tx_buf[PAA_BUFFER_SIZE];// paa_slave to paa_master
	u8					rx_rsp;
	u8					rx_len;
	u8					rx_buf[PAA_BUFFER_SIZE];// paa_master to paa_client

} di10_control;			

/// Driver-Objects
typedef struct  {					// obj_physic
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_BUSLayer;		// BUS
	prop_str 		P_INFO;			// SLOT-Message (online, offline, confused ...)
} obj_physic;

// Device-Objects
typedef struct  {					// obj_value
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_Name;					
	prop_byte 		P_Size;
	prop_u16 		P_Value;
} obj_value;

// Zentrale Inputput-Device-Struktur
struct tdi10_dev_inp {						// Input-Object
	char *				pname;
	obj_value  * 		Op_Value;
};

///	Makros: Device Inputs
// Inp.OValue.PValue.value - lesen
#define OBJS_IDEV_RD_INPUT(idev,dst)	{	\
		spin_lock_bh(&(idev)->Op_Value->lock);	\
		dst = (idev)->Op_Value->P_Value.value;	\
		spin_unlock_bh(&(idev)->Op_Value->lock);}

// Inp.OValue.PValue.value - schreiben
#define OBJS_IDEV_WR_INPUT(idev,val)	{	\
			spin_lock_bh(&(idev)->Op_Value->lock);	\
			(idev)->Op_Value->P_Value.value = val;	\
			spin_unlock_bh(&(idev)->Op_Value->lock);}

// Zentrale Driverstruktur (only Pointer-SLOT's)
// ACHTUNG: Das Element *obj_control muss immer an der 1. Stelle stehen!
struct tdi10_drv {
	obj_control 		* 	Op_Control;				// Slot:  Control-Object
	di10_control		* 	ct;						// Slot:  globale Steuerung	
	obj_physic	 		* 	Op_Physic;				// Slot:  Physic-Object	
	struct tdi10_dev_inp * 	Op_Inputs;
};

///	Makros: Driver
// OControl.Command - lesen
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

// OPhysic.
#define OBJS_DRV_WR_INFO(drv,msg,len)	{	\
		spin_lock_bh(&(drv)->Op_Physic->lock);	\
		memcpy(&(drv)->Op_Physic->P_INFO.value,msg,len);	\
		spin_unlock_bh(&(drv)->Op_Physic->lock);	}


// access = z.B. data, l2->pdata
#define OBJS_CREATE_DRV_PTR(access) struct tdi10_drv *drv = (struct tdi10_drv *)(access)
// input-device
#define OBJS_CREATE_IDEV_PTR		struct tdi10_dev_inp  *idev = drv->Op_Inputs

// Beschreiber z.B. {"triac 0", "out-0000000-000001000-00000000"} 
struct t_dig {
	char name [PROPERTY_SIZE+1];
	u32	 size;
	char s_mask[PROPERTY_SIZE+1];
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
extern char					di10_json[JSON_LIMIT_SIZE];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct tdi10_drv 		di10_O_Mct;								// Driver Object
extern struct tdi10_dev_inp 	di10_O_Inputs;							// Device Input Object
///*********************************************************
///  TEMPLATE's
///*********************************************************
extern struct t_dig	DIs[DEV_MCT_PAA_DI10_IN_CHAN_MAX];

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void 	di10_objects_create(void);
extern void 	di10_objects_generic(void);
extern ssize_t 	di10_objects_json(const char *);

#endif 
