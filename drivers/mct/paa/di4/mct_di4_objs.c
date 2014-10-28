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
#include "mct_di4_objs.h"			// objects

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

///*********************************************************
/// TEMPLATES
///*********************************************************
struct t_dig DIs[DEV_MCT_PAA_DI4_IN_CHAN_MAX] = {
		{"DI1 (1+/1-)",SIZE_VALUE1,"0x01"},
		{"DI2 (2+/2-)",SIZE_VALUE1,"0x02"},	
		{"DI3 (3+/3-)",SIZE_VALUE1,"0x04"},	
		{"DI4 (4+/4-)",SIZE_VALUE1,"0x08"}	
};

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char			di4_json[JSON_LIMIT_SIZE];	// JSON formatierte Eingabe
STATIC int 		command;					// START/STOP
STATIC char 	in_name_0[PROPERTY_SIZE+1];
STATIC char 	in_name_1[PROPERTY_SIZE+1];
STATIC char 	in_name_2[PROPERTY_SIZE+1];
STATIC char 	in_name_3[PROPERTY_SIZE+1];

// QUEUE mit in-name Zeigern 
STATIC  char * const  in_name_queue[DEV_MCT_PAA_DI4_IN_CHAN_MAX] = 	\
{	in_name_0,in_name_1,in_name_2,in_name_3 };


///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
// OControl
static obj_control 	O_Control = {
	.pname = OBJECT_CONTROL_S,
	.lock = SPIN_LOCK_UNLOCKED,
	.P_Control = {
		.pname = {PROPERTY_COMMAND_S,PROPERTY_STATE_S},
		.value = {e_stop,e_stop},
	},
};

// OPhysik
static obj_physic O_Physic = {
	.pname 	= OBJECT_PHYSIC_S,
	.lock = SPIN_LOCK_UNLOCKED,
	.P_BUSLayer = {
		.pname = PROPERTY_LAYER_BUS_S,
	},
	.P_INFO = {
		.pname = PROPERTY_INFO_S,
		.value = LABEL_SLOT_OFFLINE,
	},
};

// OValues
static obj_value O_ValuesIn =	{
 	.pname	= OBJECT_VALUE_Ss,
	.lock = SPIN_LOCK_UNLOCKED,	
	.P_Name = {
		.pname 	= PROPERTY_NAME_S,
//		.value	=  hier nicht verwendet!
	},
	.P_Size = {	// Verarbeitungsbreite
		.pname 	= PROPERTY_SIZE_S,
		.value	= (unsigned char) DEV_MCT_PAA_DI4_IN_CHAN_MAX,
	},
	.P_Value = {
		.pname = PROPERTY_VALUE_S,
		.value = 0,
	},
};

/// Devive
struct tdi4_dev_inp di4_O_Inputs = {
	.Op_Value	= &O_ValuesIn,
}; 

/// Driver
struct 	tdi4_drv di4_O_Mct = {
	.Op_Control = &O_Control,
	.Op_Physic = &O_Physic,
	.Op_Inputs = &di4_O_Inputs,
};


///*********************************************************
///  TEMPLATE's
///*********************************************************
extern struct t_dig	DIs[DEV_MCT_PAA_DI4_IN_CHAN_MAX];

///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
		{.n = {.type=nType_unused}}};
// oDevices[0].oValues[Inputs].Name 
STATIC u_node jsonkey_io_name[6] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_Ss}},	\
		{.nA = {.type=nType_array,.node=0}},	\
		{.nK = {.type=nType_string,.node= OBJECT_VALUE_Ss}},	\
		{.nA = {.type=nType_array,.node=0}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_NAME_S}},	\
		{.n = {.type=nType_unused}}};

///*********************************************************
///  OBJECT-FUNTIONS
///*********************************************************

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//				<GENERIC>
// 				Eingabeparameter beim Treiberstart checken, einstellbar als 
//				ARG-Parameter sind:  	
//
//				command				==>	di4_O_Mct.Op_Control->P_Control.value[]
//				in_name_queue[x]	==>	DIs[x].name
//
//				Es ist zu diesem Zeitpunkt kein  Zugriffsschutz notwendig , 
//				da der Timer noch nicht aktiv ist!
//***********************************************************************************
void di4_objects_generic(void) {	
	unsigned int _idx = 0;
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	// HACK SHOW HACK
	// command = e_start;
	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{		// ARG: "command" übernehmen
		di4_O_Mct.Op_Control->P_Control.value[0] = command; // Hinweis: Ein Externes Kommando wird auf [0] geschrieben,
		di4_O_Mct.Op_Control->P_Control.value[1] = command;	// während die internen Stati in [1] stehen
	}
	/// *** Properties IN_NAME_x laden
	for	(_idx= 0; _idx < DEV_MCT_PAA_DI4_IN_CHAN_MAX; _idx++){ // ARGS: "in_name_x" übernehmen, wenn Eintrag!
		if((strlen(in_name_queue[_idx]) > 0) && (strlen(in_name_queue[_idx]) <= PROPERTY_SIZE)){
			strcpy(DIs[_idx].name,in_name_queue[_idx]);
		}
	}
	return;
}

//***********************************************************************************
// Funktion:	Der Aufruf erfolgt automatisch, wenn der json-Parameter="{...}"
//				mit einem String aufgerufen wird. Als Format wird JSON-Syntax erwartet.
//				Folgende JSON-Elemente werden gesucht  und in die Werte in den 
//				entsprechenden Übergabeparametern abgelegt. 
//				<JSON>
//				oControl.Command (e_Start,e_Stop Driver)	==> command
//				{
//				oDevices[0].oValues[0-3].Name 				==> in_name_queue_[0-3]
//				}
//
//				Anschließend wird di4_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t di4_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	int 			_ch = 0;	
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
	/// DEVICE INPUT 
	///------------------
	/// NAME
	for (_ch= 0; _ch < DEV_MCT_PAA_DI4_IN_CHAN_MAX; _ch++)	{
		memset(_name,0, sizeof(_name)); 
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_io_name[1].nA.node = 0;
		jsonkey_io_name[3].nA.node = _ch;
		Context_Build(&ctx, &jsonkey_io_name[0], sizeof(jsonkey_io_name));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE)) {
			memcpy(&_name,&ctx.val.vS.value,strlen(ctx.val.vS.value));
			/// ARGUMENT: in_name_queue[x]
			memcpy(in_name_queue[_ch],&_name,strlen(_name)+1);
		}
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN-%d: name %s\n",_ch, _name);
	}
	DEBUG(MCT_DEBUG_LEVEL3, "JSON: End\n");
	return len;
}

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string	( json, di4_json, sizeof(di4_json), S_IRUGO);	
module_param		( command, int, S_IRUGO);
module_param_string( in_name_0, in_name_0, sizeof(in_name_0), S_IRUGO);
module_param_string( in_name_1, in_name_1, sizeof(in_name_1), S_IRUGO);
module_param_string( in_name_2, in_name_2, sizeof(in_name_2), S_IRUGO);
module_param_string( in_name_3, in_name_3, sizeof(in_name_3), S_IRUGO);
