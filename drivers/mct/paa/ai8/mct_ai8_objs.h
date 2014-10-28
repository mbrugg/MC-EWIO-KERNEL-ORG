/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$	

*********************************************************************************/
#ifndef MCT_AI8_OBJS_H_
	#define MCT_AI8_OBJS_H_
#include <linux/platform_device.h>
#include "../mct_paa.h"

#include "../../mct_types.h"
#include "../../mct_json.h"
#include "../../mct_boxes.h"
#include "../../mct_aio_uni.h"			// analog In/Output Defs

#define DEV_MCT_PAA_AI8_IN_MAX			8	// IN-Geräte
#define DEV_MCT_PAA_AI8_IN_CHAN_MAX		1	// IN-Kanäle pro IN-Gerät
#define DEV_MCT_PAA_AI8_IN_0			0	// Gerät 0
#define DEV_MCT_PAA_AI8_IN_1			1	// Gerät 1
#define DEV_MCT_PAA_AI8_IN_2			2	// Gerät 2
#define DEV_MCT_PAA_AI8_IN_3			3	// Gerät 3
#define DEV_MCT_PAA_AI8_IN_4			4	// Gerät 4
#define DEV_MCT_PAA_AI8_IN_5			5	// Gerät 5
#define DEV_MCT_PAA_AI8_IN_6			6	// Gerät 6
#define DEV_MCT_PAA_AI8_IN_7			7	// Gerät 7

#define DEV_MCT_PAA_AI8_CFG_CFG			8	// in Byte erforderliche Konfigurationslänge
#define DEV_MCT_PAA_AI8_CFG_IN 			32	// in Byte erforderliche Eingangsdatenlänge

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
	atomic_t			msg_byebye_job;			// Message ByeBye an Master
	struct completion	msg_byebye_quit;
	spinlock_t			msg_lock;				// Message-Protection
	struct paa_message 	msg;
	u8					tx_req;
	u8					tx_len;
	u8 					tx_buf[PAA_BUFFER_SIZE];// paa_slave to paa_master
	u8					rx_rsp;
	u8					rx_len;
	u8					rx_buf[PAA_BUFFER_SIZE];// paa_master to paa_client

} ai8_control;			

/// Driver-Objects
typedef struct  {					// obj_physic
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_BUSLayer;		// BUS
	prop_str 		P_INFO;			// SLOT-Message (online, offline, confused ...)
} obj_physic;


// Device-Objects
typedef struct  {					// obj_value_in
	char   *		pname;
	spinlock_t		lock;	
	prop_str 		P_Name;	
	prop_byte 		P_Size;
	prop_u32 		P_Value;
} obj_value_in;

typedef struct  {
	char   * 		pname;			// "OType"
	struct prop_box P_Type;
} obj_type;

// Zentrale Input-Device-Struktur
struct tai8_dev_inp {				// Input-Object
	char *				pname;
	obj_value_in  * 	Op_Value;	// Slot: analoger Eingangswert
	obj_type *			Op_Type;	// Slot: Type, d.h hier Messbereich
};

// Zentrale Driverstruktur (only Pointer-SLOT's)
// ACHTUNG: Das Element *obj_control muss immer an der 1. Stelle stehen!
struct tai8_drv {
	obj_control 		* 	Op_Control;				// Slot:  Control-Object 
	ai8_control			* 	ct;						// Slot:  globale Steuerung	
	obj_physic	 		* 	Op_Physic;				// Slot:  Physic-Object	
	struct tai8_dev_inp *	Op_Inputs[DEV_MCT_PAA_AI8_IN_MAX]; // Slots: Input-Objects
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
extern char					ai8_json[JSON_LIMIT_SIZE];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct tai8_drv 			ai8_O_Mct;								 // Driver Object
extern struct tai8_dev_inp 		ai8_O_Inputs[DEV_MCT_PAA_AI8_IN_MAX];	 // Device Input Object

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void 	ai8_objects_create(void);
extern void 	ai8_objects_generic(void);
extern ssize_t 	ai8_objects_json(const char *);

extern const char * ai_label_tab[];

#endif 
