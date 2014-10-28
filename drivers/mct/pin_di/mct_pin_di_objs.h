/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011
 	
 	Description:

 *******************************************************************************/
#ifndef MCT_PIN_DI_OBJS_H_
	#define MCT_PIN_DI_OBJS_H_

#include <linux/semaphore.h>
#include "../mct_types.h"
#include "../mct_json.h"


#define MODE_DIRECT					0
#define MODE_S0						1

#define PROPERTY_MODE_DIRECT_S		"direct"
#define PROPERTY_MODE_S0_S			"S0"

typedef struct  {
	char   * 		pname;			// "OPhysic"
	spinlock_t		lock;
	prop_byte 		P_Pin;			//gpio-pin
	prop_byte 		P_Tri;			//tristate -pullup/pulldowb
} obj_physic;

typedef struct  {
	char   * 		pname;			// "OMode"
	spinlock_t		lock;
	prop_str		P_Name;			// Name: direct, S0 ...	
	prop_byte 		P_Value;		// Value: 0,1,
} obj_mode;

typedef struct  {					// "OValue"
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_Name;
	prop_byte 		P_Size;
	prop_u64 		P_Value;
} obj_value;

typedef struct  {						// "OFreeze"
	char * 			pname;
	spinlock_t		lock;
	prop_bool		P_Init;				// Init-Flag
	prop_bool		P_InitExport;		// 1 = Init-Flag an den ICOM-Server (DIO) senden
	prop_byte 		P_Size;				// freeze-Datenbreite
	prop_u64 		P_Value;			// freeze-Value
} obj_freeze;


/// Zentrale Device-Struktur
struct tdi_dev_inp {					// Input-Object
	char *				pname;
	obj_mode	* 		Op_Mode;
	obj_value	* 		Op_Value;
	obj_freeze	*		Op_Freeze;
};

///	Makros: Zugriff auf Device-Elemente
// oValue.Value - lesen
#define OBJS_IDEV_RD_VALUE(idev,dst)	{	\
	spin_lock_bh(&(idev)->Op_Value->lock);	\
	dst = (idev)->Op_Value->P_Value.value;	\
	spin_unlock_bh(&(idev)->Op_Value->lock);}

// oValue.Value - schreiben
#define OBJS_IDEV_WR_VALUE(idev,src)	{	\
	spin_lock_bh(&(idev)->Op_Value->lock);	\
	(idev)->Op_Value->P_Value.value = src;	\
	spin_unlock_bh(&(idev)->Op_Value->lock);}

// oValue.Value - inkrementieren um 1
#define OBJS_IDEV_INC_VALUE(idev)	{	\
	spin_lock_bh(&(idev)->Op_Value->lock);	\
	(idev)->Op_Value->P_Value.value++;	\
	spin_unlock_bh(&(idev)->Op_Value->lock);	}

// oFreeze.Value - lesen
#define OBJS_IDEV_RD_FREEZE(idev,dst)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		dst = (idev)->Op_Freeze->P_Value.value;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oFreeze.Value - schreiben
#define OBJS_IDEV_WR_FREEZE(idev,src)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		(idev)->Op_Freeze->P_Value.value = src;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oFreeze.Init - lesen (0 oder 1)
#define OBJS_IDEV_RD_INIT(idev,dst)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		dst = (idev)->Op_Freeze->P_Init.value;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oFreeze.Init - schreiben (0 oder 1)
#define OBJS_IDEV_WR_INIT(idev,src)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		(idev)->Op_Freeze->P_Init.value = src;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oFreeze.InitExport - lesen (0 oder 1)
#define OBJS_IDEV_RD_INIT_EXPORT(idev,dst)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		dst = (idev)->Op_Freeze->P_InitExport.value;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oFreeze.InitExport - schreiben (0 oder 1)
#define OBJS_IDEV_WR_INIT_EXPORT(idev,src)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		(idev)->Op_Freeze->P_InitExport.value = src;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oFreeze.Size - lesen
#define OBJS_IDEV_RD_FREEZE_SIZE(idev,dst)	{	\
		spin_lock_bh(&(idev)->Op_Freeze->lock);	\
		dst = (idev)->Op_Freeze->P_Size.value;	\
		spin_unlock_bh(&(idev)->Op_Freeze->lock);}

// oMode.Value - lesen
#define OBJS_IDEV_RD_MODE(idev,mod)	{	\
	spin_lock_bh(&(idev)->Op_Mode->lock);	\
	mod = (idev)->Op_Mode->P_Value.value;	\
	spin_unlock_bh(&(idev)->Op_Mode->lock);}

/// Zentrale Driverstruktur
struct tdi_drv {
	obj_control		   * Op_Control;
	obj_physic		   * Op_Physic;
	struct tdi_dev_inp * Op_Inputs;		
	struct	timer		OT;
};

///	Makros: Zugriff auf Driver-Elemente
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

// access = z.B. data, l2->pdata
#define OBJS_CREATE_DRV_PTR(access)	struct tdi_drv *drv = (struct tdi_drv *)(access)
#define OBJS_CREATE_IDEV_PTR struct tdi_dev_inp *idev = drv->Op_Inputs

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
extern char	di_json[JSON_LIMIT_SIZE];

///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct tdi_drv 		di_O_Mct;
extern struct tdi_dev_inp 	di_O_Inputs;

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void 	di_objs_create(struct tdi_drv * drv);
extern void 	di_objs_generic(void);
extern ssize_t 	di_objs_json(const char *);

#endif /* MCT_PIN_DI_OBJECTS_H_ */
