/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

	BugFix	18.01.12	O_Timeouts.P_Code[_idx].value = 0 	  

*********************************************************************************/
#include "../mct_debug.h"			// debug-support
#include "mct_paa_mux_objs.h"

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char						mux_json[JSON_LIMIT_SIZE];		// JSON formatierte Eingabe
STATIC int 					command = 0;					// START/STOP
STATIC int					slots=LBUS_DEF_SLOTS_MAX;		// nutzbare SLOT-Anzahl

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string( json, mux_json, sizeof(mux_json), S_IRUGO);	
module_param( command, int, S_IRUGO);
module_param( slots, int, S_IRUGO);

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************

// Driver
struct paa_mux	*			Op_PaaMux;
static obj_control 			O_Control;					// Start/Stop-Kontrolle
static struct obj_physic 	O_Physic;					// physikalischer Beschreiber 
static struct obj_lbus 		O_LBus;						// LBus
static struct obj_timeouts 	O_Timeouts;					// Timeouts

///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
		{.n = {.type=nType_unused}}};

STATIC u_node jsonkey_slots[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_LBUS_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_SLOT_Ss}},	\
		{.n = {.type=nType_unused}}};

///*********************************************************
///  OBJECT-FUNTIONS
///*********************************************************
void paa_mux_objects_create(struct paa_mux * as) {
	unsigned int _idx;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/// DRIVER
	/// O_PAA_MUX: O_Control-Setup
	O_Control.pname 				= OBJECT_CONTROL_S;		// OControl
	spin_lock_init(&O_Control.lock);
	O_Control.P_Control.pname[0] 	= PROPERTY_COMMAND_S;
	O_Control.P_Control.pname[1] 	= PROPERTY_STATE_S;
	O_Control.P_Control.value[0] 	= e_stop; 				// external Command
	O_Control.P_Control.value[1] 	= e_stop;				// internal state 
	/// O_PAA_MUX: O_Physic-Setup
	O_Physic.pname					= OBJECT_PHYSIC_S;		// OPhysic
	spin_lock_init(&O_Physic.lock);
	O_Physic.P_BUSLayer.pname		= PROPERTY_BUS_LAYER_S;	
	O_Physic.P_SPILayer.pname		= PROPERTY_SPI_LAYER_S;
	
	///  O_PAA_MUX: O_LBus-Setup
	O_LBus.pname 			= OBJECT_LBUS_S;		// OLBus:{
	spin_lock_init(&O_LBus.lock);
	O_LBus.P_Slots.pname	= PROPERTY_SLOT_Ss;		// Slots: ...,
	O_LBus.P_Slots.value	= LBUS_DEF_SLOTS_MIN;
	O_LBus.P_Msg.pname		= PROPERTY_MESSAGE_S;	// Message: ...,
	// O_Slots
	O_LBus.O_Slots.pname	= OBJECT_SLOT_Ss;		// oSLots
	O_LBus.O_Slots.pmsg		= PROPERTY_MESSAGE_S;	// Message:
	O_LBus.O_Slots.ext_cfg	= "ext_cfg";			// 1= Gerätekonfiguration extern angelegt!
	O_LBus.O_Slots.ext_ini	= "ext_ini";			// 1= Initialisierung extern angelegt!
	O_LBus.O_Slots.int_cfg	= "int_cfg";			// 1= Gerätekonfiguration beendet!
	O_LBus.O_Slots.int_ini	= "int_ini";			// 1= Initialisierung beendet!
	O_LBus.O_Slots.cfg		= PROPERTY_CFG_S;		// Backup ins File
	O_LBus.O_Slots.cfg_in	= PROPERTY_CFG_IN_S;	// verwendete Länge Input-Buffer	
	O_LBus.O_Slots.cfg_out	= PROPERTY_CFG_OUT_S;	// verwendete Länge Output-Buffer
	O_LBus.O_Slots.cfg_cfg	= PROPERTY_CFG_CFG_S;	// verwendete Länge Configuration-Buffer
	O_LBus.O_Slots.buf_nam 	= PROPERTY_NAME_S;
	O_LBus.O_Slots.buf_ver	= PROPERTY_VERSION_S;
	O_LBus.O_Slots.buf_in	= "buf_in";	
	O_LBus.O_Slots.buf_out	= "buf_out";
	O_LBus.O_Slots.buf_cfg	= "buf_cfg";
	/// O_PAA_MUX: O_Errors-Setup
	O_Timeouts.pname = OBJECT_TIMEOUT_Ss;				// oTimeouts
	for(_idx= 0; _idx < LBUS_DEF_SLOTS_MAX; _idx++) {
		spin_lock_init(&O_Timeouts.lock);
		O_Timeouts.P_Slot[_idx].pname	= PROPERTY_SLOT_S;				// Slot: Nummer
		O_Timeouts.P_Slot[_idx].value	=_idx; 
		O_Timeouts.P_Code[_idx].pname	= PROPERTY_TIMEOUT_CODE_S;		// Code: Nummer
// BugFix	18.01.12	O_Timeouts.P_Code[_idx].value	=_idx;
		O_Timeouts.P_Code[_idx].value	= 0;
		O_Timeouts.P_Counter[_idx].pname = PROPERTY_TIMEOUT_COUNTER_S;	// Counter: Nummer
		O_Timeouts.P_Counter[_idx].value = 0;
	}
	/// O_MCT S
	as->Op_Control 					= &O_Control;	// O_Control
	as->Op_Physic 					= &O_Physic;	// O_Physic
	as->Op_LBus						= &O_LBus;		// Slot: O_LBus
	as->Op_Timeouts					= &O_Timeouts;	// O_Timeouts[]
}

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
// 				Eingabeparameter beim Treiberstart checken
// 				einstellbar sind:  	drv.command
//									drv.slots
//***********************************************************************************
void paa_mux_objects_generic(struct paa_mux * as) {		
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	// HACK SHOW HACK
	// command = e_start;
	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{		// ARG: "command" übernehmen
		as->Op_Control->P_Control.value[0] = command;   // Externes Kommando in [0]
		as->Op_Control->P_Control.value[1] = command;	// Interne Stati in [1]
	}	
	/// *** Property SLOTS
	if(slots > LBUS_DEF_SLOTS_MAX)
		slots = LBUS_DEF_SLOTS_MAX;
	if(slots < LBUS_DEF_SLOTS_MIN)
		slots = LBUS_DEF_SLOTS_MIN;
	as->Op_LBus->P_Slots.value = slots;
	return;
}

//***********************************************************************************
// Funktion:	Der Aufruf erfolgt automatisch, wenn der json-Parameter='{\"...\"}'
//				mit einem String aufgerufen wird. Als Format wird JSON-Syntax erwartet.
//				Folgende JSON-Elemente werden gesucht  und in die Werte in den 
//				entsprechenden Driver-Übergabeparametern abgelegt. 
//
// 				<JSON>
//				oControl.Command (e_Start,e_Stop Driver)					==> command
//				oLBus.Slots.maximum(LBUS_DEF_SLOTS_MIN...LBUS_DEF_SLOTS_MAX	==> slots
//
//				Anschließend wird paa_mux_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t paa_mux_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	char  			_buf[JSON_LIMIT_SIZE+1];

	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	if(len == 0 || len > JSON_LIMIT_SIZE+1)	{
		DEBUG(MCT_DEBUG_LEVEL3, "%s %s(): %s\n", __FILE__,__FUNCTION__, "Parameter json with corrupt limit size!");
		return -1;
	}
	
	DEBUG(MCT_DEBUG_LEVEL3, "JSON: Start\n");
	///------------------
	/// DRIVER
	///------------------
	/// COMMAND 
	memcpy(_buf,json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_command[0], sizeof(jsonkey_command));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: Integer 
	if(!ctx.err.number && ctx.val.v.type == vType_int && ctx.val.vI.value)
		command = ctx.val.vI.value;					/// ARGUMENT: command
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DRV: command %d\n", command);
	/// SLOTS
	memcpy(_buf,json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_slots[0], sizeof(jsonkey_slots));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: Integer 
	if(!ctx.err.number && ctx.val.v.type == vType_int && ctx.val.vI.value)
		slots = ctx.val.vI.value;					/// ARGUMENT: slots
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DRV: slots %d\n", slots);

	DEBUG(MCT_DEBUG_LEVEL3, "JSON: End\n");
	return len;
}
