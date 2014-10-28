/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011

 	Description:

 *******************************************************************************/
#include <linux/module.h>
#include <linux/string.h>	
#include "../mct_debug.h"
#include "../mct_dio_uni.h"			// digital In/Output Defs
#include "mct_spi_dio_objs.h"		// objects
#include "mct_dio.h"

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

///*********************************************************
/// TEMPLATES
///*********************************************************
struct t_dig DOs[OUTPUT_ELEMENTS] = {
		{ "K1 (11-12-14) ",1},
		{ "K2 (21-22-24) ",1},	
		{ "K3 (31-32-34) ",1},	
		{ "K4 (41-42-34) ",1}
};
struct t_dig DIs[INPUT_ELEMENTS] = {
		{ "DI1 (S01/1-)   ",1,},
		{ "DI2 (S02/2-)   ",1,},	
		{ "DI3 (S03/3-)   ",1,},	
		{ "DI4 (S04/4-)   ",1,},	
		{ "DI5 (5+/5-)    ",1,},	
		{ "DI6 (6+/6-)    ",1,},	
		{ "DI7 (7+/7-)    ",1,},	
		{ "DI8 (8+/8-)    ",1,}	
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char			dio_json[JSON_LIMIT_SIZE];
STATIC unchar	command;

// DEVICE-ARGs
STATIC unchar	output;			// Ausgänge-Relais 

STATIC char 	out_name_0[PROPERTY_SIZE+1];
STATIC char 	out_name_1[PROPERTY_SIZE+1];
STATIC char 	out_name_2[PROPERTY_SIZE+1];
STATIC char 	out_name_3[PROPERTY_SIZE+1];

STATIC char 	in_name_0[PROPERTY_SIZE+1];
STATIC char 	in_name_1[PROPERTY_SIZE+1];
STATIC char 	in_name_2[PROPERTY_SIZE+1];
STATIC char 	in_name_3[PROPERTY_SIZE+1];
STATIC char 	in_name_4[PROPERTY_SIZE+1];
STATIC char 	in_name_5[PROPERTY_SIZE+1];
STATIC char 	in_name_6[PROPERTY_SIZE+1];
STATIC char 	in_name_7[PROPERTY_SIZE+1];

// QUEUE mit out-name Zeigern 
STATIC char * const  out_name_queue[OUTPUT_ELEMENTS] =	\
{	out_name_0,out_name_1,out_name_2,out_name_3};

// QUEUE mit in-name Zeigern 
STATIC  char * const  in_name_queue[INPUT_ELEMENTS] = 	\
{	in_name_0,in_name_1,in_name_2,in_name_3,	\
	in_name_4,in_name_5,in_name_6,in_name_7};

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string( json, dio_json, sizeof(dio_json), S_IRUGO);// JSON formatierte Eingabe
module_param( command, byte, S_IRUGO);	// START/STOP
module_param( output, byte, S_IRUGO);	// Ausgänge RELAIS

module_param_string( out_name_0, out_name_0, sizeof(out_name_0), S_IRUGO);
module_param_string( out_name_1, out_name_1, sizeof(out_name_1), S_IRUGO);
module_param_string( out_name_2, out_name_2, sizeof(out_name_2), S_IRUGO);
module_param_string( out_name_3, out_name_3, sizeof(out_name_3), S_IRUGO);

module_param_string( in_name_0, in_name_0, sizeof(in_name_0), S_IRUGO);
module_param_string( in_name_1, in_name_1, sizeof(in_name_1), S_IRUGO);
module_param_string( in_name_2, in_name_2, sizeof(in_name_2), S_IRUGO);
module_param_string( in_name_3, in_name_3, sizeof(in_name_3), S_IRUGO);
module_param_string( in_name_4, in_name_4, sizeof(in_name_4), S_IRUGO);
module_param_string( in_name_5, in_name_5, sizeof(in_name_5), S_IRUGO);
module_param_string( in_name_6, in_name_6, sizeof(in_name_6), S_IRUGO);
module_param_string( in_name_7, in_name_7, sizeof(in_name_7), S_IRUGO);

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
// OControl
static obj_control 	O_Control = {
	.pname = OBJECT_CONTROL_S,
	.lock = SPIN_LOCK_UNLOCKED,
	.P_Control	= {
		.pname 	= {PROPERTY_COMMAND_S,PROPERTY_STATE_S},
		.value 	= {e_stop,e_idle},
	}
};

// OPhysic
static obj_physic O_Physic = {
	.pname	 	= OBJECT_PHYSIC_S,
	.P_Layer0 	= {
		.pname	= PROPERTY_LAYER_SPI_S,
	}
};

/// Device Inputs 
static obj_value O_ValuesIn = {
	.pname	= OBJECT_VALUE_S,
	.lock 	= SPIN_LOCK_UNLOCKED,
	.P_Name	= {
		.pname = PROPERTY_NAME_S,
	},
	.P_Size	= {
		.pname	= PROPERTY_SIZE_S,
		.value	= (unsigned char) DEVICE_IN_bits,
	},
	.P_Value = {
		.pname	= PROPERTY_VALUE_S,
		.value	= 0,
	}
};

struct 	tdio_dev_inp dio_O_Inputs = {
	.Op_Value 	= &O_ValuesIn,	
};

 /// Device Outputs
static obj_init O_Init = {
	.pname	= OBJECT_INIT_Ss,
	.lock 	= SPIN_LOCK_UNLOCKED,
	.P_Value	= {
		.pname	= PROPERTY_VALUE_S,
		.value	= 0,
	},
	.P_Size	= {
		.pname	= PROPERTY_SIZE_S,
		.value	= SIZE_VALUE4,			// 4bit
	},
	.P_Link = {
		.pname = PROPERTY_LINK_S,
		.value	= 0,
	}
};

static obj_value O_ValuesOut = {
	.pname	= OBJECT_VALUE_S,
	.lock	= SPIN_LOCK_UNLOCKED,
	.P_Name	= {
		.pname	= PROPERTY_NAME_S,
	},
	.P_Size	= {
		.pname	= PROPERTY_SIZE_S,
		.value	= (unsigned char) DEVICE_OUT_bits,
	},
	.P_Value = {
		.pname	= PROPERTY_VALUE_S,
		.value	= 0,
	}
};

struct 	tdio_dev_outp dio_O_Outputs = {
	.Op_Value = &O_ValuesOut, 
	.Op_Init = &O_Init,	
};


/// Driver
struct tdio_drv dio_O_Mct = {
	.Op_Control = &O_Control,
	.Op_Physic	= &O_Physic,
	.Op_Inputs	= &dio_O_Inputs,
	.Op_Outputs	= &dio_O_Outputs,
};


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
void dio_objects_create(void) {
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	return;
}

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//				<GENERIC>
// 				Eingabeparameter beim Treiberstart checken, einstellbar als 
//				ARG-Parameter sind:  	
//
//				command					==>	dio_O_Mct.Op_Control->P_Control.value[]
//				output					==>	dio_O_Outputs.Op_Value->P_Value.value
//				out_name[x]				==>	DOs[_idx].name
//				in_name[x]				==> DIs[_idx].name
//
//				Es ist zu diesem Zeitpunkt kein  Zugriffsschutz notwendig , 
//				da der Timer noch nicht aktiv ist!
//				
//***********************************************************************************
void dio_objects_generic(void) {	
	unsigned int _idx = 0;
	//	char _buffer[300];
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);

	// HACK SHOW HACK
	// command = e_start;
	
	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{		// ARG: "command" übernehmen
		dio_O_Mct.Op_Control->P_Control.value[0] = command; // Hinweis: Ein Externes Kommando wird auf [0] geschrieben,
		dio_O_Mct.Op_Control->P_Control.value[1] = command;	// während die internen Stati in [1] stehen
	}													

	/// *** Property OUTPUT laden 	
	dio_O_Outputs.Op_Value->P_Value.value = (output & SIZE_MAX_DEZ_4bit);
	
	/// *** Properties OUT_NAME_x laden
	for	(_idx= 0; _idx < OUTPUT_ELEMENTS; _idx++){ // ARGS: "out_name_x" übernehmen, wenn Eintrag!
		if((strlen(out_name_queue[_idx]) > 0) && (strlen(out_name_queue[_idx]) <= PROPERTY_SIZE)){
			strcpy(DOs[_idx].name,out_name_queue[_idx]);
		}
	}
	/// *** Properties IN_NAME_x laden
	for	(_idx= 0; _idx < INPUT_ELEMENTS; _idx++){ // ARGS: "in_name_x" übernehmen, wenn Eintrag!
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
// 				<JSON>
//				oControl.Command (e_Start,e_Stop Driver)	==> command
//				{
//				oDevices[0].oValues[x].Name 				==> out_name_x
//				oDevices[0].oValues[x].Value				==> output 
//				}
//				{
//				oDevices[1].oValues[x].Name 				==> in_name_x
//				}
//				Anschließend wird dio_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t dio_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	int 			_ch = 0;
	int				_bits = 0;
	unsigned long	_output = 0;
	char  			_buf[JSON_LIMIT_SIZE+1]; 
	char 			_name[PROPERTY_SIZE+1];
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
	/// DEVICE OUTPUT 
	///------------------
	/// BIT-VALUES 
	for (_ch= 0; _ch < OUTPUT_ELEMENTS; _ch++)	{
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
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
	output = _output;							/// ARGUMENT: output (packed)	
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-0: output %ld\n", _output);
	
	/// NAME
	for (_ch= 0; _ch < OUTPUT_ELEMENTS; _ch++)	{
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
	for (_ch= 0; _ch < INPUT_ELEMENTS; _ch++)	{
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
