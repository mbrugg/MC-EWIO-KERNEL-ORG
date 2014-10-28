/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

*********************************************************************************/
#ifndef MCT_PAA_MUX_OBJS_H_
	#define MCT_PAA_MUX_OBJS_H_

#include "../mct_types.h"
#include "../mct_json.h"
#include "mct_paa_lbus.h"		// struct: lower_bus
#include "mct_paa_spi.h"		// struct: lower_device


#define PROPERTY_SPI_LAYER_S	"SPI"
#define PROPERTY_BUS_LAYER_S	"BUS"
struct obj_physic {				// obj_physic
	char   *		pname;		// oPhysic
	spinlock_t		lock;
	prop_str 		P_BUSLayer;	
	prop_str 		P_SPILayer;
} ;

#define PROPERTY_CFG_S			"cfg"
#define PROPERTY_CFG_IN_S		"cfg_in"
#define PROPERTY_CFG_OUT_S		"cfg_out"
#define PROPERTY_CFG_CFG_S		"cfg_cfg"
struct obj_slots {				// obj_slots
	char*	pname;				// oSlots
	char*	pmsg;				// Message
	//--------------
	// FS-Bezeichner
	//--------------
	char*	ext_cfg;			// 1= Gerätekonfiguration extern angelegt!
	char*	ext_ini;			// 1= Initialisierung extern angelegt!
	char*	int_cfg;			// 1= Gerätekonfiguration beendet!
	char*	int_ini;			// 1= Initialisierung beendet!
	char*	cfg;				// Backup ins File
	char*	cfg_in;				// verwendete Länge Input-Buffer				
	char*	cfg_out;			// verwendete Länge Output-Buffer
	char* 	cfg_cfg;			// verwendete Länge Configuration-Buffer
	char*	buf_nam;
	char* 	buf_ver;
	char* 	buf_in;	
	char* 	buf_out;
	char* 	buf_cfg;
};

#define PROPERTY_TIMEOUT_CODE_S		"Code"
#define PROPERTY_TIMEOUT_COUNTER_S	"Counter"

#define MUX_TIMEOUT_ON_DATA_MAX			30
#define MUX_TIMEOUT_ON_CONFIG_MAX		3
#define MUX_TIMEOUT_ON_PING_MAX			3

struct obj_timeouts {			// obj_timeouts
	char   *		pname;		// oTimeouts
	spinlock_t		lock;
	prop_byte 		P_Slot[LBUS_DEF_SLOTS_MAX];  	// "Slot"	: %d 	= Slotnummer 
	prop_byte 		P_Code[LBUS_DEF_SLOTS_MAX];	 	// "Code"	: %d 	= letzter Fehler 
	prop_byte 		P_Counter[LBUS_DEF_SLOTS_MAX]; 	// "Counter": %d 	= Wiederholung des letzten Fehlers
};

struct obj_lbus {				// obj_lbus
	char   *			pname;	// "oLBus"
	spinlock_t			lock;
	prop_byte 			P_Slots; // "Slots"	: %d 	= Slotanzahl 
	prop_str 			P_Msg;	 // "Message": %s	= Busnachrichten
	struct obj_slots 	O_Slots; // Infos der Clients
};

struct paa_mux {
	obj_control 			*Op_Control;	// Control-Object
	struct obj_physic		*Op_Physic;		// Physic-Object	
	struct obj_lbus			*Op_LBus;		// Slot: LBus-Object 
	struct obj_timeouts		*Op_Timeouts;	// Slot: Timeouts-Object-Array
	struct timer			OT;				// ObjectTimer
	struct platform_device	*pdev;
	struct lower_device		*ldev;			// lower device
	struct lower_bus		lbus;			// lower bus
	spinlock_t				transfer_lock;
	atomic_t				transfer_stop;
	struct list_head		transfer_list;
	struct paa_device		*stay;
	u8						stopping;
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
extern char		mux_json[JSON_LIMIT_SIZE];
///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct 	paa_mux *	Op_PaaMux;
///*********************************************************
///  FUNCTION's
///*********************************************************
extern 			void paa_mux_objects_create(struct paa_mux * as);
extern 			void paa_mux_objects_generic(struct paa_mux * as);
extern ssize_t 	paa_mux_objects_json(const char *);

#endif
