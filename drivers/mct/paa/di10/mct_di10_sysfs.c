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
#include "mct_di10_objs.h"		// objects
#include "mct_di10_schema.h"	// json-schema
#include "mct_di10.h"

/// ***************
/// JSON ENCODE
/// ***************
#define json_encode_O_DrvDriver() "\"%s\":\"%s\"",\
		OBJECT_DRIVER_S,	\
		driver->name

#define json_encode_O_DrvControl(drv,cmd,state) "\"%s\":{\"%s\":%d,\"%s\":%d}",\
		(drv)->Op_Control->pname,	\
		(drv)->Op_Control->P_Control.pname[0],	\
		(cmd),	\
		(drv)->Op_Control->P_Control.pname[1],	\
		(state)


#define json_encode_O_DrvPhysic(drv) "\"%s\":{\"%s\":\"%s->%s\",\"%s\":\"%s\"}",\
		(drv)->Op_Physic->pname,\
		(drv)->Op_Physic->P_BUSLayer.pname,\
		(drv)->ct->paa->dev.driver->name,\
		dev_name(&(drv)->ct->paa->dev),\
		(drv)->Op_Physic->P_INFO.pname,\
		(drv)->Op_Physic->P_INFO.value

#define json_encode_O_InDev()	"\"%s\":\"%s.%d\"",	\
		PROPERTY_NAME_S,	\
		di10_dev_in_name,	\
		0

#define json_encode_O_InDev_Value(idx,bit)	"{\"%s\":\"%s\",\"%s\":%d,\"%s\":%01d}",	\
		PROPERTY_NAME_S,	\
		DIs[(idx)].name,	\
		PROPERTY_SIZE_S,	\
		DIs[(idx)].size,	\
		PROPERTY_VALUE_S,	\
		bit


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int   written= 0;
	written += sprintf(buf,JSON_SCHEMA_PAA_DI10_EN);
	return strlen(buf)+1;
}


/** DRV.JSON-write (only Test) */
/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{		
	struct tdi10_drv *	drv = &di10_O_Mct;
	OBJS_CREATE_IDEV_PTR;
	enum e_state state;
	enum e_state cmd;
	unsigned int value = 0;
	int written= 0;
	unsigned int idx;
	unsigned int bit;

	///-----------------------
	/// OBJECT-WERTE (protect)
	///-----------------------
	OBJS_DRV_RD_COMMAND_AND_STATE(drv,cmd,state);
	OBJS_IDEV_RD_INPUT(idev,value);	// INPUT Device  value	
	///-----------------------
	/// JSON-STRUCTURE
	///-----------------------
	written += sprintf(buf+written,"{\n");
	written += sprintf(buf+written,json_encode_O_DrvDriver());
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvControl(drv,cmd,state));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvPhysic(drv));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,"\"%s\":[",OBJECT_DEVICE_Ss);	
	///Input Device
	written += sprintf(buf+written,"{");	
	written += sprintf(buf+written,json_encode_O_InDev());
	written += sprintf(buf+written,",\n");		
	written += sprintf(buf+written, "\"%s\":[\n",OBJECT_VALUE_Ss);
	for (idx=0; idx < DEV_MCT_PAA_DI10_IN_CHAN_MAX-1; idx++)	{
		bit = (value & (1<<idx)) >> idx; 				// Bit-Filter
		written += sprintf(buf+written,json_encode_O_InDev_Value(idx,bit));
		written += sprintf(buf+written,",\n");		
	}
	bit = (value & (1<<idx)) >> idx; 				// Bit-Filter
	written += sprintf(buf+written,json_encode_O_InDev_Value(idx,bit));
	written += sprintf(buf+written,"\n]}]}\n");
	return strlen(buf)+1;
}
	
/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{		
	strcpy(buf, DRV_MCT_PAA_DI10_VERSION);
	return strlen(buf)+1;
};

/** DRV.COMMAND-read */ 
static ssize_t drv_command_R(struct device_driver *driver, char *buf)	{		
	struct tdi10_drv *drv = &di10_O_Mct;
	enum e_state cmd = 0;
	OBJS_DRV_RD_COMMAND(drv, cmd);
	sprintf(buf,"%d",cmd);
	return strlen(buf)+1;
};

/** DRV.COMMAND-write */ 
// START/STOP Befehl für die Kommunikationsroutine zwischer Treiber und SPI-Gerät
// Der interne Befehlszustand wird mit dem gewünschten Befehlszustand verglichen.
// Mehrfach hintereinander gesendete Start- oder Stop-Befehle werden so verhindert.
static ssize_t drv_command_W(struct device_driver *driver, const char *buf, size_t count) {	
	struct tdi10_drv *drv = &di10_O_Mct;
	enum e_state ext_cmd = 0; 
	enum e_state int_cmd = 0;

	ext_cmd = simple_strtoul(buf, NULL, 10);	
	// Driver-Start/Stop/Init
	if((ext_cmd == e_start)||(ext_cmd == e_stop))	{	
		spin_lock_bh(&drv->Op_Control->lock);
	    int_cmd = drv->Op_Control->P_Control.value[0];
		if(int_cmd != ext_cmd)	{
			drv->Op_Control->P_Control.value[0] = ext_cmd;	// externes Kommando
			drv->Op_Control->P_Control.value[1] = ext_cmd;	// copy also to state!
		}
		spin_unlock_bh(&drv->Op_Control->lock);
	}
	return strlen(buf)+1;	
};

/** DRV.STATE-read */ 
static ssize_t drv_state_R(struct device_driver *driver, char *buf)	{		
	struct tdi10_drv *drv = &di10_O_Mct;
	enum e_state state;
	OBJS_DRV_RD_STATE(drv,state);
	return scnprintf(buf,STRING_U32_SIZE, "%d", state);	
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE INPUT
//+++++++++++++++++++++++++++++++++++++++++++
// input (R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_input_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	struct tdi10_dev_inp *idev = dev_get_drvdata(dev);
	u32 value = 0;
	OBJS_IDEV_RD_INPUT(idev,value);
	return scnprintf(buf,STRING_U32_SIZE, "%d", value); 

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
static DEVICE_ATTR(input,  S_IRUGO, 		dev_input_R, 	NULL);

/// DI10-DRIVER-SYSFS
void di10_sysfs_ini(void) {
};	

void di10_sysfs_drv_add(struct device_driver *drv){
	int ret;
	ret = driver_create_file(drv, &driver_attr_json);		// SYSFS: drv.json
	ret = driver_create_file(drv, &driver_attr_json_schema);// SYSFS: drv.json_schema	
	ret = driver_create_file(drv, &driver_attr_version);	// SYSFS: drv.version 	
	ret = driver_create_file(drv, &driver_attr_command);	// SYSFS: drv.command 		
	ret = driver_create_file(drv, &driver_attr_state);		// SYSFS: drv.state 		
};

void di10_sysfs_drv_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
};

/// DI-DEVICES-SYSFS
void di10_sysfs_dev_add(struct device *dev){
	int ret;
	trace_call_dev(dev);								// Only: dev-Trace
	ret=device_create_file(dev, &dev_attr_input);	 	// SYSFS: device.input
};
void di10_sysfs_dev_del(struct device *dev){
	trace_call_dev(dev);								// Only: dev-Trace
	device_remove_file(dev, &dev_attr_input);	 		// SYSFS: device.input
};

