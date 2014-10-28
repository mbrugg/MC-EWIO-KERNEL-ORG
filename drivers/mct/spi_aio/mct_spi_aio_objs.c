/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#include <linux/module.h>
#include <linux/string.h>

#include "../mct_debug.h"			// debug-support
#include "mct_spi_aio_objs.h"
#include "mct_aio.h"

//HACK JSON-SYSFS-WRITE
//#define STATIC		
#define STATIC static

// check length of propertie's   zero-string("") if 
#define PROPERTY_CHECK_VALUE_LEN(x,y)  	if (strlen(x) > PROPERTY_SIZE)	{\
					  (y)[0] = 0;	}	\
					  else	{	\
						  strncpy(y,x,PROPERTY_SIZE+1);	\
						  (y)[PROPERTY_SIZE] = 0;	\
					  }
// Messbereiche für die ananlogen Eingänge 1 und 2
static char * const ai_12_modes_tab[] 	= {
	AI_PROP_MODE_0_S, AI_PROP_MODE_1_S, AI_PROP_MODE_2_S,
	AI_PROP_MODE_3_S, AI_PROP_MODE_4_S, AI_PROP_MODE_5_S,
	AI_PROP_MODE_6_S, AI_PROP_MODE_7_S, AI_PROP_MODE_8_S,
	AI_PROP_MODE_9_S, 
	NULL};

const char * ai_12_label_tab[] 	= {
	AI_LAB_MODE_0_S, AI_LAB_MODE_1_S, AI_LAB_MODE_2_S,
	AI_LAB_MODE_3_S, AI_LAB_MODE_4_S, AI_LAB_MODE_5_S,
	AI_LAB_MODE_6_S, AI_LAB_MODE_7_S, AI_LAB_MODE_8_S,
	AI_LAB_MODE_9_S,
	NULL};

// Messbereiche für die ananlogen Eingänge 3 und 4
static char * const ai_34_modes_tab[] 	= {
	AI_PROP_MODE_0_S, AI_PROP_MODE_A_S, 
	NULL};

const char * ai_34_label_tab[] 	= {
	AI_LAB_MODE_0_S, AI_LAB_MODE_A_S,
	NULL};


///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
char			aio_json[JSON_LIMIT_SIZE];	// JSON formatierte Eingabe
STATIC int 		command;					// START/STOP
// DEVICE-ARGs
STATIC char 	in_name_0[PROPERTY_SIZE+1]; // Bezeichner analoger Eingangswert - Kanal 0
STATIC char 	in_name_1[PROPERTY_SIZE+1]; // Bezeichner analoger Eingangswert - Kanal 1
STATIC char 	in_name_2[PROPERTY_SIZE+1]; // Bezeichner analoger Eingangswert - Kanal 2
STATIC char 	in_name_3[PROPERTY_SIZE+1]; // Bezeichner analoger Eingangswert - Kanal 3

STATIC char	in_mode_0[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;// Messbereich Input- Kanal 0 
STATIC char 	in_mode_1[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;// Messbereich Input- Kanal 1 
STATIC char	in_mode_2[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;// Messbereich Input- Kanal 2
STATIC char	in_mode_3[PROPERTY_SIZE+1] = AI_PROP_MODE_0_S;// Messbereich Input- Kanal 3

STATIC char 	out_name_0[PROPERTY_SIZE+1];// Bezeichner analoger Ausgabewert - Kanal 0
STATIC char 	out_name_1[PROPERTY_SIZE+1];// Bezeichner analoger Ausgabewert - Kanal 1
STATIC char 	out_name_2[PROPERTY_SIZE+1];// Bezeichner analoger Ausgabewert - Kanal 2
STATIC char 	out_name_3[PROPERTY_SIZE+1];// Bezeichner analoger Ausgabewert - Kanal 3

STATIC unsigned int output_0;		// 10bit ananloger Ausgabewert - Kanal 0 
STATIC unsigned int output_1;		// 10bit ananloger Ausgabewert - Kanal 1 
STATIC unsigned int output_2;		// 11bit ananloger Ausgabewert - Kanal 2
STATIC unsigned int output_3;		// 11bit ananloger Ausgabewert - Kanal 3

///*********************************************************
///  DRIVER-PARAMETER EXPORT
///*********************************************************
module_param_string( json, aio_json, sizeof(aio_json), S_IRUGO);	
module_param( command, int, S_IRUGO);

module_param_string( in_name_0, in_name_0, sizeof(in_name_0), S_IRUGO);
module_param_string( in_name_1, in_name_1, sizeof(in_name_1), S_IRUGO);
module_param_string( in_name_2, in_name_2, sizeof(in_name_2), S_IRUGO);
module_param_string( in_name_3, in_name_3, sizeof(in_name_3), S_IRUGO);

module_param_string( in_mode_0, in_mode_0, sizeof(in_mode_0), S_IRUGO);
module_param_string( in_mode_1, in_mode_1, sizeof(in_mode_1), S_IRUGO);
module_param_string( in_mode_2, in_mode_2, sizeof(in_mode_2), S_IRUGO);
module_param_string( in_mode_3, in_mode_3, sizeof(in_mode_3), S_IRUGO);

module_param_string( out_name_0, out_name_0, sizeof(out_name_0), S_IRUGO);
module_param_string( out_name_1, out_name_1, sizeof(out_name_1), S_IRUGO);
module_param_string( out_name_2, out_name_2, sizeof(out_name_2), S_IRUGO);
module_param_string( out_name_3, out_name_3, sizeof(out_name_3), S_IRUGO);

module_param( output_0, uint, S_IRUGO);
module_param( output_1, uint, S_IRUGO);
module_param( output_2, uint, S_IRUGO);
module_param( output_3, uint, S_IRUGO);

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

// OPhysik
static obj_physic O_Physic = {
	.pname	 	= OBJECT_PHYSIC_S,	
	.lock = SPIN_LOCK_UNLOCKED,
	.P_PICLayer = {
		.pname = PROPERTY_LAYER_PIC_S,
		.value = 0,	 // bcd-Versionskz des PIC
	},
	.P_SPILayer = {
		.pname = PROPERTY_LAYER_SPI_S,
	}
};

// OError
static obj_error O_Error = { 
	.pname	 	= OBJECT_ERROR_S,
	.lock = SPIN_LOCK_UNLOCKED,
	.P_Errors = {
		{.pname=PROPERTY_ERROR_UART_Retrieve,.value=0},	// Mehrfachversuche bei Initialisierung	
		{.pname=PROPERTY_ERROR_UART_Timeout,.value=0},	// Timeout, keine Antwort vom PIC	
		{.pname=PROPERTY_ERROR_UART_Frame,.value=0},	// unbekannter Frame vom PIC
		{.pname=PROPERTY_ERROR_UART_State,.value=0},	// LSR-Register Error-Code
		{.pname=PROPERTY_ERROR_PIC_PIC,.value=0} 		// 0xC1 -Code	
	},
};	

/// Device Outputs
// OValuesOut
static obj_value_out O_ValuesOut[DEVICE_AIO_OUT_MAX] = {
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AO1 (O1/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE10},	// bitbreite 10
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0}
	},
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AO2 (O2/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE10},	// bitbreite 10
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0}
	},
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AO3 (C1/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE11},	// bitbreite 11
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0}
	},
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AO4 (C2/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE11},	// bitbreite 11
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0}
	}
}; 

taio_dev_outp aio_O_Outputs[DEVICE_AIO_OUT_MAX] = {
	{.Op_Value= &O_ValuesOut[0]},
	{.Op_Value= &O_ValuesOut[1]},
	{.Op_Value= &O_ValuesOut[2]},
	{.Op_Value= &O_ValuesOut[3]}
};

/// Device Inputs
// OValuesIn
static obj_value_in O_ValuesIn[DEVICE_AIO_IN_MAX] = {
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AI1 (E1/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE32},	// bitbreite 32
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0},
		.P_Offset = {.pname = PROPERTY_OFFSET_S, .value = 0}		// R-Abgleich!
	},
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AI2 (E2/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE32},	// bitbreite 32
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0},
		.P_Offset = {.pname = PROPERTY_OFFSET_S, .value = 0}		// R-Abgleich!
	},
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AI3 (I1/-)"},
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE32},	// bitbreite 32
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0}
	},
	{	.pname = OBJECT_VALUE_S,
		.lock = SPIN_LOCK_UNLOCKED,
		.P_Name = {.pname = PROPERTY_NAME_S, .value = "AI4 (I2/-)" },
		.P_Size = {.pname = PROPERTY_SIZE_S, .value = SIZE_VALUE32},	// bitbreite 32
		.P_Value = {.pname = PROPERTY_VALUE_S, .value = 0}
	}
};

// OTypes
static obj_type O_Types[DEVICE_AIO_IN_MAX] = {
	{	.pname = OBJECT_TYPE_S,
		.P_Type = { .pname = PROPERTY_MODE_S,
					.setup = &box_setup,
					.select = &box_select,
					.sprintf_sel = &box_sprintf_sel,
					.get_id	 = &box_get_id
				}
	},
	{	.pname = OBJECT_TYPE_S,
		.P_Type = { .pname = PROPERTY_MODE_S,
					.setup = &box_setup,
					.select = &box_select,
					.sprintf_sel = &box_sprintf_sel,
					.get_id	 = &box_get_id
				}
	},
	{	.pname = OBJECT_TYPE_S,
		.P_Type = { .pname = PROPERTY_MODE_S,
					.setup = &box_setup,
					.select = &box_select,
					.sprintf_sel = &box_sprintf_sel,
					.get_id	 = &box_get_id
				}
	},
	{	.pname = OBJECT_TYPE_S,
		.P_Type = { .pname = PROPERTY_MODE_S,
					.setup = &box_setup,
					.select = &box_select,
					.sprintf_sel = &box_sprintf_sel,
					.get_id	 = &box_get_id
				}
	}	
};

taio_dev_inp aio_O_Inputs[DEVICE_AIO_IN_MAX] = {
	{.Op_Value	= &O_ValuesIn[0], .Op_Type= &O_Types[0]},
	{.Op_Value	= &O_ValuesIn[1], .Op_Type= &O_Types[1]},
	{.Op_Value	= &O_ValuesIn[2], .Op_Type= &O_Types[2]},
	{.Op_Value	= &O_ValuesIn[3], .Op_Type= &O_Types[3]}
};

/// Driver
struct 	taio_drv aio_O_Mct = {
	.Op_Control = &O_Control,
//	aio_control	* 	ct;				zur Laufzeit!
	.Op_Physic	= &O_Physic,
	.Op_Error	= &O_Error,
	.Op_Inputs	= {&aio_O_Inputs[0],&aio_O_Inputs[1],&aio_O_Inputs[2],&aio_O_Inputs[3]},
	.Op_Outputs	= {&aio_O_Outputs[0],&aio_O_Outputs[1],&aio_O_Outputs[2],&aio_O_Outputs[3]}
};


///*********************************************************
///  JSON-STACK-NODES (only for json-Interface) 
///*********************************************************
// oControl.Command
STATIC u_node jsonkey_command[3] =  {	\
		{.nK = {.type=nType_string,.node=OBJECT_CONTROL_S}},	\
		{.nK = {.type=nType_string,.node=PROPERTY_COMMAND_S}},	\
		{.n = {.type=nType_unused}}};

// oDevices[Inputs/Outputs].oValues.Name 
STATIC u_node jsonkey_io_name[5] =  {	\
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

//***********************************************************************************
// Funktion:	Automatisch wird immer das Generische-Interface verwendet!
//				<GENERIC>
// 				Eingabeparameter beim Treiberstart checken, einstellbar als 
//				ARG-Parameter sind: 
//
//				command		==>	aio_O_Mct.Op_Control->P_Control.value[]
//				output_name[x]	==>	O_ValuesOut[x].P_Name.value
//				output_[x]	==> 	aio_O_Outputs[x].Op_Value->P_Value.value
//
//				input_name[x]	==> 	O_ValuesIn[x].P_Name.value
//				in_mode_0[x]	==> 	aio_O_Inputs[x].Op_Type->P_Type
//
//				Es ist zu diesem Zeitpunkt kein  Zugriffsschutz notwendig , 
//				da der Timer noch nicht aktiv ist!
//***********************************************************************************
void aio_objects_generic(void) {	
	unsigned int idx = 0;
	char _mode[PROPERTY_SIZE+1] = {0};
	char _name[PROPERTY_SIZE+1] = {0};	
//	char _buffer[300];
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	
	// HACK SHOW HACK
	// command = e_start;

	/// *** Property COMMAND 	
	if(command == e_start || command == e_stop)	{		// ARG: "command" übernehmen
		aio_O_Mct.Op_Control->P_Control.value[0] = command; 	// Hinweis: Ein Externes Kommando wird auf [0] geschrieben,
		aio_O_Mct.Op_Control->P_Control.value[1] = command;	// während die internen Stati in [1] stehen
	}

	/// *** Properties IN_NAME_x laden, wenn Eintrag
	PROPERTY_CHECK_VALUE_LEN(in_name_0, _name);
		if(strlen(_name)) 
			strcpy(O_ValuesIn[0].P_Name.value,_name);
	PROPERTY_CHECK_VALUE_LEN(in_name_1, _name);
		if(strlen(_name)) 
			strcpy(O_ValuesIn[1].P_Name.value,_name);
	PROPERTY_CHECK_VALUE_LEN(in_name_2, _name);
		if(strlen(_name)) 
			strcpy(O_ValuesIn[2].P_Name.value,_name);
	PROPERTY_CHECK_VALUE_LEN(in_name_3, _name);
		if(strlen(_name)) 
			strcpy(O_ValuesIn[3].P_Name.value,_name);

	/// *** Properties TYPE_x initialisieren
	for	(idx= 0; idx < DEVICE_AIO_IN_MAX; idx++)	{
		if(idx < DEVICE_AIO_IN_12_MAX) {
			box_setup((&aio_O_Inputs[idx].Op_Type->P_Type), 0, ai_12_modes_tab);
		}
		else	{
			box_setup((&aio_O_Inputs[idx].Op_Type->P_Type), 0, ai_34_modes_tab);
		}

		switch (idx) {
			case 0: PROPERTY_CHECK_VALUE_LEN(in_mode_0,_mode); break;
			case 1: PROPERTY_CHECK_VALUE_LEN(in_mode_1,_mode); break;
			case 2: PROPERTY_CHECK_VALUE_LEN(in_mode_2,_mode); break;
			case 3: PROPERTY_CHECK_VALUE_LEN(in_mode_3,_mode); break;
		} 
		// Wenn der Typ unbekannt ist, wird der Input auf inaktiv eingestellt
		if(box_select((&aio_O_Inputs[idx].Op_Type->P_Type), _mode) == false)
			box_select((&aio_O_Inputs[idx].Op_Type->P_Type), AI_PROP_MODE_0_S);
	}

	/// *** Properties OUT_NAME_x laden, wenn Eintrag
	PROPERTY_CHECK_VALUE_LEN(out_name_0, _name);
		if(strlen(_name)) 
			strcpy(O_ValuesOut[0].P_Name.value,_name);
	PROPERTY_CHECK_VALUE_LEN(out_name_1, _name);
		if(strlen(_name) > 0) 
			strcpy(O_ValuesOut[1].P_Name.value,_name);
	PROPERTY_CHECK_VALUE_LEN(out_name_2, _name);
		if(strlen(_name) > 0) 
			strcpy(O_ValuesOut[2].P_Name.value,_name);
	PROPERTY_CHECK_VALUE_LEN(out_name_3, _name);
		if(strlen(_name) > 0) 
			strcpy(O_ValuesOut[3].P_Name.value,_name);

	/// *** Properties OUTPUT_x laden
	aio_O_Outputs[0].Op_Value->P_Value.value = (output_0 & SIZE_MAX_10bit);
	aio_O_Outputs[1].Op_Value->P_Value.value = (output_1 & SIZE_MAX_10bit);
	aio_O_Outputs[2].Op_Value->P_Value.value = (output_2 & SIZE_MAX_11bit);
	aio_O_Outputs[3].Op_Value->P_Value.value = (output_3 & SIZE_MAX_11bit);

/*
	// print adjust's
	printk("EINSTELLUNGEN Start\n");	
	printk("---< Driver >---\n");
	printk("%s: %d\n",  aio_O_Mct.Op_Control->P_Control.pname[0], aio_O_Mct.Op_Control->P_Control.value[0]); 
	for (idx = 0; idx < DEVICE_AIO_IN_MAX; idx++)	{ 
		printk("---< In-Device %d >---\n",idx);
		box_sprintf_sel((&aio_O_Inputs[idx].Op_Type->P_Type), _buffer);
		printk("%s\n",_buffer);
	}
	for (idx = 0; idx < DEVICE_AIO_OUT_MAX; idx++)	{ 
		printk("---< Out-Device %d >---\n",idx);
		printk("%s: %d\n",aio_O_Outputs[idx].Op_Value->P_Value.pname, aio_O_Outputs[idx].Op_Value->P_Value.value);	
	}
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
//				oControl.Command (e_Start,e_Stop Driver)	==> command
//				{
//				oDevices[0-3].oValue.Name					==> in_name_[0-3]
//				oDevices[0-3].Mode							==> in_mode_[0-3]
//				}
//				{	
//				oDevices[4-7].oValue.Name					==> out_name_[0-2]
//				oDevices[4-7].oValue						==> output_[0-2]
//				}
//
//				Anschließend wird dio_objects_generic() aufgerufen - und die Eingabe-
//				parameter zum Teil kontrolliert. Je nach Wichtigkeit erfolgt eine
//				genaue Überprüfung oder z.B. nur der Längencheck.
//  
//************************************************************************************ 
ssize_t aio_objects_json(const char  * json) {
	JsonDecode_State D;
	elivator  		ctx;
	ssize_t 		len = strlen(json);
	int 			_ch = 0;	
	char  			_buf[JSON_LIMIT_SIZE+1]; 
	char 			_mode[PROPERTY_SIZE+1];
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

	/// INPUT DEVICES....
	for (_ch= 0; _ch < DEVICE_AIO_IN_MAX; _ch++)	{
		memset(_name,0, sizeof(_name)); 
		memset(_mode,0, sizeof(_mode));
		/// NAME
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_io_name[1].nA.node = _ch;
		Context_Build(&ctx, &jsonkey_io_name[0], sizeof(jsonkey_io_name));
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
		switch (_ch) {
			case DEVICE_IN_0:
				memcpy(&in_name_0,&_name,strlen(_name)+1);
				memcpy(&in_mode_0,&_mode,strlen(_mode)+1);	
				break;
			case DEVICE_IN_1:
				memcpy(&in_name_1,&_name,strlen(_name)+1);
				memcpy(&in_mode_1,&_mode,strlen(_mode)+1);	
				break;
			case DEVICE_IN_2:
				memcpy(&in_name_2,&_name,strlen(_name)+1);
				memcpy(&in_mode_2,&_mode,strlen(_mode)+1);	
				break;
			case DEVICE_IN_3:
				memcpy(&in_name_3,&_name,strlen(_name)+1);
				memcpy(&in_mode_3,&_mode,strlen(_mode)+1);	
				break;
			default:
				DEBUG(MCT_DEBUG_LEVEL3, "%s %s(): %s %d\n", __FILE__, __FUNCTION__, "Attention unsupported Device_IN number!", _ch);
				break;
		}
	}
	/// OUTPUT DEVICES.... 
	for (_ch= 0; _ch < DEVICE_AIO_OUT_MAX; _ch++)	{
		memset(_name,0, sizeof(_name)); 
		_out_val = 0;
		/// NAME
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_io_name[1].nA.node = DEVICE_AIO_IN_MAX + _ch;	// Offset beachten!
		Context_Build(&ctx, &jsonkey_io_name[0], sizeof(jsonkey_io_name));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: String,  3.Längenkontrolle 
		if(!ctx.err.number && ctx.val.v.type == vType_str && (strlen(ctx.val.vS.value)<= PROPERTY_SIZE))
			memcpy(&_name,&ctx.val.vS.value,strlen(ctx.val.vS.value));		 
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-%d: name %s\n",_ch, _name);
		
		/// VALUE
		memcpy(_buf,json,len);
		// Context-Part
		Context_Init(&ctx, &D);
		jsonkey_output[1].nA.node = DEVICE_AIO_IN_MAX + _ch;	// Offset beachten!
		Context_Build(&ctx, &jsonkey_output[0], sizeof(jsonkey_output));
		// Decoder-Part
		JsonDecode_Init(&D,&ctx);
		JsonDecode(&D, _buf, len);
		// Fehlerüberwachung: 1.Parserdurchlauf i.O,  2.Returnwert: Integer , 3.Integer > 0 
		if(!ctx.err.number && ctx.val.v.type == vType_int && ctx.val.vI.value)
		_out_val = ctx.val.vI.value;
		DEBUG(MCT_DEBUG_LEVEL3, "JSON-DEV-OUT-%d: value %d\n",_ch, _out_val);
		
		/// Mappe alle Werte auf die Argument-Variablen, diese werden dann ausgewertet
		switch (_ch) {
			case DEVICE_OUT_0:
				memcpy(&out_name_0,&_name,strlen(_name)+1);
				output_0 = _out_val;
				break;
			case DEVICE_OUT_1:
				memcpy(&out_name_1,&_name,strlen(_name)+1);
				output_1 = _out_val;
				break;
			case DEVICE_OUT_2:
				memcpy(&out_name_2,&_name,strlen(_name)+1);
				output_2 = _out_val;
				break;
			case DEVICE_OUT_3:
				memcpy(&out_name_3,&_name,strlen(_name)+1);
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
