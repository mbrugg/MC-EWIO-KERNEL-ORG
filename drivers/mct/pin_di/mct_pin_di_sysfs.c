/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		17.10.2011
 	
 	Description:

 *******************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "mct_pin_di_objs.h"		// objects	
#include "mct_pin_di_schema.h"		// json-schema
#include "mct_di.h"

extern char * di_dev_in_name;

/// ***************
/// JSON ENCODE
/// ***************
#define json_encode_O_DrvDriver(driver) "\"%s\":\"%s\"",\
			OBJECT_DRIVER_S,	\
			(driver)->name

#define json_encode_O_DrvControl(drv,cmd,state) "\"%s\":{\"%s\":%d,\"%s\":%d}",\
			(drv)->Op_Control->pname,	\
			(drv)->Op_Control->P_Control.pname[0],	\
			(cmd),	\
			(drv)->Op_Control->P_Control.pname[1],	\
			(state)

#define json_encode_O_DrvPhysic(drv) "\"%s\":{\"%s\":%d,\"%s\":%d}",\
			(drv)->Op_Physic->pname,\
			(drv)->Op_Physic->P_Pin.pname,\
			(drv)->Op_Physic->P_Pin.value,\
			(drv)->Op_Physic->P_Tri.pname,\
			(drv)->Op_Physic->P_Tri.value

#define json_encode_O_Dev()	"\"%s\":\"%s\"",	\
			PROPERTY_NAME_S,	\
			di_dev_in_name

#define json_encode_O_DevMode(dev)	"\"%s\":{\"%s\":\"%s\",\"%s\":%d}",\
			(dev)->Op_Mode->pname,	\
			(dev)->Op_Mode->P_Name.pname,\
			(dev)->Op_Mode->P_Name.value,\
			(dev)->Op_Mode->P_Value.pname,\
			(dev)->Op_Mode->P_Value.value

#define json_encode_O_DevValue(dev,val)	"\"%s\":{\"%s\":\"%s\",\"%s\":%d,\"%s\":\"%s\"}",\
			(dev)->Op_Value->pname,	\
			(dev)->Op_Value->P_Name.pname,\
			(dev)->Op_Value->P_Name.value,\
			(dev)->Op_Value->P_Size.pname,\
			(dev)->Op_Value->P_Size.value,\
			(dev)->Op_Value->P_Value.pname,\
			val

#define json_encode_O_DevFreeze(dev,val)	"\"%s\":{\"%s\":%d,\"%s\":%d,\"%s\":\"%s\"}",\
			(dev)->Op_Freeze->pname,	\
			(dev)->Op_Freeze->P_Init.pname,\
			(dev)->Op_Freeze->P_Init.value,\
			(dev)->Op_Freeze->P_Size.pname,\
			(dev)->Op_Freeze->P_Size.value,\
			(dev)->Op_Freeze->P_Value.pname,\
			val

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++

/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int written= 0;
	written += sprintf(buf,JSON_SCHEMA_PIN_DI_EN);
	return strlen(buf)+1;
}

/** DRV.JSON-write (only Test) */

/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{		
	struct tdi_drv *drv	= &di_O_Mct;
	OBJS_CREATE_IDEV_PTR;
	unsigned long long 	u64_val;
	unsigned long long 	u64_fre;
	char 				s64_val[STRING_LLD_SIZE];	
	char 				s64_fre[STRING_LLD_SIZE];	
	enum e_state 		state;
	enum e_state 		cmd;
	int 				_size = 0;
	int 				 written= 0;
	u8					mode =0;
	///**********************
	/// OBJECT-WERTE (protect)
	///***********************
	OBJS_DRV_RD_COMMAND_AND_STATE(drv,cmd,state);
	OBJS_IDEV_RD_VALUE(idev,u64_val);
	OBJS_IDEV_RD_FREEZE(idev,u64_fre);
	OBJS_IDEV_RD_MODE(idev,mode);
	_size = scnprintf(s64_val,STRING_LLD_SIZE, "%llu", u64_val);
	_size = scnprintf(s64_fre,STRING_LLD_SIZE, "%llu", u64_fre);	

	///*********************
	/// JSON-STRUCTURE
	///*********************
	written += sprintf(buf+written,"{\n");
	written += sprintf(buf+written,json_encode_O_DrvDriver(driver));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvControl(drv,cmd, state));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvPhysic(drv));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,"\"%s\":{\n",OBJECT_DEVICE_S);	
	written += sprintf(buf+written,json_encode_O_Dev());
	written += sprintf(buf+written,",\n");	
	written += sprintf(buf+written,json_encode_O_DevMode(idev));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DevValue(idev,s64_val));
	if(mode == MODE_S0)	{
		written += sprintf(buf+written,",\n");
		written += sprintf(buf+written,json_encode_O_DevFreeze(idev,s64_fre));
	}
	written += sprintf(buf+written,"\n}}\n");
	return strlen(buf)+1;	
};

/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{		
	strcpy(buf, DRIVER_VERSION_STRING);
	return strlen(buf)+1;
};

/** DRV.COMMAND-read */ 
static ssize_t drv_command_R(struct device_driver *driver, char *buf)	{		
	struct tdi_drv *drv	= &di_O_Mct;
	enum e_state cmd = 0;
	OBJS_DRV_RD_COMMAND(drv,cmd);
	sprintf(buf,"%d",cmd);
	return strlen(buf)+1;	
};

/** DRV.COMMAND-write */ 
// START/STOP Befehl für die Kommunikationsroutine zwischer Treiber und SPI-Gerät
// Der interne Befehlszustand wird mit dem gewünschten Befehlszustand verglichen.
// Mehrfach hintereinander gesendete Start- oder Stop-Befehle werden so verhindert.
static ssize_t drv_command_W(struct device_driver *driver, const char *buf, size_t count) {	
	struct tdi_drv *drv	= &di_O_Mct;
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
	struct tdi_drv *drv	= &di_O_Mct;
	enum e_state state = 0;
	OBJS_DRV_RD_STATE(drv,state);
	return scnprintf(buf,STRING_U32_SIZE, "%d", state);	
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++
// input (X,R) 
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_input_R(struct device *device, struct device_attribute *attr, char *buf) {
	struct tdi_dev_inp *idev = dev_get_drvdata(device);
	unsigned long long u64 = 0;
	OBJS_IDEV_RD_VALUE(idev,u64);
	scnprintf(buf,STRING_LLD_SIZE, "%llu", u64); // anzahl zeichen, ohne die letzte '0'
	return strlen(buf)+1; 
};
//+++++++++++++++++++++++++++++++++++++++++++
// freeze (X,R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_freeze_R(struct device *device, struct device_attribute *attr, char *buf) {	
	struct tdi_dev_inp *idev = dev_get_drvdata(device);
	unsigned long long u64 = 0;
	OBJS_IDEV_RD_FREEZE(idev,u64);
	scnprintf(buf,STRING_LLD_SIZE, "%llu", u64); // anzahl zeichen, ohne die letzte '0'
	return strlen(buf)+1; 
};

//+++++++++++++++++++++++++++++++++++++++++++
// freeze (X,R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_init_R(struct device *device, struct device_attribute *attr, char *buf)	{
	struct tdi_dev_inp *idev = dev_get_drvdata(device);
	bool flag;
	OBJS_IDEV_RD_INIT(idev,flag);
	return scnprintf(buf,2,"%d",flag); 
};

//Write nur init=0 zugelassen!
static ssize_t dev_init_W(struct device *device, struct device_attribute *attr, const char *buf, size_t count) {	
	struct tdi_dev_inp *idev = dev_get_drvdata(device);
	bool init 	= ((bool)(simple_strtoul(buf, NULL, 2))) & 0x01;  // 0 oder 1 zulassen!
	if(init == 0)	{
		OBJS_IDEV_WR_INIT_EXPORT(idev,1);	// InitExport aktualisieren!
		OBJS_IDEV_WR_INIT(idev,0) 			// Initialisierung beendet!
	}
	return count; 
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER & DEVICE INTERFACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DRIVER_ATTR(json, 		S_IRUGO,drv_json_R, NULL);			// json: 		R
static DRIVER_ATTR(json_schema, S_IRUGO, drv_json_schema_R,	NULL);	// json_schema: R
static DRIVER_ATTR(version, 	S_IRUGO, drv_version,	NULL);		// version: 	R
static DRIVER_ATTR(command, 	S_IRUGO|S_IWUSR,drv_command_R,	drv_command_W);	// Command 	R/W
static DRIVER_ATTR(state, 		S_IRUGO,drv_state_R,	NULL);		// State:		R

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DEVICE_ATTR(input, 	S_IRUGO,	dev_input_R,  NULL);
static DEVICE_ATTR(freeze, 	S_IRUGO,	dev_freeze_R, NULL);
static DEVICE_ATTR(init, 	S_IRUGO|S_IWUGO,	dev_init_R, dev_init_W);

void di_sysfs_ini(void) {};	

void di_sysfs_drv_add(struct device_driver *drv){
	int ret;
	ret = driver_create_file(drv, &driver_attr_json);		// SYSFS: drv.json
	ret = driver_create_file(drv, &driver_attr_json_schema);// SYSFS: drv.json_schema	
	ret = driver_create_file(drv, &driver_attr_version);	// SYSFS: drv.version 	
	ret = driver_create_file(drv, &driver_attr_command);	// SYSFS: drv.command 		
	ret = driver_create_file(drv, &driver_attr_state);		// SYSFS: drv.state 		

};

void di_sysfs_drv_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
};

void di_sysfs_dev_add(struct device *dev){
	int ret;
	trace_call_dev(dev);									// Only: dev-Trace
	ret=device_create_file(dev, &dev_attr_input);			// SYSFS: device.X/input 
	ret=device_create_file(dev, &dev_attr_freeze);			// SYSFS: device.X/freeze 
	ret=device_create_file(dev, &dev_attr_init);			// SYSFS: device.X/init 
};

void di_sysfs_dev_del(struct device *dev){
	trace_call_dev(dev);									// Only: dev-Trace
	device_remove_file(dev, &dev_attr_input);
	device_remove_file(dev, &dev_attr_freeze);
	device_remove_file(dev, &dev_attr_init);
};
