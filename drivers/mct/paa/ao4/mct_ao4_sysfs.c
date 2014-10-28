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
#include "mct_ao4_objs.h"		// objects
#include "mct_ao4_schema.h"		// json-schema
#include "mct_ao4.h"

#define SYSFS_CREATE_DEVICE_OUTPUT_PTR 	struct tao4_dev_outp 	*oobj 	=	dev_get_drvdata(dev)

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
			spin_lock_bh(&ao4_O_Mct.Op_Control->lock);	\
			cmd = ao4_O_Mct.Op_Control->P_Control.value[0];	\
			spin_unlock_bh(&ao4_O_Mct.Op_Control->lock);}

// Makros für Zugriff auf Driver.State
#define SYSFS_READ_STATE(state)		{	\
			spin_lock_bh(&ao4_O_Mct.Op_Control->lock);	\
			state = ao4_O_Mct.Op_Control->P_Control.value[1];	\
			spin_unlock_bh(&ao4_O_Mct.Op_Control->lock);}

// Makros für Zugriff auf Driver.State & Command - syncron Access
#define SYSFS_READ_COMMAND_AND_STATE(cmd, state)	{ 	\
			spin_lock_bh(&ao4_O_Mct.Op_Control->lock);	\
			cmd = ao4_O_Mct.Op_Control->P_Control.value[0];	\
			state = ao4_O_Mct.Op_Control->P_Control.value[1];	\
			spin_unlock_bh(&ao4_O_Mct.Op_Control->lock);}

///************************************************
///	OBJECT-ACCESS-MAKROS (device specific: obj->)
///************************************************
// Makros für Zugriff auf Device.Output
#define SYSFS_READ_OUTPUT(dst)	{	\
			spin_lock_bh(&oobj->Op_Value->lock);	\
			dst = oobj->Op_Value->P_Value.value;	\
			spin_unlock_bh(&oobj->Op_Value->lock);}


#define SYSFS_WRITE_OUTPUT(src)	{	\
			spin_lock_bh(&oobj->Op_Value->lock);	\
			oobj->Op_Value->P_Value.value = src;	\
			spin_unlock_bh(&oobj->Op_Value->lock);}

// Makros für Zugriff auf Device.Mask
#define SYSFS_READ_MASK(dst)	{	\
			spin_lock_bh(&oobj->Op_Mask->lock);	\
			dst = oobj->Op_Mask->P_Value.value;	\
			spin_unlock_bh(&oobj->Op_Mask->lock);}

/// ***************
/// JSON ENCODE
/// ***************
#define json_encode_O_DrvDriver() "\"%s\":\"%s\"",\
			OBJECT_DRIVER_S,	\
			driver->name

#define json_encode_O_DrvControl(command, state) "\"%s\":{\"%s\":%d,\"%s\":%d}",\
			ao4_O_Mct.Op_Control->pname,	\
			ao4_O_Mct.Op_Control->P_Control.pname[0],	\
			(command),	\
			ao4_O_Mct.Op_Control->P_Control.pname[1],	\
			(state)

#define json_encode_O_DrvPhysic() "\"%s\":{\"%s\":\"%s->%s\",\"%s\":\"%s\"}",\
			ao4_O_Mct.Op_Physic->pname,\
			ao4_O_Mct.Op_Physic->P_BUSLayer.pname,\
			ao4_O_Mct.ct->paa->dev.driver->name,\
			dev_name(&ao4_O_Mct.ct->paa->dev),\
			ao4_O_Mct.Op_Physic->P_INFO.pname,\
			ao4_O_Mct.Op_Physic->P_INFO.value


#define json_encode_O_DevOut(idx)	"\"%s\":\"%s.%d\"",	\
			PROPERTY_NAME_S,	\
			ao4_dev_out_name,	\
			_idx

#define json_encode_O_DevOut_Value(val) "\"%s\":{\"%s\":\"%s\",\"%s\":%d,\"%s\":%d}",	\
			oobj->Op_Value->pname,\
			oobj->Op_Value->P_Name.pname,\
			oobj->Op_Value->P_Name.value,\
			oobj->Op_Value->P_Size.pname,\
			oobj->Op_Value->P_Size.value,\
			oobj->Op_Value->P_Value.pname,\
			val

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int   written= 0;
	written += sprintf(buf,JSON_SCHEMA_PAA_AO4_EN);
	return strlen(buf)+1;
}

/** DRV.JSON-write (only Test) */
/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{		
	enum e_state 		_drv_state;
	enum e_state 		_drv_command;
	struct tao4_dev_outp  *oobj;
	u16					_oVal	[DEV_MCT_PAA_AO4_OUT_MAX];	
	int 				 written= 0;
	unsigned int		_idx;

	memset(_oVal,0, sizeof(_oVal));

	///-----------------------
	/// OBJECT-WERTE (protect)
	///-----------------------
	SYSFS_READ_COMMAND_AND_STATE(_drv_command, _drv_state);	// CONTROL/STATE
	for(_idx=0 ; _idx< DEV_MCT_PAA_AO4_OUT_MAX; _idx++)	{
		oobj = ao4_O_Mct.Op_Outputs[_idx];					// OUTPUT Device value	
		SYSFS_READ_OUTPUT(_oVal[_idx]);
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
	///Output-Devices
	for (_idx=0; _idx < DEV_MCT_PAA_AO4_OUT_MAX; _idx++)	{
		oobj = ao4_O_Mct.Op_Outputs[_idx];
		written += sprintf(buf+written,"{");
		written += sprintf(buf+written,json_encode_O_DevOut(_idx));
		written += sprintf(buf+written,",\n");
		written += sprintf(buf+written,json_encode_O_DevOut_Value(_oVal[_idx]));
		written += sprintf(buf+written,"},\n");
	}
	written += sprintf(buf+written-2,"]}\n");
	return strlen(buf)+1;
}
	
/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{		
	strcpy(buf, DRV_MCT_PAA_AO4_VERSION);
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
		
		spin_lock_bh(&ao4_O_Mct.Op_Control->lock);
	   _int_command = ao4_O_Mct.Op_Control->P_Control.value[0];
		if(_int_command != _ext_command)	{
			ao4_O_Mct.Op_Control->P_Control.value[0] = _ext_command;	// externes Kommando
			ao4_O_Mct.Op_Control->P_Control.value[1] = _ext_command;	// copy also to state!
		}
		spin_unlock_bh(&ao4_O_Mct.Op_Control->lock);
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
/// SYSFS-DEVICE-OUTPUT
//+++++++++++++++++++++++++++++++++++++++++++
// output (X,R/W)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_output_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	SYSFS_CREATE_DEVICE_OUTPUT_PTR;
	int _out = 0;
	SYSFS_READ_OUTPUT(_out);
	return scnprintf(buf,STRING_U32_SIZE, "%d", _out);
};

// Write 
static ssize_t dev_output_W(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {	
	SYSFS_CREATE_DEVICE_OUTPUT_PTR;
	int _out 	= simple_strtoul(buf, NULL, 0);	// neuer,externer Eingabewert 
	
	if(_out < 0)					// Untere Grenze 
		_out = 0;		
	else if (_out > SIZE_MAX_10bit)	// Obere Grenze
		_out = SIZE_MAX_10bit;

	SYSFS_WRITE_OUTPUT(_out);
	return strlen(buf)+1;
}	


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
static DEVICE_ATTR(output, S_IRUGO|S_IWUGO, dev_output_R, 	dev_output_W);


/// DIO42-DRIVER-SYSFS
void ao4_sysfs_ini(void) {
};	

void ao4_sysfs_drv_add(struct device_driver *drv){
	int ret;
	ret = driver_create_file(drv, &driver_attr_json);		// SYSFS: drv.json
	ret = driver_create_file(drv, &driver_attr_json_schema);// SYSFS: drv.json_schema	
	ret = driver_create_file(drv, &driver_attr_version);	// SYSFS: drv.version 	
	ret = driver_create_file(drv, &driver_attr_command);	// SYSFS: drv.command 		
	ret = driver_create_file(drv, &driver_attr_state);		// SYSFS: drv.state 		
};

void ao4_sysfs_drv_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
};

/// DIO42-OUTPUT-DEVICES-SYSFS
void ao4_sysfs_dev_add(struct device *dev){
	int ret;
	trace_call_dev(dev);								// Only: dev-Trace
	ret= device_create_file(dev, &dev_attr_output);	 	// SYSFS: device.output
};
void ao4_sysfs_dev_del(struct device *dev){
	trace_call_dev(dev);								// Only: dev-Trace
	device_remove_file(dev, &dev_attr_output);	  	 	// SYSFS: device.output
};

