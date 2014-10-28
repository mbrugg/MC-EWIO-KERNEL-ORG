/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

*********************************************************************************/
#ifndef MCT_DIO42_OBJS_H_
	#define MCT_DIO42_OBJS_H_

#include <linux/platform_device.h>
#include "../mct_paa.h"

#include "../../mct_types.h"
#include "../../mct_json.h"

#define DEV_MCT_PAA_DIO42_OUT_MAX		1	// OUT-Geräte
#define DEV_MCT_PAA_DIO42_OUT_CHAN_MAX	2	// OUT-Kanäle pro OUT-Gerät
#define DEV_MCT_PAA_DIO42_OUT_CHAN_0	0
#define DEV_MCT_PAA_DIO42_OUT_CHAN_1	1

#define DEV_MCT_PAA_DIO42_IN_MAX		1	// IN-Geräte
#define DEV_MCT_PAA_DIO42_IN_CHAN_MAX	4	// IN-Kanäle pro IN-Gerät
#define DEV_MCT_PAA_DIO42_IN_CHAN_0		0
#define DEV_MCT_PAA_DIO42_IN_CHAN_1		1
#define DEV_MCT_PAA_DIO42_IN_CHAN_2		2
#define DEV_MCT_PAA_DIO42_IN_CHAN_3		3


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

} dio42_control;			


/// Driver-Objects
typedef struct  {					// obj_physic
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_BUSLayer;		// BUS
	prop_str 		P_INFO;			// SLOT-Message (online, offline, confused ...)
} obj_physic;

// Device-Objects
typedef struct  {					// obj_mask
	char   *		pname;
	spinlock_t		lock;
	prop_byte 		P_Value;
	prop_byte 		P_Size;
} obj_mask;

typedef struct  {					// obj_value
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_Name;					
	prop_byte 		P_Size;
	prop_byte 		P_ValueOut;
	prop_byte 		P_ValueIn;
} obj_value;

// Zentrale Output-Device-Struktur
struct tdio42_dev_outp {				// Output-Object
	char *				pname;
	obj_value  * 		Op_Value;
	obj_mask *			Op_Mask;		// Handmaske
};

// Zentrale Input-Device-Struktur
struct tdio42_dev_inp {					// Input-Object
	char *				pname;
	obj_value  * 		Op_Value;
};

// Zentrale Driverstruktur (only Pointer-SLOT's)
// ACHTUNG: Das Element *obj_control muss immer an der 1. Stelle stehen!
struct tdio42_drv {
	obj_control 			* 	Op_Control;		// Slot:  Control-Object
	dio42_control			* 	ct;				// Slot:  globale Steuerung	
	obj_physic	 			* 	Op_Physic;		// Slot:  Physic-Object	
	struct tdio42_dev_outp 	* 	Op_Outputs;
	struct tdio42_dev_inp 	* 	Op_Inputs;
};

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
extern char					dio42_json[JSON_LIMIT_SIZE];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct tdio42_drv 		dio42_O_Mct;			// Driver Object
extern struct tdio42_dev_outp 	dio42_O_Outputs;		// Device Output Object
extern struct tdio42_dev_inp 	dio42_O_Inputs;			// Device Input Object
///*********************************************************
///  TEMPLATE's
///*********************************************************
extern struct t_dig	DOs[DEV_MCT_PAA_DIO42_OUT_CHAN_MAX];
extern struct t_dig	DIs[DEV_MCT_PAA_DIO42_IN_CHAN_MAX];

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void 	dio42_objects_create(void);
extern void 	dio42_objects_generic(void);
extern ssize_t 	dio42_objects_json(const char *);

#endif 
