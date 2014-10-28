/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011

 	Description:

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../mct_debug.h"
#include "mct_spi_dio_objs.h"		// objects
#include "mct_spi_dio_schema.h"		// json-schema
#include "mct_dio.h"

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

#define json_encode_O_DrvPhysic(drv) "\"%s\":{\"%s\":\"%s->%s\"}",\
			(drv)->Op_Physic->pname,\
			(drv)->Op_Physic->P_Layer0.pname,\
			dev_name(&(drv)->ct->spi->dev),\
			(drv)->ct->spi->dev.driver->name

#define json_encode_O_OutDev()	"\"%s\":\"%s.%d\"",	\
			PROPERTY_NAME_S,	\
			DEVICE_OUT_NAME,	\
			DEVICE_OUT_0

#define json_encode_O_OutDev_Init(dev,ini) "\"%s\":{\"%s\":%02d,\"%s\":%01d,\"%s\":%02d}",	\
			(dev)->Op_Init->pname,	\
			(dev)->Op_Init->P_Value.pname,	\
			(ini),	\
			(dev)->Op_Init->P_Size.pname,	\
			(dev)->Op_Init->P_Size.value,	\
			(dev)->Op_Init->P_Link.pname,	\
			(dev)->Op_Init->P_Link.value

#define json_encode_O_OutDev_Value(dev,idx,bit)	"{\"%s\":\"%s\",\"%s\":%d,\"%s\":%01d}",	\
			(dev)->Op_Value->P_Name.pname,\
			DOs[(idx)].name,	\
			(dev)->Op_Value->P_Size.pname,\
			DOs[(idx)].size,	\
			(dev)->Op_Value->P_Value.pname,\
			bit

#define json_encode_O_InDev()	"\"%s\":\"%s.%d\"",	\
			PROPERTY_NAME_S,	\
			DEVICE_IN_NAME,	\
			DEVICE_IN_0

#define json_encode_O_InDev_Value(dev,idx,bit)	"{\"%s\":\"%s\",\"%s\":%d,\"%s\":%01d}",	\
			(dev)->Op_Value->P_Name.pname,\
			DIs[(idx)].name,	\
			(dev)->Op_Value->P_Size.pname,\
			DIs[(idx)].size,	\
			(dev)->Op_Value->P_Value.pname,\
			bit

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int 				 written= 0;
	written += sprintf(buf,JSON_SCHEMA_SPI_DIO_EN);
	return strlen(buf)+1;
}

/** DRV.JSON-write (only Test) */

/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{		
	struct tdio_drv *drv = &dio_O_Mct;
	OBJS_CREATE_ODEV_PTR;
	OBJS_CREATE_IDEV_PTR;

	enum e_state 		state;
	enum e_state 		cmd;
	unchar				out = 0;
	unsigned int		in = 0;
	unchar				ini	= 0;
	int 				written= 0;
	unsigned int		_idx;
	unsigned int		_bit;

	///**********************
	/// OBJECT-WERTE (protect)
	///***********************
	OBJS_DRV_RD_COMMAND_AND_STATE(drv,cmd,state);
	OBJS_ODEV_RD_OUTPUT(odev,out);		// output lesen
	OBJS_ODEV_RD_INIT(odev,ini);		// init lesen
	OBJS_IDEV_RD_INPUT(idev,in);		// input lesen
		
	///-----------------------
	/// JSON-STRUCTURE
	///-----------------------
	written += sprintf(buf+written,"{\n");
	written += sprintf(buf+written,json_encode_O_DrvDriver());
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvControl(drv,cmd, state));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvPhysic(drv));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,"\"%s\":[\n",OBJECT_DEVICE_Ss);
		
	///Output-Device
	written += sprintf(buf+written,"{");
	written += sprintf(buf+written,json_encode_O_OutDev());
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_OutDev_Init(odev,ini));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written, "\"%s\":[\n",OBJECT_VALUE_Ss);
	for (_idx=0; _idx < OUTPUT_ELEMENTS-1; _idx++)	{
		_bit = (out & (1<<_idx)) >> _idx; 			// Bit-Filter
		written += sprintf(buf+written,json_encode_O_OutDev_Value(odev,_idx,_bit));
		written += sprintf(buf+written,",\n");
	}
	_bit = (out & (1<<_idx)) >> _idx; 				// Bit-Filter
	written += sprintf(buf+written,json_encode_O_OutDev_Value(odev,_idx,_bit));	
	written += sprintf(buf+written,"]},\n");
	
	///Input Device
	written += sprintf(buf+written,"{");	
	written += sprintf(buf+written,json_encode_O_InDev());
	written += sprintf(buf+written,",\n");		
	written += sprintf(buf+written, "\"%s\":[\n",OBJECT_VALUE_Ss);
	for (_idx=0; _idx < INPUT_ELEMENTS-1; _idx++)	{
		_bit = (in & (1<<_idx)) >> _idx; 				// Bit-Filter
		written += sprintf(buf+written,json_encode_O_InDev_Value(idev,_idx,_bit));
		written += sprintf(buf+written,",\n");		
	}
	_bit = (in & (1<<_idx)) >> _idx; 					// Bit-Filter
	written += sprintf(buf+written,json_encode_O_InDev_Value(idev,_idx,_bit));
	written += sprintf(buf+written,"]}\n");	
	written += sprintf(buf+written,"]}\n");
	return strlen(buf)+1;
}
	
/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{		
	strcpy(buf, DRIVER_VERSION_STRING);
	return strlen(buf)+1;
};

/** DRV.COMMAND-read */ 
static ssize_t drv_command_R(struct device_driver *driver, char *buf)	{		
	struct tdio_drv *drv = &dio_O_Mct;
	enum e_state cmd = 0;
	OBJS_DRV_RD_COMMAND(drv,cmd);
	sprintf(buf,"%d",cmd);
	return strlen(buf)+1;
};

/** DRV.COMMAND-write */ 
// START/STOP Befehl für die Kommunikationsroutine zwischer Treiber und SPI-Gerät
// Der interne Befehlszustand wird mit dem gewünschten Befehlszustand verglichen.
// Mehrfach hintereinander gesendete Start- oder Stop-Befehle werden so verhindert.
static ssize_t drv_command_W(struct device_driver *driver, const char *buf, size_t count)	{	
	struct tdio_drv *drv	= &dio_O_Mct;
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
	struct tdio_drv *drv = &dio_O_Mct;
	enum e_state state = 0;
	OBJS_DRV_RD_STATE(drv,state);
	return scnprintf(buf,STRING_U32_SIZE, "%d", state);	
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
	struct tdio_dev_outp *odev = dev_get_drvdata(dev);
	u32 out = 0;
	OBJS_ODEV_RD_OUTPUT(odev,out);
	return scnprintf(buf,STRING_U32_SIZE, "%d", out);
};

// Write 
static ssize_t dev_output_W(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {	
	struct tdio_dev_outp *odev = dev_get_drvdata(dev);
	unchar out	= 0;
	unchar prot;
	unchar new 	= simple_strtoul(buf, NULL, 10);	// neuer,externer Eingabewert 
	OBJS_ODEV_RD_OUTPUT(odev,out);					// aktuelle Ausgänge lesen
	if (count > 0) {
		prot = new >> OUT8_RELS_SHIFTER_PROTECT;	// protect Maske
		new &= ~prot;
		out &= prot;								// clear unprotected bits
		out |= new;
		DEBUG(MCT_DEBUG_LEVEL4,"SET %d\n",out);
		OBJS_ODEV_WR_OUTPUT_4Bit(odev,out);		
		return count;
	}
	return -EINVAL;
};

//+++++++++++++++++++++++++++++++++++++++++++
// mask (X,R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_init_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	struct tdio_dev_outp *odev = dev_get_drvdata(dev);	
	unchar ini = 0;
	OBJS_ODEV_RD_INIT(odev,ini);
	return scnprintf(buf,STRING_U32_SIZE, "%d", ini); 
};

/// SYSFS-DEVICE INPUT
//+++++++++++++++++++++++++++++++++++++++++++
// input (R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_input_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	struct tdio_dev_inp *idev = dev_get_drvdata(dev);
	u32 in = 0;
	OBJS_IDEV_RD_INPUT(idev,in);
	return scnprintf(buf,STRING_U32_SIZE, "%d", in); 
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
static DEVICE_ATTR(output, S_IRUGO|S_IWUGO, dev_output_R, 	dev_output_W);
static DEVICE_ATTR(init,   S_IRUGO, 		dev_init_R, 	NULL);
static DEVICE_ATTR(input,  S_IRUGO, 		dev_input_R, 	NULL);

/// DIO-DRIVER-SYSFS
void dio_sysfs_ini(void) {
};	
void dio_sysfs_drv_add(struct device_driver *drv){
	int ret;
	ret = driver_create_file(drv, &driver_attr_json);		// SYSFS: drv.json
	ret = driver_create_file(drv, &driver_attr_json_schema);// SYSFS: drv.json_schema	
	ret = driver_create_file(drv, &driver_attr_version);	// SYSFS: drv.version 	
	ret = driver_create_file(drv, &driver_attr_command);	// SYSFS: drv.command 		
	ret = driver_create_file(drv, &driver_attr_state);		// SYSFS: drv.state 		
};
void dio_sysfs_drv_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
};

/// DO-DEVICES-SYSFS
void dio_do_sysfs_dev_add(struct device *dev){
	int ret;
	trace_call_dev(dev);								// Only: dev-Trace
	ret= device_create_file(dev, &dev_attr_output);	 	// SYSFS: device.output
	ret= device_create_file(dev, &dev_attr_init);	 	// SYSFS: device.init
};
void dio_do_sysfs_dev_del(struct device *dev){
	trace_call_dev(dev);								// Only: dev-Trace
	device_remove_file(dev, &dev_attr_output);	  	 	// SYSFS: device.output
	device_remove_file(dev, &dev_attr_init);	   		// SYSFS: device.init
};

/// DI-DEVICES-SYSFS
void dio_di_sysfs_dev_add(struct device *dev){
	int ret;
	trace_call_dev(dev);								// Only: dev-Trace
	ret=device_create_file(dev, &dev_attr_input);	 	// SYSFS: device.input
};
void dio_di_sysfs_dev_del(struct device *dev){
	trace_call_dev(dev);								// Only: dev-Trace
	device_remove_file(dev, &dev_attr_input);	 		// SYSFS: device.input
};

