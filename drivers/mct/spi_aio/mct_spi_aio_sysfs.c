/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../mct_debug.h"
#include "mct_spi_aio_objs.h"
#include "mct_spi_aio_schema.h"	// json-schema
#include "mct_aio.h"

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

#define json_encode_O_DrvPhysic(drv,bcd) "\"%s\":{\"%s\":\"%s->%s\",\"%s\":\"%d.%d\"}",\
		(drv)->Op_Physic->pname,\
		(drv)->Op_Physic->P_SPILayer.pname,\
		dev_name(&(drv)->ct->spi->dev),\
		(drv)->ct->spi->dev.driver->name,\
		(drv)->Op_Physic->P_PICLayer.pname,\
		(bcd >> 4),\
		(bcd & 0x0F)

#define json_encode_O_DrvError(drv,ERetrieve, ETimeout, EFrame, EState, EPicToPic ) "\"%s\":{\"%s\":\"0x%02x\",\"%s\":\"0x%02x\",\"%s\":\"0x%02x\",\"%s\":\"0x%02x\",\"%s\":\"0x%02x\"}",\
		(drv)->Op_Error->pname,\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Retrieve].pname,\
		ERetrieve,\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Timeout].pname,\
		ETimeout,\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Frame].pname,\
		EFrame,\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_State].pname,\
		EState,\
		(drv)->Op_Error->P_Errors[ERROR_UART_TYPE_Pic].pname,\
		EPicToPic

#define json_encode_O_DevIn(idx)	"\"%s\":\"%s.%d\"",	\
		PROPERTY_NAME_S,	\
		DEVICE_IN_NAME,	\
		idx

#define json_encode_O_DevIn_Value(idev,val) "\"%s\":{\"%s\":\"%s\",\"%s\":%d,\"%s\":\"%x\"}",	\
		(idev)->Op_Value->pname,\
		(idev)->Op_Value->P_Name.pname,\
		(idev)->Op_Value->P_Name.value,\
		(idev)->Op_Value->P_Size.pname,\
		(idev)->Op_Value->P_Size.value,\
		(idev)->Op_Value->P_Value.pname,\
		val

#define json_encode_O_DevIn_Value_Off(idev,val,off) "\"%s\":{\"%s\":\"%s\",\"%s\":%d,\"%s\":\"%x\",\"%s\":\"%x\"}",	\
		(idev)->Op_Value->pname,\
		(idev)->Op_Value->P_Name.pname,\
		(idev)->Op_Value->P_Name.value,\
		(idev)->Op_Value->P_Size.pname,\
		(idev)->Op_Value->P_Size.value,\
		(idev)->Op_Value->P_Value.pname,\
		val,\
		(idev)->Op_Value->P_Offset.pname,\
		off

#define json_encode_O_DevOut(name,idx)	"\"%s\":\"%s.%d\"",	\
		PROPERTY_NAME_S,	\
		name,	\
		idx

#define json_encode_O_DevOut_Value(odev,val) "\"%s\":{\"%s\":\"%s\",\"%s\":%d,\"%s\":%d}",	\
		(odev)->Op_Value->pname,\
		(odev)->Op_Value->P_Name.pname,\
		(odev)->Op_Value->P_Name.value,\
		(odev)->Op_Value->P_Size.pname,\
		(odev)->Op_Value->P_Size.value,\
		(odev)->Op_Value->P_Value.pname,\
		val

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int written= 0;
	written += sprintf(buf,JSON_SCHEMA_SPI_AIO_EN);
	return strlen(buf)+1;
}

/** DRV.JSON-write (only Test) */
/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{			
	struct taio_drv *drv = &aio_O_Mct;
	OBJS_CREATE_IDEV_PTR_NULL;
	OBJS_CREATE_ODEV_PTR_NULL;
	enum e_state state;
	enum e_state cmd;
	u8	version;
	u32	iVal[DEVICE_AIO_IN_MAX];
	u32 iValO[DEVICE_AIO_IN_12_MAX];
	u16	oVal[DEVICE_AIO_OUT_MAX];
	int written= 0;
	unsigned int idx;
	u8	ERetrieve,ETimeout,EFrame,EState,EPicToPic;	// errors

	memset(iVal,0, sizeof(iVal));
	memset(iValO,0, sizeof(iValO));
	memset(oVal,0, sizeof(oVal));

	///**********************
	/// OBJECT-WERTE (protect)
	///***********************
	OBJS_DRV_RD_COMMAND_AND_STATE(drv,cmd,state);
	OBJS_DRV_RD_PIC_VERSION(drv,version);					// PIC-Version
	OBJS_DRV_RD_ERRORS(drv,ERetrieve, ETimeout, EFrame, EState, EPicToPic);	// FehlerCodes

	for(idx=0 ;idx< DEVICE_AIO_IN_MAX;idx++)	{			// INPUT Devices
		OBJS_CREATE_IDEV_PTR(idx);
		OBJS_IDEV_RD_INPUT(idev,iVal[idx]);
		if(idx< DEVICE_AIO_IN_12_MAX)					// R-Abgleich 
			OBJS_IDEV_RD_OFFSET(idev,iValO[idx]);
	}
	for(idx=0 ; idx< DEVICE_AIO_OUT_MAX; idx++)	{
		OBJS_CREATE_ODEV_PTR(idx);
		OBJS_ODEV_RD_OUTPUT(odev,oVal[idx]);
	}
	OBJS_DRV_CLR_ERRORS(drv); // alle Fehler werden nach dem Auslesen gelöscht!

	///*********************
	/// JSON-STRUCTURE
	///*********************
	written += sprintf(buf+written,"{\n");
	written += sprintf(buf+written,json_encode_O_DrvDriver());
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvControl(drv,cmd, state));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvPhysic(drv,version));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvError(drv,ERetrieve, ETimeout, EFrame, EState, EPicToPic));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,"\"%s\":[\n",OBJECT_DEVICE_Ss);
	///Input-Devices
	for (idx=0; idx < DEVICE_AIO_IN_MAX; idx++)	{
		idev = drv->Op_Inputs[idx];
		written += sprintf(buf+written,"{");
		written += sprintf(buf+written,json_encode_O_DevIn(idx));
		written += sprintf(buf+written,",\n");
		if(idx < DEVICE_AIO_IN_12_MAX) {
			written += sprintf(buf+written,json_encode_O_DevIn_Value_Off(idev,iVal[idx],iValO[idx]));
		}
		else {
			written += sprintf(buf+written,json_encode_O_DevIn_Value(idev,iVal[idx]));
		}
		written += sprintf(buf+written,",\n");
		written += box_sprintf_sel(&(idev->Op_Type->P_Type),buf + written);
		written += sprintf(buf+written,"},\n");
	}
	///Output-Devices
	for (idx=0; idx < DEVICE_AIO_OUT_MAX; idx++)	{
		OBJS_CREATE_ODEV_PTR(idx);		
		written += sprintf(buf+written,"{");
		if( idx < DEVICE_AIO_OUT_12_MAX)
			written += sprintf(buf+written,json_encode_O_DevOut(DEVICE_OUT_U_NAME,idx));
		else
			written += sprintf(buf+written,json_encode_O_DevOut(DEVICE_OUT_I_NAME,idx-2));
		written += sprintf(buf+written,",\n");
		written += sprintf(buf+written,json_encode_O_DevOut_Value(odev,oVal[idx]));
		written += sprintf(buf+written,"},\n");
	}
	written += sprintf(buf+written-2,"]}\n");
	return strlen(buf)+1;
}
	
/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{		
	strcpy(buf, DRIVER_VERSION_STRING);
	return strlen(buf)+1;
};

/** DRV.COMMAND-read */ 
static ssize_t drv_command_R(struct device_driver *driver, char *buf)	{		
	struct taio_drv *drv = &aio_O_Mct;
	enum e_state cmd = 0;
	OBJS_DRV_RD_COMMAND(drv, cmd);
	sprintf(buf,"%d",cmd);
	return strlen(buf)+1;	
};

/** DRV.COMMAND-write */ 
// START/STOP Befehl für die Kommunikationsroutine zwischer Treiber und SPI-Gerät
// Der interne Befehlszustand wird mit dem gewünschten Befehlszustand verglichen.
// Mehrfach hintereinander gesendete Start- oder Stop-Befehle werden so verhindert.
static ssize_t drv_command_W(struct device_driver *driver, const char *buf, size_t count)	{	
	struct taio_drv *drv = &aio_O_Mct;
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
	struct taio_drv *drv = &aio_O_Mct;
	enum e_state state;
	OBJS_DRV_RD_STATE(drv,state);
	return scnprintf(buf,STRING_U32_SIZE, "%d", state);	
};

/** DRV.Error-read */ 
static ssize_t drv_error_R(struct device_driver *driver, char *buf)	{
	struct taio_drv *drv = &aio_O_Mct;
	u8 ERetrieve,ETimeout,EFrame,EState,EPICToPIC;
	OBJS_DRV_RD_ERRORS(drv,ERetrieve,ETimeout,EFrame,EState,EPICToPIC);
	OBJS_DRV_CLR_ERRORS(drv);
	return scnprintf(buf,STRING_U32_SIZE, "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",ERetrieve, ETimeout,EFrame,EState,EPICToPIC);	
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE-OUTPUT
//+++++++++++++++++++++++++++++++++++++++++++
// output (X,R/W)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
// liefert: D/A-Wert als BINÄR-Wert (10 bit, 11 bit)
// Es wird immer nur eine Cache-Variable gelesen, da kein Lesezugriff auf
// die Hardware-D/A-Ports des PICs möglich ist!
static ssize_t dev_output_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	taio_dev_outp *odev = dev_get_drvdata(dev);
	int out;
	OBJS_ODEV_RD_OUTPUT(odev,out);
	return scnprintf(buf,256, "%d", out);
};
// Write 
// erwartet: D/A-Wert als BINÄR-Wert (10 bit, 11 bit)
static ssize_t dev_output_W(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {	
	taio_dev_outp *odev = dev_get_drvdata(dev);
	int out = simple_strtoul(buf, NULL, 0);	// neuer,externer Eingabewert 
	OBJS_ODEV_WR_OUTPUT_xBit(odev,out); /// Intern auf 10/11 bit !
	return strlen(buf)+1;
};

/// SYSFS-DEVICE INPUT
//+++++++++++++++++++++++++++++++++++++++++++
// input (R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_input_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	taio_dev_inp *idev = dev_get_drvdata(dev);
	u32 in;
	OBJS_IDEV_RD_INPUT(idev,in);
	return scnprintf(buf,STRING_U32_SIZE, "%x",in); 
};
//+++++++++++++++++++++++++++++++++++++++++++
// mode (X,R)
//+++++++++++++++++++++++++++++++++++++++++++
// Read
static ssize_t dev_mode_R(struct device *dev, struct device_attribute *attr, char *buf) {	
	int written= 0;
	unsigned char pos;
	taio_dev_inp *idev =	dev_get_drvdata(dev);

	pos = box_get_sel(&(idev->Op_Type->P_Type));
	written += sprintf(buf,box_get_elem(&(idev->Op_Type->P_Type),pos));
	return strlen(buf)+1;	
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER & DEVICE INTERFACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DRIVER_ATTR(json, 	S_IRUGO, 	drv_json_R, NULL);			// json: 	R
static DRIVER_ATTR(json_schema, S_IRUGO, 	drv_json_schema_R,	NULL);		// json_schema: R
static DRIVER_ATTR(version, 	S_IRUGO, 	drv_version,NULL);			// version:	R
static DRIVER_ATTR(command, 	S_IRUGO|S_IWUSR,drv_command_R,		drv_command_W); // Command R/W
static DRIVER_ATTR(state, 	S_IRUGO, 	drv_state_R,		NULL);		// state: 	R
static DRIVER_ATTR(error, 	S_IRUGO, 	drv_error_R,		NULL);		// error: 	R
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DEVICE attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DEVICE_ATTR(output, S_IRUGO|S_IWUGO,	dev_output_R, 	dev_output_W);
static DEVICE_ATTR(input,  S_IRUGO,		dev_input_R,	NULL);
static DEVICE_ATTR(mode,   S_IRUGO, 		dev_mode_R, 	NULL);

/// AIO-DRIVER-SYSFS
void aio_sysfs_ini(void) {
};	
void aio_sysfs_attr_add(struct device_driver *drv){
	int ret;
	ret= driver_create_file(drv, &driver_attr_json);	// SYSFS: drv.json
	ret= driver_create_file(drv, &driver_attr_json_schema); // SYSFS: drv.json_schema
	ret= driver_create_file(drv, &driver_attr_version);	// SYSFS: drv.version
	ret= driver_create_file(drv, &driver_attr_command);	// SYSFS: drv.command
	ret= driver_create_file(drv, &driver_attr_state);	// SYSFS: drv.state
	ret= driver_create_file(drv, &driver_attr_error);	// SYSFS: drv.error
};
void aio_sysfs_attr_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
	driver_remove_file(drv, &driver_attr_error);	
};

/// AO-DEVICES-SYSFS
void aio_ao_sysfs_attr_add(struct device *dev){
	int ret;
	trace_call_dev(dev);					// Only: dev-Trace
	ret= device_create_file(dev, &dev_attr_output);	 	// SYSFS: device.output
};
void aio_ao_sysfs_attr_del(struct device *dev){
	trace_call_dev(dev);					// Only: dev-Trace
	device_remove_file(dev, &dev_attr_output);	  	// SYSFS: device.output
};

/// AI-DEVICES-SYSFS
void aio_ai_sysfs_attr_add(struct device *dev){
	int ret;
	trace_call_dev(dev);								// Only: dev-Trace
	ret= device_create_file(dev, &dev_attr_input);	 	// SYSFS: device.input
	ret= device_create_file(dev, &dev_attr_mode);	 	// SYSFS: device.mode
};
void aio_ai_sysfs_attr_del(struct device *dev){
	trace_call_dev(dev);								// Only: dev-Trace
	device_remove_file(dev, &dev_attr_input);	 		// SYSFS: device.input
	device_remove_file(dev, &dev_attr_mode);	 		// SYSFS: device.mode
};

