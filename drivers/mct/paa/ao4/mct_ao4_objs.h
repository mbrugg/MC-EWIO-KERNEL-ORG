/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

*********************************************************************************/
#ifndef MCT_AO4_OBJS_H_
	#define MCT_AO4_OBJS_H_

#include <linux/platform_device.h>
#include "../mct_paa.h"

#include "../../mct_types.h"
#include "../../mct_json.h"

#define DEV_MCT_PAA_AO4_OUT_MAX			4	// OUT-Geräte
#define DEV_MCT_PAA_AO4_OUT_CHAN_MAX	1	// OUT-Kanäle pro OUT-Gerät
#define DEV_MCT_PAA_AO4_OUT_0			0	// Gerät 0
#define DEV_MCT_PAA_AO4_OUT_1			1	// Gerät 1
#define DEV_MCT_PAA_AO4_OUT_2			2	// Gerät 2
#define DEV_MCT_PAA_AO4_OUT_3			3	// Gerät 3

#define DEV_MCT_PAA_AO4_CFG_OUT 		8	// in Byte erforderliche Ausgangsdatenlänge

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

} ao4_control;			


/// Driver-Objects
typedef struct  {					// obj_physic
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_BUSLayer;		// BUS
	prop_str 		P_INFO;			// SLOT-Message (online, offline, confused ...)
} obj_physic;

// Device-Objects
typedef struct  {					// obj_value_out
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_Name;
	prop_byte 		P_Size;
	prop_u16 		P_Value;
} obj_value_out;

// Zentrale Output-Device-Struktur
struct tao4_dev_outp {				// Output-Object
	char *				pname;
	obj_value_out  * 	Op_Value;	// Slot: analoger Ausgabewert
};

// Zentrale Driverstruktur (only Pointer-SLOT's)
// ACHTUNG: Das Element *obj_control muss immer an der 1. Stelle stehen!
struct tao4_drv {
	obj_control 		* 	Op_Control;				// Slot:  Control-Object
	ao4_control			* 	ct;						// Slot:  globale Steuerung	
	obj_physic	 		* 	Op_Physic;				// Slot:  Physic-Object	
	struct tao4_dev_outp *	Op_Outputs[DEV_MCT_PAA_AO4_OUT_MAX]; // Slots: Output-Objects
};

// Beschreiber z.B. {"triac 0", "out-0000000-000001000-00000000"} 
struct t_alg {
	char name [PROPERTY_SIZE+1];
	u32	 size;
	char s_mask[PROPERTY_SIZE+1];
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
extern char					ao4_json[JSON_LIMIT_SIZE];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct tao4_drv 			ao4_O_Mct;								 // Driver Object
extern struct tao4_dev_outp 	ao4_O_Outputs[DEV_MCT_PAA_AO4_OUT_MAX];	 // Device Output Object
///*********************************************************
///  TEMPLATE's
///*********************************************************
extern struct t_alg	AOs[DEV_MCT_PAA_AO4_OUT_MAX];

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void 	ao4_objects_create(void);
extern void 	ao4_objects_generic(void);
extern ssize_t 	ao4_objects_json(const char *);

#endif 
