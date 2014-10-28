/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#ifndef MCT_SPI_AIO_OBJS_H_
	#define MCT_SPI_AIO_OBJS_H_

#include <linux/platform_device.h>

#include "mct_spi_aio_uart.h"		// UART
#include "mct_spi_aio_pic.h"		// PIC-Steuerung
#include "../mct_types.h"
#include "../mct_boxes.h"
#include "../mct_json.h"
#include "../mct_aio_uni.h"		// analog In/Output Defs

///********************
/// CONTROL
///*********************
typedef struct  {
	struct spi_device *	spi;	
	struct timer_list 	timer;
	atomic_t		timer_stop_job;
	struct completion	timer_stop_quit;
	struct spi_message 	msg;
	atomic_t		uart;
	struct t_pic		pic;			// PIC Steuerstruktur
	atomic_t		pic_RcvTlgTicker;
	struct 			spi_transfer	InitX[UART_SEQS_MAXI];
	struct 			spi_transfer	RetrieveX[UART_SEQS_MAXI];
	struct 			spi_transfer	StateX[UART_SEQS_MAXI];
	struct 			spi_transfer	ErrorX[UART_SEQS_MAXI];
	struct 			spi_transfer	RxX;
	struct 			spi_transfer	TxX;
	#define DUAL_PART	SC_16IS750_FIFO_SIZE+1
	#define DUAL_RD_TX	0			// [000]
	#define DUAL_RD_RX	DUAL_PART		// [065]
	#define DUAL_RD_CMD	DUAL_RD_TX		// [000] RHR
	#define DUAL_RD_DATA	DUAL_RD_RX+1		// [066...]
	#define DUAL_WR_TX	2* DUAL_PART		// [130]
	#define DUAL_WR_RX	3* DUAL_PART		// [185...] nicht notwendig!
	#define DUAL_WR_CMD	DUAL_WR_TX		// [130] THR
	#define DUAL_WR_DATA	DUAL_WR_TX+1		// [131...]
	u8 					dual_buffer[4*DUAL_PART];
	u8 					cmdo_buffer[UART_SEQS_MAXI*2];
	u8 					cmdi_buffer[UART_SEQS_MAXI*2];
	spinlock_t			protect;	// protect the rest
	u8 				version_pic;	// Firmwareversion des PIC, bcd gepackt
} aio_control;			

///****************
/// Device Inputs
///****************
typedef struct  {			// obj_value_in
	char   *		pname;
	spinlock_t		lock;	
	prop_str 		P_Name;	
	prop_byte 		P_Size;
	prop_u32 		P_Value;
	prop_u32 		P_Offset;
} obj_value_in;

typedef struct  {			// obj_type
	char   * 		pname;
	struct prop_box P_Type;
} obj_type;

typedef struct {		// dev_inp
	char *				pname;
	obj_value_in  * 	Op_Value;	// Slot: analoger Eingangswert
	obj_type *			Op_Type;	// Slot: Type, d.h hier Messbereich
} taio_dev_inp;

///	Makros: Device Inputs
// Inp.OValue.PValue.value - lesen
#define OBJS_IDEV_RD_INPUT(idev,dst)	{	\
			spin_lock_bh(&(idev)->Op_Value->lock);	\
			dst = (idev)->Op_Value->P_Value.value;	\
			spin_unlock_bh(&(idev)->Op_Value->lock);}

// Inp.OValue.PValue.value - schreiben
#define OBJS_IDEV_WR_INPUT(idev,val)	{	\
			spin_lock_bh(&(idev)->Op_Value->lock);	\
			(idev)->Op_Value->P_Value.value = val;	\
			spin_unlock_bh(&(idev)->Op_Value->lock);}

// Inp.OValue.POffset.value - lesen
#define OBJS_IDEV_RD_OFFSET(idev,dst)	{	\
			spin_lock_bh(&(idev)->Op_Value->lock);	\
			dst = (idev)->Op_Value->P_Offset.value;	\
			spin_unlock_bh(&(idev)->Op_Value->lock);}

// Inp.OValue.POffset.value - schreiben
#define OBJS_IDEV_WR_OFFSET(idev,val)	{	\
			spin_lock_bh(&(idev)->Op_Value->lock);	\
			(idev)->Op_Value->P_Offset.value = val;	\
			spin_unlock_bh(&(idev)->Op_Value->lock);}

// Inp.OType.value - lesen
#define OBJS_IDEV_RD_TYPE(idev,dst)	{	\
			spin_lock_bh(&(idev)->Op_Type->lock);	\
			dst = (idev)->Op_Type->P_Value.value;	\
			spin_unlock_bh(&(idev)->Op_Type->lock);}

///****************
/// Device Outputs
///****************
typedef struct  {			// obj_value_out
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_Name;
	prop_byte 		P_Size;
	prop_u16 		P_Value;
} obj_value_out;

typedef struct {		// dev_outp
	char *				pname;
	obj_value_out  * 	Op_Value;	// Slot: analoger Ausgabewert
} taio_dev_outp;

///	Makros: Device Outputs
// Outp.OValue.PValue.value - schreiben  mit Überwachung 10/11 bit !
#define OBJS_ODEV_WR_OUTPUT_xBit(odev,src)	{	\
	spin_lock_bh(&(odev)->Op_Value->lock);	\
	if(((odev)->Op_Value->P_Size.value) == SIZE_VALUE10)	\
		(odev)->Op_Value->P_Value.value = (src & SIZE_MAX_10bit);	\
	else \
		(odev)->Op_Value->P_Value.value = (src & SIZE_MAX_11bit);	\
	spin_unlock_bh(&(odev)->Op_Value->lock);}

// Outp.OValue.PValue.value - lesen
#define OBJS_ODEV_RD_OUTPUT(odev,dst)	{	\
	spin_lock_bh(&(odev)->Op_Value->lock);	\
	dst = (odev)->Op_Value->P_Value.value;	\
	spin_unlock_bh(&(odev)->Op_Value->lock);}

// Outp.OValue.PSize.value - lesen
#define OBJS_ODEV_RD_SIZE(odev,dst)	{	\
	spin_lock_bh(&(odev)->Op_Value->lock);	\
	dst = (odev)->Op_Value->P_Size.value;	\
	spin_unlock_bh(&(odev)->Op_Value->lock);}

///****************
/// Driver
///****************
typedef struct  {					// obj_physic
	char   *		pname;
	spinlock_t		lock;
	prop_str 		P_SPILayer;		// SPI
	prop_byte 		P_PICLayer;		// PIC	
} obj_physic;

#define PROPERTY_ERROR_UART_Retrieve	"UART-Retrieve"
#define PROPERTY_ERROR_UART_Timeout	"UART-Timeout"
#define	PROPERTY_ERROR_UART_Frame	"UART-Frame"			
#define PROPERTY_ERROR_UART_State	"UART-State"
#define PROPERTY_ERROR_PIC_PIC		"PICMaster<->PICSlave"

#define ERROR_UART_TYPE_Retrieve	0
#define ERROR_UART_TYPE_Timeout		1
#define ERROR_UART_TYPE_Frame		2
#define ERROR_UART_TYPE_State		3
#define ERROR_UART_TYPE_Pic		4
#define ERROR_UART_TYPES		5

typedef struct  {			// obj_error
	char   *		pname;
	spinlock_t		lock;
	prop_byte		P_Errors[ERROR_UART_TYPES];
} obj_error;

// Zentrale Driverstruktur (only Pointer-SLOT's)
// ACHTUNG: Das Element *obj_control muss immer an der 1. Stelle stehen!
struct taio_drv {
	obj_control * 	Op_Control;			// Slot:  Control-Object
	aio_control	* 	ct;			// Slot:  globale Steuerung	
	obj_physic	*	Op_Physic;		// Slot:  Physic
	obj_error	*	Op_Error;		// Slot:  Error
	taio_dev_inp*	Op_Inputs[DEVICE_AIO_IN_MAX];	// Slots: Input-Objects
	taio_dev_outp*	Op_Outputs[DEVICE_AIO_OUT_MAX]; // Slots: Output-Objects
};

///	Makros: Driver
// OControl.Command - lesen
#define OBJS_DRV_RD_COMMAND(drv,cmd) 	{	\
		spin_lock_bh(&(drv)->Op_Control->lock);	\
		cmd = (drv)->Op_Control->P_Control.value[0];	\
		spin_unlock_bh(&(drv)->Op_Control->lock);}

// OControl.Command - schreiben
#define OBJS_DRV_WR_COMMAND(drv,cmd) 	{	\
		spin_lock_bh(&(drv)->Op_Control->lock);	\
		(drv)->Op_Control->P_Control.value[0] = cmd;	\
		spin_unlock_bh(&(drv)->Op_Control->lock);}

// OControl.State - lesen
#define OBJS_DRV_RD_STATE(drv,state)	{	\
		spin_lock_bh(&(drv)->Op_Control->lock);	\
		state = (drv)->Op_Control->P_Control.value[1];	\
		spin_unlock_bh(&(drv)->Op_Control->lock);}

// OControl.State - schreiben
#define OBJS_DRV_WR_STATE(drv,state)	{	\
		spin_lock_bh(&(drv)->Op_Control->lock);	\
		(drv)->Op_Control->P_Control.value[1] = state;	\
		spin_unlock_bh(&(drv)->Op_Control->lock);}

// .command & .state - lesen syncron Access
#define OBJS_DRV_RD_COMMAND_AND_STATE(drv,cmd, state)	{ 	\
		spin_lock_bh(&(drv)->Op_Control->lock);	\
		cmd = (drv)->Op_Control->P_Control.value[0];	\
		state = (drv)->Op_Control->P_Control.value[1];	\
		spin_unlock_bh(&(drv)->Op_Control->lock);}

// .command & .state - write syncron Access
#define OBJS_DRV_WR_COMMAND_AND_STATE(drv,cmd, state)	{ 	\
		spin_lock_bh(&(drv)->Op_Control->lock);	\
		(drv)->Op_Control->P_Control.value[0] = cmd;	\
		(drv)->Op_Control->P_Control.value[1] = state;	\
		spin_unlock_bh(&(drv)->Op_Control->lock);}

// Makros für Zugriff auf Driver.Error
#define OBJS_DRV_RD_ERRORS(drv,Retrieve,Timeout,Frame,State,PicToPic ){ 	\
		spin_lock_bh(&(drv)->Op_Error->lock);	\
		Retrieve= (drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Retrieve].value;	\
		Timeout	= (drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Timeout].value;	\
		Frame 	= (drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Frame].value;	\
		State 	= (drv)->Op_Error->P_Errors[ERROR_UART_TYPE_State].value;	\
		PicToPic= (drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Pic].value;	\
		spin_unlock_bh(&(drv)->Op_Error->lock);}

#define OBJS_DRV_CLR_ERRORS(drv)	{ 	\
		spin_lock_bh(&(drv)->Op_Error->lock);	\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Retrieve].value=0;	\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Timeout].value=0;	\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Frame].value=0;	\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_State].value=0;	\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Pic].value=0;	\
		spin_unlock_bh(&(drv)->Op_Error->lock);}

#define OBJS_DRV_WR_ERROR(drv,type,err)	{	\
		spin_lock_bh(&(drv)->Op_Error->lock);	\
		(drv)->Op_Error->P_Errors[type].value = err ;	\
		spin_unlock_bh(&(drv)->Op_Error->lock);	}

#define OBJS_DRV_RD_ERROR(drv,type,err)	{	\
		spin_lock_bh(&(drv)->Op_Error->lock);	\
		err = (drv)->Op_Error->P_Errors[type].value;	\
		spin_unlock_bh(&(drv)->Op_Error->lock);	}

#define OBJS_DRV_INC_ERROR(drv,type)	{	\
		spin_lock_bh(&(drv)->Op_Error->lock);	\
		(drv)->Op_Error->P_Errors[type].value = (drv)->Op_Error->P_Errors[type].value + 1;	\
		spin_unlock_bh(&(drv)->Op_Error->lock);	}

#define OBJS_DRV_RD_PIC_VERSION(drv,val)	{	\
		spin_lock_bh(&(drv)->Op_Physic->lock);	\
		val = (drv)->Op_Physic->P_PICLayer.value;	\
		spin_unlock_bh(&(drv)->Op_Physic->lock);	}

#define OBJS_DRV_WR_PIC_VERSION(drv,val)	{	\
		spin_lock_bh(&(drv)->Op_Physic->lock);	\
		(drv)->Op_Physic->P_PICLayer.value = val;	\
		spin_unlock_bh(&(drv)->Op_Physic->lock);	}

#define GET_DRV_FIRMWARE_PIC(val)	{	\
		spin_lock_bh(&drv->Op_Physic->lock);	\
		val = drv->Op_Physic->P_PICLayer.value;	\
		spin_unlock_bh(&drv->Op_Physic->lock);	}

// access = z.B. data, l2->pdata
#define OBJS_CREATE_DRV_PTR(access) struct taio_drv *drv = (struct taio_drv *)(access)
// input-device
#define OBJS_CREATE_IDEV_PTR_NULL	taio_dev_inp   * idev = NULL
#define OBJS_CREATE_IDEV_PTR(i)		idev = drv->Op_Inputs[i]
// output-device
#define OBJS_CREATE_ODEV_PTR_NULL	taio_dev_outp  * odev = NULL
#define OBJS_CREATE_ODEV_PTR(i)		odev = drv->Op_Outputs[i]


///*********************************************************
/// DRIVER-PARAMETER (ARGUMENTS) 
///*********************************************************
// DRIVER-ARG's
extern char				aio_json[JSON_LIMIT_SIZE];

///*********************************************************
///  OBJECT's	(Driver, Devices) 
///*********************************************************
extern struct taio_drv 	aio_O_Mct;							// Central Driver Object
extern taio_dev_outp 	aio_O_Outputs[DEVICE_AIO_OUT_MAX];	// Central Device Output Objects
extern taio_dev_inp  	aio_O_Inputs[DEVICE_AIO_IN_MAX];	// Central Device Input Objects

///*********************************************************
///  FUNCTION's
///*********************************************************
extern void aio_objects_generic(void);
extern ssize_t aio_objects_json(const char *);

//extern const char * ai_label_tab[];
extern const char * ai_12_label_tab[];
extern const char * ai_34_label_tab[];

#endif 
