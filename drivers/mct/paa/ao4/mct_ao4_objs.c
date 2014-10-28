/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../../mct_debug.h"
#include "../../mct_aio_uni.h"		// digital In/Output Defs
#include "mct_ao4_objs.h"			// objects

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

///*********************************************************
/// TEMPLATES
///*********************************************************
struct t_alg AOs[DEV_MCT_PAA_AO4_OUT_MAX] = {
		{"AO1 (1/C2)",SIZE_VALUE10,"0x01"},
		{"AO2 (2/C2)",SIZE_VALUE10,"0x02"},	
		{"AO3 (3/C2)",SIZE_VALUE10,"0x04"},	
		{"AO4 (4/C2)",SIZE_VALUE10,"0x08"}	
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char					ao4_json[JSON_LIMIT_SIZE];	// JSON formatierte Eingabe
STATIC int 				command;					// START/STOP
STATIC unsigned int 	output_0;					// 10bit ananloger Ausgabewert - Kanal 0 
STATIC unsigned int 	output_1;					// 10bit ananloger Ausgabewert - Kanal 1 
STATIC unsigned int 	output_2;					// 10bit ananloger Ausgabewert - Kanal 2
STATIC unsigned int 	output_3;					// 10bit ananloger Ausgabewert - Kanal 3
STATIC char 			out_name_0[PROPERTY_SIZE+1];
STATIC char 			out_name_1[PROPERTY_SIZE+1];
STATIC char 			out_name_2[PROPERTY_SIZE+1];
STATIC char 			out_name_3[PROPERTY_SIZE+1];

// QUEUE mit out-name Zeigern 
STATIC  char * const  out_name_queue[DEV_MCT_PAA_AO4_OUT_MAX] = 	\
{	out_name_0,out_name_1,out_name_2,out_name_3 };

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string	( json, ao4_json, sizeof(ao4_json), S_IRUGO);	
module_param		( command, int, S_IRUGO);
module_param		( output_0, uint, S_IRUGO);
module_param		( output_1, uint, S_IRUGO);
module_param		( output_2, uint, S_IRUGO);
module_param		( output_3, uint, S_IRUGO);
module_param_string( out_name_0, out_name_0, sizeof(out_name_0), S_IRUGO);
module_param_string( out_name_1, out_name_1, sizeof(out_name_1), S_IRUGO);
module_param_string( out_name_2, out_name_2, sizeof(out_name_2), S_IRUGO);
module_param_string( out_name_3, out_name_3, sizeof(out_name_3), S_IRUGO);

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
// Driver
struct 	tao4_drv 		ao4_O_Mct;
static obj_control 		O_Control;
static obj_physic		O_Physic;
// Output-Devices
struct 	tao4_dev_outp 	ao4_O_Outputs[DEV_MCT_PAA_AO4_OUT_MAX];
static obj_value_out 	O_ValuesOut[DEV_MCT_PAA_AO4_OUT_MAX];

///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
		{.n = {.type=nType_unused}}};

// oDevices[Outputs].oValues.Name 
STATIC u_node jsonkey_o_name[5] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
		{.nA = {.type=nType_array,.node=0}},	\
		{.nK = {.type=nType_string,.node= OBJECT_VALUE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_NAME_S}},	\
		{.n = {.type=nType_unused}}};

// oDevices[Outputs].oValue.Value 
STATIC u_node jsonkey_output[5] = {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
		{.nA = {.type=nType_array,.node=0}},	\
		{.nK = {.type=nType_string,.node= OBJECT_VALUE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_VALUE_S}},	\
		{.n = {.type=nType_unused}}};


///*********************************************************
///  OBJECT-FUNTIONS
///*********************************************************
void ao4_objects_create(void) {
	unsigned int _idx;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	/// DRIVER
	/// O_MCT: O_Control-Setup
	O_Control.pname = OBJECT_CONTROL_S;
	spin_lock_init(&O_Control.lock);
	O_Control.P_Control.pname[0] = PROPERTY_COMMAND_S;
	O_Control.P_Control.pname[1] = PROPERTY_STATE_S;
	O_Control.P_Control.value[0] = e_start; 			// external Command
	O_Control.P_Control.value[1] = e_stop;				// internal state 
	/// O_MCT: OPhysic-Setup
	O_Physic.pname				= OBJECT_PHYSIC_S;		// OPhysic	
	spin_lock_init(&O_Physic.lock);
	O_Physic.P_BUSLayer.pname	= PROPERTY_LAYER_BUS_S;	// BUS:
	O_Physic.P_INFO.pname		= PROPERTY_INFO_S;		// Info:
	strcpy(O_Physic.P_INFO.value, LABEL_SLOT_OFFLINE);	// Nachricht

	/// OUTPUT-DEVICES	
	// OValuesOut
	for(_idx= 0; _idx < DEV_MCT_PAA_AO4_OUT_MAX; _idx++)	{		
		O_ValuesOut[_idx].pname			= OBJECT_VALUE_S;
		spin_lock_init(&O_ValuesOut[_idx].lock);
		O_ValuesOut[_idx].P_Name.pname 	= PROPERTY_NAME_S;
		strcpy(O_ValuesOut[_idx].P_Name.value, AOs[_idx].name);	// "analog..."
		O_ValuesOut[_idx].P_Size.pname 	= PROPERTY_SIZE_S;
		O_ValuesOut[_idx].P_Size.value 	= AOs[_idx].size;		// bitgröße
		O_ValuesOut[_idx].P_Value.pname = PROPERTY_VALUE_S;
		O_ValuesOut[_idx].P_Value.value	= 0;
	}
	// Alle Slots von O_Outputs[] für alle Output-Geräte initialisieren!
	for(_idx= 0; _idx < DEV_MCT_PAA_AO4_OUT_MAX; _idx++)	{
		ao4_O_Outputs[_idx].Op_Value	= &O_ValuesOut[_idx];
	}

	/// ROOT Objekte einhängen!
	ao4_O_Mct.Op_Control 		= &O_Control;				// Slot: O_Control
	ao4_O_Mct.Op_Physic 		= &O_Physic;				// Slot: O_Physic
	for(_idx= 0; _idx < DEV_MCT_PAA_AO4_OUT_MAX; _idx++)	{
		ao4_O_Mct.Op_Outputs[_idx] = &ao4_O_Outputs[_idx];
	}
	return;
}

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//				<GENERIC>
// 				Eingabeparameter beim Treiberstart checken, einstellbar als 
//				ARG-Parameter sind:  	
//
//				command				==>	ao4_O_Mct.Op_Control->P_Control.value[]	
//				output[x]			==> ao4_O_Outputs[x].Op_Value->P_Value.value
//				out_name_queue[x]	==> ao4_O_Outputs[x].Op_Value->P_Name.value
//
//				Es ist zu diesem Zeitpunkt kein  Zugriffsschutz notwendig , 
//				da der Timer noch nicht aktiv ist!
//***********************************************************************************
void ao4_objects_generic(void) {	
	unsigned int _idx = 0;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	// HACK SHOW HACK
	// command = e_start;
	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{			// ARG: "command" übernehmen
		ao4_O_Mct.Op_Control->P_Control.value[0] = command; // Hinweis: Ein Externes Kommando wird auf [0] geschrieben,
		ao4_O_Mct.Op_Control->P_Control.value[1] = command;	// während die internen Stati in [1] stehen
	}
	/// *** Properties OUT_NAME_x laden, wenn Eintrag
	for	(_idx= 0; _idx < DEV_MCT_PAA_AO4_OUT_MAX; _idx++){
		// ARGS: "out_name_x" übernehmen, wenn Eintrag!
		if((strlen(out_name_queue[_idx]) > 0) && (strlen(out_name_queue[_idx]) <= PROPERTY_SIZE)){
			strcpy(ao4_O_Outputs[_idx].Op_Value->P_Name.value,out_name_queue[_idx]);
		}
	}
	/// *** Properties OUTPUT_x laden 	
	ao4_O_Outputs[0].Op_Value->P_Value.value = output_0 & SIZE_MAX_10bit;
	ao4_O_Outputs[1].Op_Value->P_Value.value = output_1 & SIZE_MAX_10bit;
	ao4_O_Outputs[2].Op_Value->P_Value.value = output_2 & SIZE_MAX_10bit;
	ao4_O_Outputs[3].Op_Value->P_Value.value = output_3 & SIZE_MAX_10bit;
	return;
}

//***********************************************************************************
// Funktion:	Der Aufruf erfolgt automatisch, wenn der json-Parameter='{\"...\"}'
//				mit einem String aufgerufen wird. Als Format wird JSON-Syntax erwartet.
//				Folgende JSON-Elemente werden gesucht  und in die Werte in den 
//				entsprechenden Driver-Übergabeparametern abgelegt. 
// 				<JSON>
//				oControl.Command (e_Start,e_Stop Driver)	==> command
//				{
//				oDevices[0-3].oValue.Name 					==> out_name_queue[0-3]
//				oDevices[0-3].oValue.Value					==> output_[0-3]
//				}
//				Anschließend wird ao4_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t ao4_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	int 			_ch = 0;
	char  			_buf[JSON_LIMIT_SIZE+1];
	char 			_name[PROPERTY_SIZE+1];
	u16				_out_val;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	if(len == 0 || len > JSON_LIMIT_SIZE+1)	{
		DEBUG(MCT_DEBUG_LEVEL3, "%s %s(): %s\n", __FILE__,__FUNCTION__, "Parameter json with corrupt limit size!");
		return -1;
	}
	
	///------------------
	/// DRIVER
	///------------------
	DEBUG(MCT_DEBUG_LEVEL3, "JSON: Start\n");
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
	/// OUTPUT DEVICES 
	///------------------
	for (_ch= 0; _ch < DEV_MCT_PAA_AO4_OUT_MAX; _ch++)	{
		memset(_name,0, sizeof(_name)); 
		_out_val = 0;
		/// VALUE
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_output[1].nA.node = _ch;	// Offset beachten!
		Context_Build(&ctx, &jsonkey_output[0], sizeof(jsonkey_output));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: Integer , 3.Integer > 0 
		if(!ctx.err.number && ctx.val.v.type == vType_int && ctx.val.vI.value)
		_out_val = ctx.val.vI.value;
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-%d: value %d\n",_ch, _out_val);
		/// NAME
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_o_name[1].nA.node = _ch;
		Context_Build(&ctx, &jsonkey_o_name[0], sizeof(jsonkey_o_name));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))	{
			memcpy(&_name,&ctx.val.vS.value,strlen(ctx.val.vS.value));
		}
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-%d: name %s\n",_ch, _name);
		
		/// Mappe alle Werte auf die Argument-Variablen, diese werden dann ausgewertet
		/// out_name_queue_[0...3]
		/// output_[0..3]
		switch (_ch) {
			case DEV_MCT_PAA_AO4_OUT_0:
				memcpy(out_name_queue[_ch],&_name,strlen(_name)+1);	
				output_0 = _out_val;
				break;
			case DEV_MCT_PAA_AO4_OUT_1:
				memcpy(out_name_queue[_ch],&_name,strlen(_name)+1);	
				output_1 = _out_val;
				break;
			case DEV_MCT_PAA_AO4_OUT_2:
				memcpy(out_name_queue[_ch],&_name,strlen(_name)+1);	
				output_2 = _out_val;
				break;
			case DEV_MCT_PAA_AO4_OUT_3:
				memcpy(out_name_queue[_ch],&_name,strlen(_name)+1);	
				output_3 = _out_val;
				break;
			default:
				DEBUG(MCT_DEBUG_LEVEL3, "%s %s(): %s %d\n", __FILE__, __FUNCTION__, "Attention unsupported Device_OUT number!", _ch);
				break;
			}
	}
	DEBUG(MCT_DEBUG_LEVEL3, "JSON: End\n");
	return len;
}
