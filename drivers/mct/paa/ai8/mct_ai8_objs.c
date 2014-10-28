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
#include "../../mct_aio_uni.h"		// digital In/Output Defs
#include "mct_ai8_objs.h"			// objects

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

// check length of propertie's   zero-string("") if 
#define PROPERTY_CHECK_VALUE_LEN(x,y)  	if (strlen(x) > PROPERTY_SIZE)	{\
												(y)[0] = 0;	}	\
										else	{	\
											strncpy(y,x,PROPERTY_SIZE+1);	\
											(y)[PROPERTY_SIZE] = 0;\
										}

// mögliche Messbereiche für die ananlogen Eingänge
// inactiv, 0-10V, 40Ohm-4MOhm
static char * const ai_modes_tab[] 	= {
			AI_PROP_MODE_0_S, AI_PROP_MODE_1_S,"DUMMY", AI_PROP_MODE_3_S,NULL};

const char * ai_label_tab[] 	= {
			AI_LAB_MODE_0_S, AI_LAB_MODE_1_S, "DUMMY",AI_LAB_MODE_3_S,NULL};

///*********************************************************
/// TEMPLATES
///*********************************************************
struct t_alg AIs[DEV_MCT_PAA_AI8_IN_MAX] = {
		{"AI1 (1/C2)",SIZE_VALUE32,""},
		{"AI2 (2/C2)",SIZE_VALUE32,""},	
		{"AI3 (3/C2)",SIZE_VALUE32,""},	
		{"AI4 (4/C2)",SIZE_VALUE32,""},
		{"AI5 (5/C2)",SIZE_VALUE32,""},
		{"AI6 (6/C2)",SIZE_VALUE32,""},	
		{"AI7 (7/C2)",SIZE_VALUE32,""},	
		{"AI8 (8/C2)",SIZE_VALUE32,""}
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char			ai8_json[JSON_LIMIT_SIZE];	// JSON formatierte Eingabe
STATIC int 		command;					// START/STOP
STATIC char 	in_name_0[PROPERTY_SIZE+1];
STATIC char 	in_name_1[PROPERTY_SIZE+1];
STATIC char 	in_name_2[PROPERTY_SIZE+1];
STATIC char 	in_name_3[PROPERTY_SIZE+1];
STATIC char 	in_name_4[PROPERTY_SIZE+1];
STATIC char 	in_name_5[PROPERTY_SIZE+1];
STATIC char 	in_name_6[PROPERTY_SIZE+1];
STATIC char 	in_name_7[PROPERTY_SIZE+1];
STATIC char		in_mode_0[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 0 
STATIC char 	in_mode_1[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 1 
STATIC char		in_mode_2[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 2
STATIC char		in_mode_3[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 3
STATIC char		in_mode_4[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 4 
STATIC char 	in_mode_5[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 5 
STATIC char		in_mode_6[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 6
STATIC char		in_mode_7[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;	// Messbereich Input - Kanal 7

// QUEUE mit in-name Zeigern 
STATIC  char * const  in_name_queue[DEV_MCT_PAA_AI8_IN_MAX] = 	\
{	in_name_0,in_name_1,in_name_2,in_name_3,\
	in_name_4,in_name_5,in_name_6,in_name_7};

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string	( json, ai8_json, sizeof(ai8_json), S_IRUGO);	
module_param		( command, int, S_IRUGO);

module_param_string( in_name_0, in_name_0, sizeof(in_name_0), S_IRUGO);
module_param_string( in_name_1, in_name_1, sizeof(in_name_1), S_IRUGO);
module_param_string( in_name_2, in_name_2, sizeof(in_name_2), S_IRUGO);
module_param_string( in_name_3, in_name_3, sizeof(in_name_3), S_IRUGO);
module_param_string( in_name_4, in_name_4, sizeof(in_name_4), S_IRUGO);
module_param_string( in_name_5, in_name_5, sizeof(in_name_5), S_IRUGO);
module_param_string( in_name_6, in_name_6, sizeof(in_name_6), S_IRUGO);
module_param_string( in_name_7, in_name_7, sizeof(in_name_7), S_IRUGO);

module_param_string( in_mode_0, in_mode_0, sizeof(in_mode_0), S_IRUGO);
module_param_string( in_mode_1, in_mode_1, sizeof(in_mode_1), S_IRUGO);
module_param_string( in_mode_2, in_mode_2, sizeof(in_mode_2), S_IRUGO);
module_param_string( in_mode_3, in_mode_3, sizeof(in_mode_3), S_IRUGO);
module_param_string( in_mode_4, in_mode_4, sizeof(in_mode_4), S_IRUGO);
module_param_string( in_mode_5, in_mode_5, sizeof(in_mode_5), S_IRUGO);
module_param_string( in_mode_6, in_mode_6, sizeof(in_mode_6), S_IRUGO);
module_param_string( in_mode_7, in_mode_7, sizeof(in_mode_7), S_IRUGO);

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
// Driver
struct 	tai8_drv 		ai8_O_Mct;
static obj_control 		O_Control;
static obj_physic		O_Physic;
// Output-Devices
struct 	tai8_dev_inp 	ai8_O_Inputs[DEV_MCT_PAA_AI8_IN_MAX];
static obj_value_in 	O_ValuesIn[DEV_MCT_PAA_AI8_IN_MAX];			//32bit (Float)
static obj_type 		O_Types[DEV_MCT_PAA_AI8_IN_MAX];			//Modus


///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
		{.n = {.type=nType_unused}}};

// oDevices[Inputs].oValues.Name 
STATIC u_node jsonkey_in_name[5] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
		{.nA = {.type=nType_array,.node=0}},	\
		{.nK = {.type=nType_string,.node= OBJECT_VALUE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_NAME_S}},	\
		{.n = {.type=nType_unused}}};

// oDevices[Inputs].Mode 
STATIC u_node jsonkey_in_mode[4] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
		{.nA = {.type=nType_array,.node=0}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_MODE_S}},	\
		{.n = {.type=nType_unused}}};


///*********************************************************
///  OBJECT-FUNTIONS
///*********************************************************
void ai8_objects_create(void) {
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
	/// INPUT-DEVICES	
	/// OValuesIn
	for(_idx= 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{		
		O_ValuesIn[_idx].pname			= OBJECT_VALUE_S;
		spin_lock_init(&O_ValuesIn[_idx].lock);
		O_ValuesIn[_idx].P_Name.pname 	= PROPERTY_NAME_S;
		strcpy(O_ValuesIn[_idx].P_Name.value, AIs[_idx].name);	// "analog..."
		O_ValuesIn[_idx].P_Size.pname 	= PROPERTY_SIZE_S;
		O_ValuesIn[_idx].P_Size.value 	= AIs[_idx].size;		// bitgröße
		O_ValuesIn[_idx].P_Value.pname = PROPERTY_VALUE_S;
		O_ValuesIn[_idx].P_Value.value	= 0;
	}
	/// OTypes
	for(_idx= 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{
		O_Types[_idx].pname				= OBJECT_TYPE_S;
		O_Types[_idx].P_Type.pname 		= PROPERTY_MODE_S;
		O_Types[_idx].P_Type.setup 		= &box_setup;
		O_Types[_idx].P_Type.select 	= &box_select;
		O_Types[_idx].P_Type.sprintf_sel= &box_sprintf_sel,
		O_Types[_idx].P_Type.get_id		= &box_get_id;
	}
	// Alle Slots von O_Inputs[] für alle Input-Geräte initialisieren!
	for(_idx= 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{
		ai8_O_Inputs[_idx].Op_Value	= &O_ValuesIn[_idx];
		ai8_O_Inputs[_idx].Op_Type	= &O_Types[_idx];	
	}

	/// ROOT Objekte einhängen!
	ai8_O_Mct.Op_Control 		= &O_Control;				// Slot: O_Control
	ai8_O_Mct.Op_Physic 		= &O_Physic;				// Slot: O_Physic
	for(_idx= 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{
		ai8_O_Mct.Op_Inputs[_idx] = &ai8_O_Inputs[_idx];
	}
	return;
}

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//				<GENERIC>
// 				Eingabeparameter beim Treiberstart checken, einstellbar als 
//				ARG-Parameter sind:	
//
//				command				==>	ai8_O_Mct.Op_Control->P_Control.value[]
//				in_name_queue[x]	==>	ai8_O_Inputs[x].Op_Value->P_Name.value
//				in_mode_[x]			==> ai8_O_Inputs[x].Op_Type->P_Type
//
//				Es ist zu diesem Zeitpunkt kein  Zugriffsschutz notwendig , 
//				da der Timer noch nicht aktiv ist!
//***********************************************************************************
void ai8_objects_generic(void) {	
	unsigned int _idx = 0;
	char _mode[PROPERTY_SIZE+1] = {0};
//	char _buffer[300];
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	// HACK SHOW HACK
	// command = e_start;
	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{			// ARG: "command" übernehmen
		ai8_O_Mct.Op_Control->P_Control.value[0] = command; // Hinweis: Ein Externes Kommando wird auf [0] geschrieben,
		ai8_O_Mct.Op_Control->P_Control.value[1] = command;	// während die internen Stati in [1] stehen
	}
	/// *** Properties IN_NAME_x laden, wenn Eintrag
	for	(_idx= 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++){ 
		// ARGS: "in_name_x" übernehmen, wenn Eintrag!
		if((strlen(in_name_queue[_idx]) > 0) && (strlen(in_name_queue[_idx]) <= PROPERTY_SIZE)){
			strcpy(ai8_O_Inputs[_idx].Op_Value->P_Name.value,in_name_queue[_idx]);
		}
	}
	/// *** Properties TYPE_x initialisieren
	for	(_idx= 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{
		box_setup((&ai8_O_Inputs[_idx].Op_Type->P_Type), 0, ai_modes_tab);
		switch (_idx) {
			case 0: PROPERTY_CHECK_VALUE_LEN(in_mode_0,_mode); break;
			case 1: PROPERTY_CHECK_VALUE_LEN(in_mode_1,_mode); break;
			case 2: PROPERTY_CHECK_VALUE_LEN(in_mode_2,_mode); break;
			case 3: PROPERTY_CHECK_VALUE_LEN(in_mode_3,_mode); break;
			case 4: PROPERTY_CHECK_VALUE_LEN(in_mode_4,_mode); break;
			case 5: PROPERTY_CHECK_VALUE_LEN(in_mode_5,_mode); break;
			case 6: PROPERTY_CHECK_VALUE_LEN(in_mode_6,_mode); break;
			case 7: PROPERTY_CHECK_VALUE_LEN(in_mode_7,_mode); break;
		} 
		// Wenn der Typ unbekannt ist, wird der Input auf inaktiv eingestellt
		if(box_select((&ai8_O_Inputs[_idx].Op_Type->P_Type), _mode) == false)
			box_select((&ai8_O_Inputs[_idx].Op_Type->P_Type), AI_PROP_MODE_0_S);
	}
	
	// print adjust's
	/*
	printk("EINSTELLUNGEN Start\n");	
	printk("---< Driver >---\n");
	printk("%s: %d\n",  ai8_O_Mct.Op_Control->P_Control.pname[0], ai8_O_Mct.Op_Control->P_Control.value[0]); 
	for (_idx = 0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{ 
		printk("---< In-Device %d >---\n",_idx);
		box_sprintf_sel((&ai8_O_Inputs[_idx].Op_Type->P_Type), _buffer);
		printk("%s\n",_buffer);
	}
	printk("EINSTELLUNGEN Ende\n");
	*/
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
//				oDevices[0-7].oValue.Name 					==> in_name_[0-7]
//				oDevices[0-7].Mode							==> in_type_[0-7]
//				}
//				Anschließend wird ai8_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t ai8_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	int 			_ch = 0;
	char  			_buf[JSON_LIMIT_SIZE+1];
	char 			_mode[PROPERTY_SIZE+1];
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
	/// DEVICES INPUT 
	///------------------
	for (_ch= 0; _ch < DEV_MCT_PAA_AI8_IN_MAX; _ch++)	{
		memset(_name,0, sizeof(_name)); 
		memset(_mode,0, sizeof(_mode));
		/// NAME
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_in_name[1].nA.node = _ch;
		Context_Build(&ctx, &jsonkey_in_name[0], sizeof(jsonkey_in_name));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))
			memcpy(&_name,&ctx.val.vS.value,strlen(ctx.val.vS.value));		 
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN-%d: name %s\n",_ch, _name);
		/// MODE
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_in_mode[1].nA.node = _ch;
		Context_Build(&ctx, &jsonkey_in_mode[0], sizeof(jsonkey_in_mode));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))
			memcpy(&_mode,&ctx.val.vS.value,strlen(ctx.val.vS.value));		 		
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN-%d: mode %s\n",_ch, _mode);

		/// Mappe alle Werte auf die Argument-Variablen, diese werden dann ausgewertet 
		/// in_name_queue_[0...7]
		/// in_mode_[0...7]
		switch (_ch) {
			case DEV_MCT_PAA_AI8_IN_0:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_0,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_1:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_1,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_2:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_2,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_3:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_3,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_4:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_4,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_5:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_5,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_6:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_6,&_mode,strlen(_mode)+1);	
				break;
			case DEV_MCT_PAA_AI8_IN_7:
				memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);	
				memcpy(&in_mode_7,&_mode,strlen(_mode)+1);	
				break;
			default:
				DEBUG(MCT_DEBUG_LEVEL3, "%s %s(): %s %d\n", __FILE__, __FUNCTION__, "Attention unsupported Device_IN number!", _ch);
				break;
		}
	}
	DEBUG(MCT_DEBUG_LEVEL3, "JSON: End\n");
	return len;
}
