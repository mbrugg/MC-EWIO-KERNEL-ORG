/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		
 	
 	Description:  mct Typedefs

	HackList:	
	04.05.2011	Raus:	typedef int  BOOL
 			Neu:	prop_bool	
 *******************************************************************************/
#ifndef MCT_TYPES_H_
	#define MCT_TYPES_H_
#include <linux/module.h>
#include <linux/spinlock.h>

#define PROPERTY_SIZE		50
#define STRING_LLD_SIZE 	20			// Stellen:20 = 18.446.744.073.709.551.615
#define STRING_U32_SIZE		10			// Stellen:10 = 4.294.967.295
#define STRING_MAX_VALUE	"18446744073709551615"	// 20 stellig als String /0-terminated
#define STRING_MIN_VALUE	"00000000"

#define SIZE_VALUE64		64		// liefert Länge 64bit
#define SIZE_VALUE32		32		// liefert Länge 32bit
#define SIZE_VALUE16		16		// liefert Länge 16bit
#define SIZE_VALUE11		11		// liefert Länge 11bit
#define SIZE_VALUE10		10		// liefert Länge 10bit
#define SIZE_VALUE8		8		// liefert Länge 8bit
#define	SIZE_VALUE4		4	
#define	SIZE_VALUE2		2	
#define SIZE_VALUE1		1		// liefert Länge 1bit

#define SIZE_MAX_1bit		0x1		// 1bit
#define SIZE_MAX_2bit		0x3		// 2bit
#define SIZE_MAX_4bit		0x0f		// 4bit
#define SIZE_MAX_10bit		0x03ff		// 10bit
#define SIZE_MAX_11bit		0x07ff		// 11bit
#define SIZE_MAX_16bit		0xffff		// 16bit
#define SIZE_MAX_24bit		0x00ffffff	// 24bit
#define SIZE_MAX_32bit		0xffffffff	// 32bit

#define SIZE_MAX_DEZ_1bit	1		// 1bit
#define SIZE_MAX_DEZ_2bit	3		// 2bit
#define SIZE_MAX_DEZ_4bit	15		// 4bit
#define SIZE_MAX_DEZ_8bit	255		// 8bit
#define SIZE_MAX_DEZ_10bit	1023		// 10bit
#define SIZE_MAX_DEZ_11bit	2047		// 11bit
#define SIZE_MAX_DEZ_16bit	65535		// 16bit
#define SIZE_MAX_DEZ_24bit	16777215	// 24bit
#define SIZE_MAX_DEZ_32bit	4294967295	// 32bit

///******************************
/// Object-defines
///******************************
// Objects
#define OBJECT_DRIVER_S		"oDriver"
#define OBJECT_DEVICE_S		"oDevice"
#define OBJECT_DEVICE_Ss	"oDevices"
#define OBJECT_CONTROL_S	"oControl"
#define OBJECT_VALUE_S		"oValue"
#define OBJECT_VALUE_Ss		"oValues"
#define OBJECT_TYPE_S		"oType" 
#define OBJECT_PHYSIC_S		"oPhysic"
#define OBJECT_MASK_S		"oMask"
#define OBJECT_INPUT_S		"oInput"
#define OBJECT_ATTR_S		"oAttribute"
#define OBJECT_PARAM_S		"oParam"
#define OBJECT_ERROR_S		"oError"
#define OBJECT_ERROR_Ss		"oErrors"
#define OBJECT_SLOT_Ss		"oSlots"
#define OBJECT_LBUS_S		"oLBus"
#define OBJECT_TIMEOUT_S	"oTimeout"
#define OBJECT_TIMEOUT_Ss	"oTimeouts"
#define OBJECT_MODE_S		"oMode"
#define OBJECT_MODE_Ss		"oModes"
#define OBJECT_FREEZE_S		"oFreeze"
#define OBJECT_INIT_S		"oInit"
#define OBJECT_INIT_Ss		"oInits"

// Names
#define PROPERTY_PARAM_S	"Parameter"
#define PROPERTY_POSITION_S	"Position"
#define PROPERTY_MESSAGE_S	"Message"
#define PROPERTY_COMMAND_S	"Command"	
#define PROPERTY_ERROR_S	"Error"
#define PROPERTY_NAME_S		"Name"
#define PROPERTY_VALUE_S	"Value"
#define PROPERTY_STATE_S	"State"	
#define PROPERTY_MODE_S		"Mode"
#define PROPERTY_SIZE_S		"Size"
#define PROPERTY_MASK_S		"Mask"
#define PROPERTY_SLOT_S		"Slot"
#define PROPERTY_SLOT_Ss	"Slots"
#define PROPERTY_ADDR_S		"Adress"	
#define PROPERTY_PIN_S		"Pin"
#define PROPERTY_TRI_S		"Tri"
#define PROPERTY_VERSION_S	"Version"
#define PROPERTY_INFO_S		"Info"
#define PROPERTY_INIT_S		"Init"
#define PROPERTY_LINK_S		"Link"
#define PROPERTY_OFFSET_S	"Offset"

// Layers
#define PROPERTY_LAYER_BUS_S	"Bus"
#define PROPERTY_LAYER_SPI_S	"SPI"
#define PROPERTY_LAYER_PIC_S	"PIC"


// *****************************
// Driver and Device States
//******************************
enum e_state{			// Values for OState
	e_idle 		= 100,
	e_start 	= 1,	// external control/switchable
	e_running 	= 2,
	e_stop 		= 0,	// external control/switchable
	e_error 	= 200,
				// spezials
	e_config 	= 50,	// configure mode
	e_init 		= 60,	// init mode
	e_reset  	= 70,	// reset mode
	e_shutdown	= 80,	// shutdown
	e_byebye	= 90	// bye 
};

#define PROPERTY_COMMAND_AND_STATE_IDLE		100
#define PROPERTY_COMMAND_AND_STATE_START	1
#define PROPERTY_COMMAND_AND_STATE_RUNNING	2
#define PROPERTY_COMMAND_AND_STATE_STOP		0
#define PROPERTY_COMMAND_AND_STATE_ERROR	200
#define PROPERTY_COMMAND_AND_STATE_CONFIG	50
#define PROPERTY_COMMAND_AND_STATE_INIT		60
#define PROPERTY_COMMAND_AND_STATE_RESET	70
#define PROPERTY_COMMAND_AND_STATE_SHUTDOWN	80


// expands 
#define LABEL_COMMAND_AND_STATE_IDLE		"idle"
#define LABEL_COMMAND_AND_STATE_START		"start"
#define LABEL_COMMAND_AND_STATE_RUNNING		"running"
#define LABEL_COMMAND_AND_STATE_STOP		"stop"
#define LABEL_COMMAND_AND_STATE_ERROR		"error"
#define LABEL_COMMAND_AND_STATE_CONFIG		"config"
#define LABEL_COMMAND_AND_STATE_INIT		"init"
#define LABEL_COMMAND_AND_STATE_RESET		"reset"
#define LABEL_COMMAND_AND_STATE_SHUTDOWN	"shutdown"

#define LABEL_SLOT_ONLINE			"online"
#define LABEL_SLOT_OFFLINE			"offline"
#define LABEL_SLOT_MISSING			"missing"
#define LABEL_SLOT_CONFUSED			"confused"

// ***********************
// Timer-Type
//************************

struct timer{
	struct timer_list 	timer;
	atomic_t		atomic;
	struct completion	completion;
	unsigned long		granularity;
};

// ***********************
// Property-Types
//************************
// standard bool property	(0/1)
typedef struct {
	char  *		pname;	
	bool		value;
}prop_bool;

// standard byte property (unsigned 8 bit)
typedef struct {
	char  *		pname;	
	unchar		value;
}prop_byte;

// standard syncronized 2x8 Bit property
typedef struct {
	char  *	 	pname[2];
	unchar		value[2];
}prop_2x_byte;

// standard 16 Bit property
typedef struct {
	char  	*	pname;
	u16		value;
}prop_u16;

// standard 32 Bit property
typedef struct {
	char  	*	pname;
	u32		value;
}prop_u32;

// standard 64 Bit property
typedef struct {
	char  	*	pname;
	u64		value;
}prop_u64;

// standard string property
typedef struct {
	char * 		pname;
	char 		value[PROPERTY_SIZE+1];
} prop_str;

// standard string pointer to value property (z.B. für Hinweise
typedef struct {
	char * 		pname;
	char *		value;
} prop_pstr;

// ***********************
//		Object-Types
//************************
typedef struct  {
	char *		pname;		// "OControl"
	spinlock_t	lock;
	prop_2x_byte	P_Control;
} obj_control;

// universelle drv-Struktur
typedef struct	{
	obj_control * 	Op_Control;	//
} tpl_drv;


#endif
