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
#include "mct_ai8_objs.h"		// objects
#include "mct_ai8_schema.h"		// json-schema
#include "mct_ai8.h"

#define SYSFS_CREATE_DEVICE_INPUT_PTR 	struct tai8_dev_inp 	*iobj 	=	dev_get_drvdata(dev)

// BIT-MACROS setzen, loeschen, testen auf gesetzt, geloescht
#define _setbit(d,b)				(d|=1<<b)
#define _clrbit(d,b)				(d&=~(1<<b))
#define _bitset(d,b)				(((d)&(1<<b))!=0)
#define _bitclr(d,b)				((d&1<<b)==0)

///************************************************
///	OBJECT-ACCESS-MAKROS (driver specific: O_Mct)
///************************************************
// Makros für Zugriff auf Driver.Command
#define SYSFS_READ_COMMAND(cmd) 	{	\
			spin_lock_bh(&ai8_O_Mct.Op_Control->lock);	\
			cmd = ai8_O_Mct.Op_Control->P_Control.value[0];	\
			spin_unlock_bh(&ai8_O_Mct.Op_Control->lock);}

// Makros für Zugriff auf Driver.State
#define SYSFS_READ_STATE(state)		{	\
			spin_lock_bh(&ai8_O_Mct.Op_Control->lock);	\
			state = ai8_O_Mct.Op_Control->P_Control.value[1];	\
			spin_unlock_bh(&ai8_O_Mct.Op_Control->lock);}

// Makros für Zugriff auf Driver.State & Command - syncron Access
#define SYSFS_READ_COMMAND_AND_STATE(cmd, state)	{ 	\
			spin_lock_bh(&ai8_O_Mct.Op_Control->lock);	\
			cmd = ai8_O_Mct.Op_Control->P_Control.value[0];	\
			state = ai8_O_Mct.Op_Control->P_Control.value[1];	\
			spin_unlock_bh(&ai8_O_Mct.Op_Control->lock);}

///************************************************
///	OBJECT-ACCESS-MAKROS (device specific: obj->)
///************************************************
// Makros für Zugriff auf Device.Input
#define SYSFS_READ_INPUT(dst)	{	\
			spin_lock_bh(&iobj->Op_Value->lock);	\
			dst = iobj->Op_Value->P_Value.value;	\
			spin_unlock_bh(&iobj->Op_Value->lock);}

#define SYSFS_WRITE_INPUT(src)	{	\
			spin_lock_bh(&iobj->Op_Value->lock);	\
			iobj->Op_Value->P_Value.value = src;	\
			spin_unlock_bh(&iobj->Op_Value->lock);}

// Makros für Zugriff auf Device.Mask
#define SYSFS_READ_MASK(dst)	{	\
			spin_lock_bh(&iobj->Op_Mask->lock);	\
			dst = iobj->Op_Mask->P_Value.value;	\
			spin_unlock_bh(&iobj->Op_Mask->lock);}

/// ***************
/// JSON ENCODE
/// ***************
#define json_encode_O_DrvDriver() "\"%s\":\"%s\"",\
			OBJECT_DRIVER_S,	\
			driver->name

#define json_encode_O_DrvControl(command, state) "\"%s\":{\"%s\":%d,\"%s\":%d}",\
			ai8_O_Mct.Op_Control->pname,	\
			ai8_O_Mct.Op_Control->P_Control.pname[0],	\
			(command),	\
			ai8_O_Mct.Op_Control->P_Control.pname[1],	\
			(state)

#define json_encode_O_DrvPhysic() "\"%s\":{\"%s\":\"%s->%s\",\"%s\":\"%s\"}",\
			ai8_O_Mct.Op_Physic->pname,\
			ai8_O_Mct.Op_Physic->P_BUSLayer.pname,\
			ai8_O_Mct.ct->paa->dev.driver->name,\
			dev_name(&ai8_O_Mct.ct->paa->dev),\
			ai8_O_Mct.Op_Physic->P_INFO.pname,\
			ai8_O_Mct.Op_Physic->P_INFO.value

#define json_encode_O_DevIn(idx)	"\"%s\":\"%s.%d\"",	\
			PROPERTY_NAME_S,	\
			ai8_dev_in_name,	\
			_idx

#define json_encode_O_DevIn_Value(val) "\"%s\":{\"%s\":\"%s\",\"%s\":%d,\"%s\":\"%x\"}",	\
			iobj->Op_Value->pname,\
			iobj->Op_Value->P_Name.pname,\
			iobj->Op_Value->P_Name.value,\
			iobj->Op_Value->P_Size.pname,\
			iobj->Op_Value->P_Size.value,\
			iobj->Op_Value->P_Value.pname,\
			val

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int   written= 0;
	written += sprintf(buf,JSON_SCHEMA_PAA_AI8_EN);
	return strlen(buf)+1;
}


/** DRV.JSON-write (only Test) */
/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{		
	enum e_state 		_drv_state;
	enum e_state 		_drv_command;
	struct tai8_dev_inp  *iobj;
	u32					_iVal	[DEV_MCT_PAA_AI8_IN_MAX];	
	int 				 written= 0;
	unsigned int		_idx;

	///-----------------------
	/// OBJECT-WERTE (protect)
	///-----------------------
	SYSFS_READ_COMMAND_AND_STATE(_drv_command, _drv_state);	// CONTROL/STATE
	for(_idx=0 ; _idx< DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{
		iobj = ai8_O_Mct.Op_Inputs[_idx];					// INPUT Device value	
		SYSFS_READ_INPUT(_iVal[_idx]);
	}
	///-----------------------
	/// JSON-STRUCTURE
	///-----------------------
	written += sprintf(buf+written,"{\n");
	written += sprintf(buf+written,json_encode_O_DrvDriver());
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvControl(_drv_command, _drv_state));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvPhysic());
	written += sprintf(buf+written,",\n");	
	written += sprintf(buf+written,"\"%s\":[\n",OBJECT_DEVICE_Ss);
	///Input-Devices
	for (_idx=0; _idx < DEV_MCT_PAA_AI8_IN_MAX; _idx++)	{
		iobj = ai8_O_Mct.Op_Inputs[_idx];
		written += sprintf(buf+written,"{");
		written += sprintf(buf+written,json_encode_O_DevIn(_idx));
		written += sprintf(buf+written,",\n");
		written += sprintf(buf+written,json_encode_O_DevIn_Value(_iVal[_idx]));
		written += sprintf(buf+written,",\n");
		written += box_sprintf_sel(&(iobj->Op_Type->P_Type),buf + written);
		written += sprintf(buf+written,"},\n");
	}
	written += sprintf(buf+written-2,"]}\n");
	return strlen(buf)+1;
}
	
/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{		
	strcpy(buf, DRV_MCT_PAA_AI8_VERSION);
	return strlen(buf)+1;
};

/** DRV.COMMAND-read */ 
static ssize_t drv_command_R(struct device_driver *driver, char *buf)	{		
	enum e_state _command = 0;
	SYSFS_READ_COMMAND(_command);
	sprintf(buf,"%d",_command);
	return strlen(buf)+1;	
};

/** DRV.COMMAND-write */ 
// START/STOP Befehl für die Kommunikationsroutine zwischer Treiber und SPI-Gerät
// Der interne Befehlszustand wird mit dem gewünschten Befehlszustand verglichen.
// Mehrfach hintereinander gesendete Start- oder Stop-Befehle werden so verhindert.
static ssize_t drv_command_W(struct device_driver *driver, const char *buf, size_t count) {	
	enum e_state _ext_command = 0; 
	enum e_state _int_command = 0;
	_ext_command = simple_strtoul(buf, NULL, 10);
	// Driver-Start/Stop
	if((_ext_command == e_start) ||(_ext_command == e_stop) )	{	
		
		spin_lock_bh(&ai8_O_Mct.Op_Control->lock);
	   _int_command = ai8_O_Mct.Op_Control->P_Control.value[0];
		if(_int_command != _ext_command)	{
			ai8_O_Mct.Op_Control->P_Control.value[0] = _ext_command;	// externes Kommando
			ai8_O_Mct.Op_Control->P_Control.value[1] = _ext_command;	// copy also to state!
		}
		spin_unlock_bh(&ai8_O_Mct.Op_Control->lock);
	}
	return strlen(buf)+1;	
};

/** DRV.STATE-read */ 
static ssize_t drv_state_R(struct device_driver *driver, char *buf)	{		
	enum e_state _state = 0;
	SYSFS_READ_STATE(_state);
	return scnprintf(buf,STRING_U32_SIZE, "%d", _state);	
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE-INPUT
//+++++++++++++++++++++++++++++++++++++++++++
// input (R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_input_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	SYSFS_CREATE_DEVICE_INPUT_PTR;
	u32 _in = 0;
	SYSFS_READ_INPUT(_in);
	return scnprintf(buf,STRING_U32_SIZE, "%x", _in); 
};
//+++++++++++++++++++++++++++++++++++++++++++
// mode (R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_mode_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	int 				 written= 0;
	unsigned char		pos;
	SYSFS_CREATE_DEVICE_INPUT_PTR;
	pos = box_get_sel(&(iobj->Op_Type->P_Type));
	written += sprintf(buf,box_get_elem(&(iobj->Op_Type->P_Type),pos));
	return strlen(buf)+1;	
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER & DEVICE INTERFACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DRIVER_ATTR(json, 	S_IRUGO,		drv_json_R,		NULL);		// json: 	R
static DRIVER_ATTR(json_schema, S_IRUGO,	drv_json_schema_R,	NULL);	// json_schema: R
static DRIVER_ATTR(version, S_IRUGO, 		drv_version,	NULL);		// version: R
static DRIVER_ATTR(command, S_IRUGO|S_IWUSR,drv_command_R,	drv_command_W);	// Command 	R/W
static DRIVER_ATTR(state, 	S_IRUGO,		drv_state_R,	NULL);		// State:	R

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DEVICE_ATTR(input, S_IRUGO, dev_input_R, NULL);
static DEVICE_ATTR(mode,  S_IRUGO, dev_mode_R, 	NULL);

/// AI8-DRIVER-SYSFS
void ai8_sysfs_ini(void) {
};	

void ai8_sysfs_drv_add(struct device_driver *drv){
	int ret;
	ret = driver_create_file(drv, &driver_attr_json);		// SYSFS: drv.json
	ret = driver_create_file(drv, &driver_attr_json_schema);// SYSFS: drv.json_schema	
	ret = driver_create_file(drv, &driver_attr_version);	// SYSFS: drv.version 	
	ret = driver_create_file(drv, &driver_attr_command);	// SYSFS: drv.command 		
	ret = driver_create_file(drv, &driver_attr_state);		// SYSFS: drv.state 		
};

void ai8_sysfs_drv_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
};

/// AI8-INPUT-DEVICES-SYSFS
void ai8_sysfs_dev_add(struct device *dev){
	int ret;
	trace_call_dev(dev);								// Only: dev-Trace
	ret= device_create_file(dev, &dev_attr_input);	 	// SYSFS: device.input
	ret= device_create_file(dev, &dev_attr_mode);	 	// SYSFS: device.mode
};
void ai8_sysfs_dev_del(struct device *dev){
	trace_call_dev(dev);								// Only: dev-Trace
	device_remove_file(dev, &dev_attr_input);	  	 	// SYSFS: device.input
	device_remove_file(dev, &dev_attr_mode);	 		// SYSFS: device.mode
};

