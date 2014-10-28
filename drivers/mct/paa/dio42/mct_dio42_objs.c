/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"
#include "../../mct_dio_uni.h"		// digital In/Output Defs
#include "mct_dio42_objs.h"			// objects

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

///*********************************************************
/// TEMPLATES
///*********************************************************
struct t_dig DOs[DEV_MCT_PAA_DIO42_OUT_CHAN_MAX] = {
		{"DO1 (12-11-14)",SIZE_VALUE1,"0x01"},
		{"DO2 (22-21-24)",SIZE_VALUE1,"0x02"}
};

struct t_dig DIs[DEV_MCT_PAA_DIO42_IN_CHAN_MAX] = {
		{"DI1 (1/C1)",SIZE_VALUE1,"0x01"},
		{"DI2 (2/C1)",SIZE_VALUE1,"0x02"},
		{"DI3 (3/C1)",SIZE_VALUE1,"0x04"},
		{"DI4 (4/C1)",SIZE_VALUE1,"0x08"}
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char					dio42_json[JSON_LIMIT_SIZE];	// JSON formatierte Eingabe
STATIC int 				command;					// START/STOP
// DEVICE-ARGs
STATIC ushort 			output;						// 8bit Ausgang (2bit genutzt)	
STATIC char 			out_name_0[PROPERTY_SIZE+1];
STATIC char 			out_name_1[PROPERTY_SIZE+1];

STATIC ushort 			input;						// 8bit Eingang (4bit genutzt)
STATIC char 			in_name_0[PROPERTY_SIZE+1];
STATIC char 			in_name_1[PROPERTY_SIZE+1];
STATIC char 			in_name_2[PROPERTY_SIZE+1];
STATIC char 			in_name_3[PROPERTY_SIZE+1];


// QUEUE mit out-name Zeigern 
STATIC  char * const  out_name_queue[DEV_MCT_PAA_DIO42_OUT_CHAN_MAX] = 	\
{	out_name_0,out_name_1};

// QUEUE mit in-name Zeigern 
STATIC  char * const  in_name_queue[DEV_MCT_PAA_DIO42_IN_CHAN_MAX] = 	\
{	in_name_0,in_name_1,in_name_2,in_name_3 };

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string	( json, dio42_json, sizeof(dio42_json), S_IRUGO);	
module_param		( command, int, S_IRUGO);
module_param		( output, ushort, S_IRUGO);				// output
module_param		( input, ushort, S_IRUGO);				// input
module_param_string( out_name_0, out_name_0, sizeof(out_name_0), S_IRUGO);
module_param_string( out_name_1, out_name_1, sizeof(out_name_1), S_IRUGO);

module_param_string( in_name_0, in_name_0, sizeof(in_name_0), S_IRUGO);
module_param_string( in_name_1, in_name_1, sizeof(in_name_1), S_IRUGO);
module_param_string( in_name_2, in_name_2, sizeof(in_name_2), S_IRUGO);
module_param_string( in_name_3, in_name_3, sizeof(in_name_3), S_IRUGO);

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
// Driver
struct 	tdio42_drv 		dio42_O_Mct;
static obj_control 		O_Control;
static obj_physic		O_Physic;

// Output-Device
struct 	tdio42_dev_outp dio42_O_Outputs;
static obj_mask 		O_Mask;
static obj_value 		O_ValuesOut;

// Input-Device
struct 	tdio42_dev_inp  dio42_O_Inputs;
static obj_value 		O_ValuesIn;

///*********************************************************
///  TEMPLATE's
///*********************************************************
struct t_dig	DOs[DEV_MCT_PAA_DIO42_OUT_CHAN_MAX];
struct t_dig	DIs[DEV_MCT_PAA_DIO42_IN_CHAN_MAX];

///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
			{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
			{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
			{.n = {.type=nType_unused}}};

// oDevices[].oValues[].Value 
STATIC u_node jsonkey_bits[6] =  {	\
			{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
			{.nA = {.type=nType_array,.node=0}},	\
			{.nK = {.type=nType_string,.node= OBJECT_VALUE_Ss}},	\
			{.nA = {.type=nType_array,.node=0}},	\
			{.nK = {.type=nType_string,.node=PROPERTY_VALUE_S}},	\
			{.n = {.type=nType_unused}}};

// oDevices[].oValues[].Name 
STATIC u_node jsonkey_name[6] =  {	\
			{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
			{.nA = {.type=nType_array,.node=0}},	\
			{.nK = {.type=nType_string,.node= OBJECT_VALUE_Ss}},	\
			{.nA = {.type=nType_array,.node=0}},	\
			{.nK = {.type=nType_string,.node=PROPERTY_NAME_S}},	\
			{.n = {.type=nType_unused}}};

///*********************************************************
///  OBJECT-FUNTIONS
///*********************************************************
void dio42_objects_create(void) {
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/// DRIVER
	/// O_MCT: O_Control-Setup
	O_Control.pname = OBJECT_CONTROL_S;
	spin_lock_init(&O_Control.lock);
	O_Control.P_Control.pname[0] = PROPERTY_COMMAND_S;
	O_Control.P_Control.pname[1] = PROPERTY_STATE_S;
	O_Control.P_Control.value[0] = e_stop; 				// external Command
	O_Control.P_Control.value[1] = e_stop;				// internal state 
	/// O_MCT: OPhysic-Setup
	O_Physic.pname				= OBJECT_PHYSIC_S;		// OPhysic	
	spin_lock_init(&O_Physic.lock);
	O_Physic.P_BUSLayer.pname	= PROPERTY_LAYER_BUS_S;	// BUS:
	O_Physic.P_INFO.pname		= PROPERTY_INFO_S;		// Info:
	strcpy(O_Physic.P_INFO.value, LABEL_SLOT_OFFLINE);	// Nachricht
	/// OUTPUT-DEVICE
	dio42_O_Mct.Op_Outputs		= &dio42_O_Outputs;
	O_ValuesOut.pname			= OBJECT_VALUE_Ss;		// OValuesOut
	spin_lock_init(&O_ValuesOut.lock);
	O_ValuesOut.P_Name.pname 	= PROPERTY_NAME_S;
	O_ValuesOut.P_Size.pname 	= PROPERTY_SIZE_S;		// Verarbeitungsbreite
	O_ValuesOut.P_Size.value 	= DEV_MCT_PAA_DIO42_OUT_CHAN_MAX; 
	O_ValuesOut.P_ValueOut.pname = PROPERTY_VALUE_S;
	O_ValuesOut.P_ValueOut.value = 0;
	O_ValuesOut.P_ValueIn.value	 = 0;					// Feedback	
	O_Mask.pname				= OBJECT_MASK_S;		// OMask
	spin_lock_init(&O_Mask.lock);
	O_Mask.P_Size.pname 		= PROPERTY_SIZE_S;
	O_Mask.P_Size.value 		= SIZE_VALUE2;			// 2bit
	O_Mask.P_Value.pname 		= PROPERTY_VALUE_S;
	O_Mask.P_Value.value		= 0;
	/// INPUT-DEVICE
	dio42_O_Mct.Op_Inputs		= &dio42_O_Inputs;
	O_ValuesIn.pname			= OBJECT_VALUE_Ss;		// OValuesIn
	spin_lock_init(&O_ValuesIn.lock);
	O_ValuesIn.P_Name.pname 	= PROPERTY_NAME_S;
	O_ValuesIn.P_Size.pname 	= PROPERTY_SIZE_S;		// Verarbeitungsbreite
	O_ValuesIn.P_Size.value 	= DEV_MCT_PAA_DIO42_IN_CHAN_MAX; 
	O_ValuesIn.P_ValueIn.pname 	= PROPERTY_VALUE_S;
	O_ValuesIn.P_ValueIn.value	= 0;	
//	O_ValuesIn.P_ValueOut.pname = PROPERTY_VALUE_S;		// unbenutzt
//	O_ValuesIn.P_ValueOut.value	= 0;					// unbenutzt
	
	/// ROOT Objekte einhängen!
	dio42_O_Mct.Op_Control 		= &O_Control;		// Slot: O_Control
	dio42_O_Mct.Op_Physic 		= &O_Physic;		// Slot: O_Physic
	dio42_O_Outputs.Op_Value	= &O_ValuesOut;		// Slot->Outputs->O_Value
	dio42_O_Outputs.Op_Mask		= &O_Mask;			// Slot->Outputs->O_Mask
	dio42_O_Inputs.Op_Value		= &O_ValuesIn;		// Slot->Inputs->O_Value
	return;
}

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//				<GENERIC>
// 				Eingabeparameter beim Treiberstart checken, einstellbar als 
//				ARG-Parameter sind:  	
//
//				command				==>	dio42_O_Mct.Op_Control->P_Control.value[]
//				output				==>	dio42_O_Outputs.Op_Value->P_Value.value
//				out_name_queue[x]	==>	DOs[x].name
//				in_name_queue[x]	==> DIs[x].name
//
//				Es ist zu diesem Zeitpunkt kein  Zugriffsschutz notwendig , 
//				da der Timer noch nicht aktiv ist!
//***********************************************************************************
void dio42_objects_generic(void) {	
	unsigned int _idx = 0;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	// HACK SHOW HACK
	// command = e_start;
	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{			// ARG: "command" übernehmen
		dio42_O_Mct.Op_Control->P_Control.value[0] = command; // Hinweis: Ein Externes Kommando wird auf [0] geschrieben,
		dio42_O_Mct.Op_Control->P_Control.value[1] = command;	// während die internen Stati in [1] stehen
	}
	/// *** Property OUTPUT laden 	
	output &=  SIZE_MAX_2bit;			// "0000 0011"  
	dio42_O_Outputs.Op_Value->P_ValueOut.value = output;
	/// *** Properties OUT_NAME_x laden
	for	(_idx= 0; _idx < DEV_MCT_PAA_DIO42_OUT_CHAN_MAX; _idx++){ // ARGS: "out_name_x" übernehmen, wenn Eintrag!
		if((strlen(out_name_queue[_idx]) > 0) && (strlen(out_name_queue[_idx]) <= PROPERTY_SIZE)){
			strcpy(DOs[_idx].name,out_name_queue[_idx]);
		}
	}
	/// *** Properties IN_NAME_x laden
	for	(_idx= 0; _idx < DEV_MCT_PAA_DIO42_IN_CHAN_MAX; _idx++){ // ARGS: "in_name_x" übernehmen, wenn Eintrag!
		if((strlen(in_name_queue[_idx]) > 0) && (strlen(in_name_queue[_idx]) <= PROPERTY_SIZE)){
			strcpy(DIs[_idx].name,in_name_queue[_idx]);
		}
	}
	return;
}

//***********************************************************************************
// Funktion:	Der Aufruf erfolgt automatisch, wenn der json-Parameter='{\"...\"}'
//				mit einem String aufgerufen wird. Als Format wird JSON-Syntax erwartet.
//				Folgende JSON-Elemente werden gesucht  und in die Werte in den 
//				entsprechenden Driver-Übergabeparametern abgelegt. 
//				<JSON>
//				oControl.Command (e_Start,e_Stop Driver)	==> command
//				{
//				oDevices[0].oValues[x].Name 				==> out_name_[0-3]
//				oDevices[0].oValues[x].Value				==> output
//				}
//				{
//				oDevices[1].oValues[x].Name 				==> in_name_[0-1]
//				}
//
//				Anschließend wird dio42_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t dio42_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	int 			_ch = 0;
	short			_bits = 0;
	short			_output = 0;	
	char  			_buf[JSON_LIMIT_SIZE+1];
	char 			_name[PROPERTY_SIZE+1];
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
	///------------------
	/// DEVICE OUTPUT 
	///------------------
	/// BIT-VALUES 
	for (_ch= 0; _ch < DEV_MCT_PAA_DIO42_OUT_CHAN_MAX; _ch++)	{
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_bits[1].nA.node = 0;		// Output-Device
		jsonkey_bits[3].nA.node = _ch;		// Outputs++
		Context_Build(&ctx, &jsonkey_bits[0], sizeof(jsonkey_bits));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: Integer, 3. Integer > 0
		if(!ctx.err.number && ctx.val.v.type == vType_int && ctx.val.vI.value){	
			_bits = ctx.val.vI.value;
			_output = _output | (_bits << _ch);
		}
	}
	output = _output;							/// ARGUMENT: output
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-0: output %d\n", _output);
	
	/// NAME
	for (_ch= 0; _ch < DEV_MCT_PAA_DIO42_OUT_CHAN_MAX; _ch++)	{
		 memset(_name,0, sizeof(_name)); // immer löschen
		// NAME
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_name[1].nA.node = 0;		// Output-Device
		jsonkey_name[3].nA.node = _ch;		// Outputs++
		Context_Build(&ctx, &jsonkey_name[0], sizeof(jsonkey_name));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))	{
			memcpy(&_name,&ctx.val.vS.value,strlen(ctx.val.vS.value));
			strcpy(out_name_queue[_ch] ,_name);	 /// ARGUMENT: out_name_queue[x] points to out_name[x]
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-0: name-%d %s\n",_ch, _name);
		}
	}
	///-------------
	/// DEVICE INPUT
	///-------------
	/// NAME
	for (_ch= 0; _ch < DEV_MCT_PAA_DIO42_IN_CHAN_MAX; _ch++)	{
		 memset(_name,0, sizeof(_name)); // immer löschen
		// NAME
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_name[1].nA.node = 1;		// Input-Device
		jsonkey_name[3].nA.node = _ch;		// Inputs++
		Context_Build(&ctx, &jsonkey_name[0], sizeof(jsonkey_name));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))	{
			memcpy(&_name,&ctx.val.vS.value,strlen(ctx.val.vS.value));
			strcpy(in_name_queue[_ch] ,_name);	/// ARGUMENT: in_name_queue[x] points to in_name[x]
			DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN-0: name-%d %s\n",_ch, _name);
		}
	}
	DEBUG(MCT_DEBUG_LEVEL3, "JSON: End\n");
	return len;
}
