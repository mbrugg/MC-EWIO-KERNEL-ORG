/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011
 	
 	Description:

 *******************************************************************************/
#include <linux/module.h>
#include <linux/string.h>	

#include "mct_pin_di_objs.h"		// objects
#include "../mct_debug.h"
#include "mct_di.h"

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char			di_json[JSON_LIMIT_SIZE];		// JSON formatierte Eingabe
STATIC unchar	command;

// DEVICE-ARGs
STATIC char		mode[PROPERTY_SIZE+1];
STATIC char		name[PROPERTY_SIZE+1];
STATIC char		input[STRING_LLD_SIZE+1];
STATIC char		freeze[STRING_LLD_SIZE+1];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
// OControl
static obj_control 	O_Control = {
	.pname = OBJECT_CONTROL_S,
	.lock = SPIN_LOCK_UNLOCKED,
	.P_Control = {
		.pname = {PROPERTY_COMMAND_S,PROPERTY_STATE_S},
		.value = {e_stop,e_idle},
	},
};

// OPhysik
static obj_physic O_Physic = {
	.pname 	= OBJECT_PHYSIC_S,
	.lock = SPIN_LOCK_UNLOCKED,
	.P_Pin = {
		.pname = PROPERTY_PIN_S,
//		.value = di_pin,
	},
	.P_Tri = {
		.pname = PROPERTY_TRI_S,
		.value = TRI_PULL_UP,
	},
};

// OMode
static obj_mode O_Mode = {
	.pname	= OBJECT_MODE_S,
	.P_Name = {
		.pname = PROPERTY_MODE_S,
		.value = PROPERTY_MODE_DIRECT_S,
	},
	.P_Value = {
		.pname = PROPERTY_VALUE_S,
		.value = MODE_DIRECT,
	},
};

// OValue
static obj_value O_Value =	{
 	.pname	= OBJECT_VALUE_S,
	.lock = SPIN_LOCK_UNLOCKED,	
	.P_Name = {
		.pname 	= PROPERTY_NAME_S,
//		.value	= di_pin_name,
	},
	.P_Size = {
		.pname 	= PROPERTY_SIZE_S,
		.value	= SIZE_VALUE1,
	},
	.P_Value = {
		.pname = PROPERTY_VALUE_S,
		.value = 0,
	},
};

// OFreeze
static obj_freeze O_Freeze = {
	.pname = OBJECT_FREEZE_S,
	.P_Init = {
		.pname 	= PROPERTY_INIT_S,
		.value	= 0,						// immer 0 beim Start!
	},
	.P_InitExport = {
		.pname 	= "InitExport(Hidden)",		// intern verwendet, d.h
		.value	= 0,						// P_InitExport wird weder in JSON noch
	},										// durch das Sys-File im- bzw. exportiert 
	.P_Size = {
		.pname 	= PROPERTY_SIZE_S,
		.value	= SIZE_VALUE64,
	},
	.P_Value = {
		.pname 	= PROPERTY_VALUE_S,
		.value	= 0,
	},
};

/// Devive
struct tdi_dev_inp di_O_Inputs = {
	.Op_Mode 	= &O_Mode,
	.Op_Value	= &O_Value,
	.Op_Freeze	= &O_Freeze,
}; 

/// Driver
struct tdi_drv di_O_Mct = {
	.Op_Control = &O_Control,
	.Op_Physic	= &O_Physic,
	.Op_Inputs	= &di_O_Inputs,
};
 
///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
		{.n = {.type=nType_unused}}};
// oDevice.oMode.Mode
STATIC  u_node jsonkey_mode[4] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_S}},	\
		{.nK = {.type=nType_string,.node=OBJECT_MODE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_MODE_S}},	\
		{.n = {.type=nType_unused}}
};
// oDevice.oValue.Value
STATIC u_node jsonkey_value[4] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_S}},	\
		{.nK = {.type=nType_string,.node=OBJECT_VALUE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_VALUE_S}},	\
		{.n = {.type=nType_unused}}
};
// oDevice.oFreeze.Value
STATIC u_node jsonkey_freeze[4] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_S}},	\
		{.nK = {.type=nType_string,.node=OBJECT_FREEZE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_VALUE_S}},	\
		{.n = {.type=nType_unused}}
};
// oDevice.oValue.Name
STATIC u_node jsonkey_name[4] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_DEVICE_S}},	\
		{.nK = {.type=nType_string,.node= OBJECT_VALUE_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_NAME_S}},	\
		{.n = {.type=nType_unused}}
};

///*********************************************************
///  OBJECT-FUNTIONS
///*********************************************************
void di_objs_create(struct tdi_drv * drv) {
	OBJS_CREATE_IDEV_PTR;	
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	drv->Op_Physic->P_Pin.value = di_pin;				// "siehe: mct_di_x.c-File"
	strcpy(idev->Op_Value->P_Name.value, di_pin_name);	// "siehe: mct_di_x.c-File"
	return;
}

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//
// 				Eingabeparameter unmittelbar beim Treiberstart checken
// 				- command
//				- name
//				- input
//				- freeze
//				- init (flag) //HACK  init ReadOnly
//
// Vorbeugend schon mal den zugehörigen param Parameter einlesen und anschließend auf
// die zulässige bit-Länge prüfen und begrenzen! 
//	
//				OCommand,OValue,OInit, werden initialisiert. 
//				Es ist zu diesem Zeitpunkt kein 
//				Zugriffsschutz notwendig , da die Timer noch nicht aktiv sind!
//
//***********************************************************************************
void di_objs_generic(void)	
{	
	unsigned long long 	fre = 0;
	unsigned long long	val = 0;

	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);

	val = simple_strtoull(input, NULL, 10);		// input
	fre = simple_strtoull(freeze, NULL, 10);	// freeze wert

	/// Parameter: command= start,stop
	if(command == e_start || command == e_stop)	{
		di_O_Mct.Op_Control->P_Control.value[0] = command;
		di_O_Mct.Op_Control->P_Control.value[1] = command;	
	}
	/// Parameter: name=? 
	if(strlen(name) > 0) { 
		strncpy((char*)&di_O_Inputs.Op_Value->P_Name.value,(char*)&name,PROPERTY_SIZE+1);
		di_O_Inputs.Op_Value->P_Name.value[PROPERTY_SIZE] = 0;	// terminate
	}
	/// Parameter: mode=?
	// Standard-Initialisierung ist erst mal der direkt-Mode ohne Parameter.	
	// Prüfen ob eventuell Mode: SO
	if (!strcmp(mode,PROPERTY_MODE_S0_S)) {
		strcpy((char*)&di_O_Inputs.Op_Mode->P_Name.value,PROPERTY_MODE_S0_S);	
		di_O_Inputs.Op_Mode->P_Value.value = MODE_S0;		// interner Merker!
		di_O_Inputs.Op_Value->P_Size.value = SIZE_VALUE64;	// 64 bit
		di_O_Inputs.Op_Value->P_Value.value = val;			// externer Zählerwert
		di_O_Inputs.Op_Freeze->P_Value.value = fre;			// externer Freezewert
	}	
	// Mode: "direkt" 
	else {
		// OValue
		di_O_Inputs.Op_Value->P_Value.value = val & SIZE_MAX_1bit;	// 1 bit
	}
/*
	printk("EINSTELLUNGEN Start\n");	
	printk("%s: %s %d\n",
			di_O_Mct.Op_Control->pname,
			di_O_Mct.Op_Control->P_Control.pname[0],
			di_O_Mct.Op_Control->P_Control.value[0]);
	printk("%s: %s %s %d\n",
			di_O_Inputs.Op_Mode->pname,
			di_O_Inputs.Op_Mode->P_Name.pname,
			di_O_Inputs.Op_Mode->P_Name.value,
			di_O_Inputs.Op_Mode->P_Value.value);
	printk("%s: %s %lld\n",
			di_O_Inputs.Op_Value->pname,
			di_O_Inputs.Op_Value->P_Value.pname,
			di_O_Inputs.Op_Value->P_Value.value);
	printk("%s: %s %lld\n",
			di_O_Inputs.Op_Freeze->pname,
			di_O_Inputs.Op_Freeze->P_Value.pname,
			di_O_Inputs.Op_Freeze->P_Value.value);
	printk("%s: %s %d\n",
			di_O_Inputs.Op_Freeze->pname,
			di_O_Inputs.Op_Freeze->P_Init.pname,
			di_O_Inputs.Op_Freeze->P_Init.value);
	printk("%s: %s %s\n",
			di_O_Inputs.Op_Value->pname,
			di_O_Inputs.Op_Value->P_Name.pname,
			di_O_Inputs.Op_Value->P_Name.value);
	printk("EINSTELLUNGEN Ende\n");	
*/
	return;
}

//***********************************************************************************
// Funktion:	Der Aufruf erfolgt automatisch, wenn der json-Parameter="{...}"
//				mit einem String aufgerufen wird. Als Format wird JSON-Syntax erwartet.
//				Folgende JSON-Elemente werden gesucht  und in die Werte in den 
//				entsprechenden Übergabeparametern abgelegt. 
// 
//				oControl.Command						=> command
//				oDevice.oMode.value		("direkt","S0")	=> mode
//				oDevice.oValue.Value 					=> input
//				oDevice.oFreeze.Value					=> freeze
//				HACK  init ReadOnly
//				// oDevice.oFreeze.Init					=> init
//				oDevice.oValue.Name						=> name
//
//				Anschließend wird di_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t di_objs_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	char  			_buf[JSON_LIMIT_SIZE+1]; 
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
	/// DEVICE
	///------------------
	/// MODE
	memcpy(_buf,di_json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_mode[0], sizeof(jsonkey_mode));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
	if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))
		memcpy(&mode,&ctx.val.vS.value,strlen(ctx.val.vS.value));
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN: mode %s\n",mode);

	/// VALUE
	memcpy(_buf,di_json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_value[0], sizeof(jsonkey_value));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle
	if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= STRING_LLD_SIZE))
		memcpy(&input,&ctx.val.vS.value,strlen(ctx.val.vS.value));
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN: input %s\n", input);

	/// FREEZE
	memcpy(_buf,di_json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_freeze[0], sizeof(jsonkey_freeze));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
	if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= STRING_LLD_SIZE))
		memcpy(&freeze,&ctx.val.vS.value,strlen(ctx.val.vS.value));
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN: param %s\n", freeze);

	/// INIT (number)
	//HACK  init ReadOnly
	/*
	memcpy(_buf,di_json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_init[0], sizeof(jsonkey_init));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: Integer , 3.Integer > 0 
	if(!ctx.err.number && ctx.val.v.type == vType_int && ctx.val.vI.value)
		O_Freeze.P_Init.value = ctx.val.vI.value & 0x01;
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN: init %d\n", O_Freeze.P_Init.value);
	*/

	/// NAME
	memcpy(_buf,di_json,len);
	// Context-Part
	Context_Init(&ctx, &D);
	Context_Build(&ctx, &jsonkey_name[0], sizeof(jsonkey_name));
	// Decoder-Part
	JsonDecode_Init(&D,&ctx);
	JsonDecode(&D, _buf, len);
	// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
	if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))
		memcpy(&name,&ctx.val.vS.value,strlen(ctx.val.vS.value));
	DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-IN: name %s\n", name);

	DEBUG(MCT_DEBUG_LEVEL3, "JSON: End\n");	
	return len;
};

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param		( command, byte, S_IRUGO);
module_param_string	( json, di_json, sizeof(di_json), S_IRUGO);

module_param_string	( mode,  mode, sizeof(mode), S_IRUGO);
module_param_string	( name,  name, sizeof(name), S_IRUGO);
module_param_string	( input, input, sizeof(input), S_IRUGO);
module_param_string	( freeze,freeze, sizeof(freeze), S_IRUGO);
//HACK  init ReadOnly
//module_param_named(init,O_Freeze.P_Init.value, bool, S_IRUGO);

