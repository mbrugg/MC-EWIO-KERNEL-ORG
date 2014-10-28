/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$	

*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <linux/init.h>

#include "../mct_debug.h"
#include "mct_paa.h"
#include "mct_paa_mux.h"
#include "mct_paa_mux_objs.h"
#include "mct_paa_mux_schema.h"	// json-schema

///************************************************
///	OBJECT-ACCESS-MAKROS (driver specific: Op_PaaMux)
///************************************************
// Makros für Zugriff auf Driver.Command
#define SYSFS_READ_COMMAND(cmd) {	\
			spin_lock_bh(&Op_PaaMux->Op_Control->lock);	\
			cmd = Op_PaaMux->Op_Control->P_Control.value[0];	\
			spin_unlock_bh(&Op_PaaMux->Op_Control->lock);}

// Makros für Zugriff auf Driver.State
#define SYSFS_READ_STATE(state)	{ 	\
			spin_lock_bh(&Op_PaaMux->Op_Control->lock);	\
			state = Op_PaaMux->Op_Control->P_Control.value[1];	\
			spin_unlock_bh(&Op_PaaMux->Op_Control->lock);}

// Makros für Zugriff auf Driver.State & Command - syncron Access
#define SYSFS_READ_COMMAND_AND_STATE(cmd, state)	{ 	\
			spin_lock_bh(&Op_PaaMux->Op_Control->lock);	\
			cmd = Op_PaaMux->Op_Control->P_Control.value[0];	\
			state = Op_PaaMux->Op_Control->P_Control.value[1];	\
			spin_unlock_bh(&Op_PaaMux->Op_Control->lock);}

/*
#define SYSFS_READ_SLOTS(slots) {	\
			spin_lock_bh(&Op_PaaMux->Op_LBus->lock);	\
			slots = Op_PaaMux->Op_LBus->P_Slots.value;	\
			spin_unlock_bh(&Op_PaaMux->Op_LBus->lock);	}
*/

/// ***************
/// JSON ENCODE
/// ***************
#define json_encode_O_DrvDriver() "\"%s\":\"%s\"",\
			OBJECT_DRIVER_S,	\
			driver->name

#define json_encode_O_DrvControl(command, state) "\"%s\":{\"%s\":%d,\"%s\":%d}",\
			Op_PaaMux->Op_Control->pname,	\
			Op_PaaMux->Op_Control->P_Control.pname[0],	\
			(command),	\
			Op_PaaMux->Op_Control->P_Control.pname[1],	\
			(state)

#define json_encode_O_DrvPhysic "\"%s\":{\"%s\":\"%s->%s\",\"%s\":\"%s\"}",\
			Op_PaaMux->Op_Physic->pname,\
			Op_PaaMux->Op_Physic->P_SPILayer.pname,\
			dev_name(&Op_PaaMux->ldev->spi->dev),\
			Op_PaaMux->ldev->spi->dev.driver->name,\
			Op_PaaMux->Op_Physic->P_BUSLayer.pname,\
			dev_name(&Op_PaaMux->pdev->dev)

#define json_encode_O_DrvLBus(slots) "\"%s\":{\"%s\":%d",\
			Op_PaaMux->Op_LBus->pname,	\
			Op_PaaMux->Op_LBus->P_Slots.pname,	\
			(slots)

#define json_encode_O_DrvSlot(slot) "{\"%s\":\"%s\",\"%s\":\"%s\",\"%s\":\"0x%02x\",\"%s\":\"0x%02x\",\"%s\":\"0x%02x\",\"%s\":\"0x%02x\"}",\
			Op_PaaMux->Op_LBus->O_Slots.buf_nam, \
			Op_PaaMux->lbus.ctrl[(slot)].buf_nam, \
			Op_PaaMux->Op_LBus->O_Slots.buf_ver, \
			Op_PaaMux->lbus.ctrl[(slot)].buf_ver, \
			Op_PaaMux->Op_LBus->O_Slots.cfg, \
			Op_PaaMux->lbus.ctrl[(slot)].cfg,	\
			Op_PaaMux->Op_LBus->O_Slots.cfg_in, \
			Op_PaaMux->lbus.ctrl[(slot)].cfg_in,	\
			Op_PaaMux->Op_LBus->O_Slots.cfg_out, \
			Op_PaaMux->lbus.ctrl[(slot)].cfg_out,\
			Op_PaaMux->Op_LBus->O_Slots.cfg_cfg,\
			Op_PaaMux->lbus.ctrl[(slot)].cfg_cfg


#define json_encode_O_DrvTimeouts(slots) "{\"%s\":%d,\"%s\":\"0x%02x\",\"%s\":\"0x%02x\"}",\
		Op_PaaMux->Op_Timeouts->P_Slot[slots].pname,\
		Op_PaaMux->Op_Timeouts->P_Slot[slots].value,\
		Op_PaaMux->Op_Timeouts->P_Code[slots].pname,\
		Op_PaaMux->Op_Timeouts->P_Code[slots].value,\
		Op_PaaMux->Op_Timeouts->P_Counter[slots].pname,\
		Op_PaaMux->Op_Timeouts->P_Counter[slots].value


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/** DRV.JSON_SCHEMA-read */
static ssize_t drv_json_schema_R(struct device_driver *driver, char *buf)	{	
	int written= 0;
	written += sprintf(buf,JSON_SCHEMA_PAA_MUX_EN);
	return strlen(buf)+1;
}

/** DRV.JSON-write (only Test) */
/** DRV.JSON-read */
static ssize_t drv_json_R(struct device_driver *driver, char *buf)	{			
	enum e_state 		_drv_state;
	enum e_state 		_drv_command;
	unsigned int		_idx;
	int 				 written= 0;
	u8					_drv_slots = Op_PaaMux->Op_LBus->P_Slots.value; //Slotanzahl
	///**********************
	/// OBJECT-WERTE (protect)
	///***********************
	SYSFS_READ_COMMAND_AND_STATE(_drv_command, _drv_state);		// CONTROL/STATE
	///*********************
	/// JSON-STRUCTURE
	///*********************
	written += sprintf(buf+written,"{\n");
	written += sprintf(buf+written,json_encode_O_DrvDriver());
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvControl(_drv_command, _drv_state));
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvPhysic);
	written += sprintf(buf+written,",\n");
	written += sprintf(buf+written,json_encode_O_DrvLBus(_drv_slots));
	written += sprintf(buf+written,",\n");

	written += sprintf(buf+written,"\"%s\":",Op_PaaMux->Op_Timeouts->pname);	
	written += sprintf(buf+written,"[\n");
	for(_idx = 0; _idx < _drv_slots; _idx++) {
		written += sprintf(buf+written,json_encode_O_DrvTimeouts(_idx));
		written += sprintf(buf+written,",\n");	
	}
	written = written-2;
	written += sprintf(buf+written,"\n],\n");
	written += sprintf(buf+written,"\"%s\":",Op_PaaMux->Op_LBus->O_Slots.pname);
	written += sprintf(buf+written,"[\n");
	for(_idx = 0; _idx < _drv_slots; _idx++) {
		written += sprintf(buf+written,json_encode_O_DrvSlot(_idx));
		written += sprintf(buf+written,",\n");
	}
	written += sprintf(buf+written-2,"\n]}}");
	return strlen(buf)+1;
}
	
/** DRV.VERSION-read */ 
static ssize_t drv_version(struct device_driver *driver, char *buf)	{
	strcpy(buf, DRIVER_VERSION_STRING);
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
static ssize_t drv_command_W(struct device_driver *driver, const char *buf, size_t count)	{	
	enum e_state _ext_command = 0; 
	enum e_state _int_command = 0; 
	_ext_command = simple_strtoul(buf, NULL, 10);
	// Driver-Start/Stop
	if((_ext_command == e_start) ||(_ext_command == e_stop) )	{	
		
		spin_lock_bh(&Op_PaaMux->Op_Control->lock);
	   _int_command = Op_PaaMux->Op_Control->P_Control.value[0];
		if(_int_command != _ext_command)	{
			printk("STOP------------>! intern: %d extern: %d\n", Op_PaaMux->Op_Control->P_Control.value[0],
			Op_PaaMux->Op_Control->P_Control.value[1]);
			
			Op_PaaMux->Op_Control->P_Control.value[0] = _ext_command;	// externes Kommando
			Op_PaaMux->Op_Control->P_Control.value[1] = _ext_command;	// copy also to state!
		}
		spin_unlock_bh(&Op_PaaMux->Op_Control->lock);
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
/// SYSFS-DRIVER INTERFACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
/// SYSFS-DRIVER attributes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++
static DRIVER_ATTR(json, 		S_IRUGO, 	drv_json_R, NULL);			// json: 		R
static DRIVER_ATTR(json_schema, S_IRUGO, 	drv_json_schema_R,	NULL);	// json_schema: R
static DRIVER_ATTR(version, 	S_IRUGO, 	drv_version,NULL);			// version:		R
static DRIVER_ATTR(command, 	S_IRUGO|S_IWUSR,drv_command_R,drv_command_W); // Command R/W
static DRIVER_ATTR(state, 		S_IRUGO, 	drv_state_R,	NULL);		// state: 		R
//static DRIVER_ATTR(error, 		S_IRUGO, 	drv_error_R,	NULL);	// error: 		R

/// PAA_MUX-DRIVER-SYSFS
void paa_mux_attr_ini(void) {
};	

void paa_mux_attr_add(struct device_driver *drv){
	int ret;
	ret= driver_create_file(drv, &driver_attr_json);		// SYSFS: drv.json
	ret= driver_create_file(drv, &driver_attr_json_schema); // SYSFS: drv.json_schema	
	ret= driver_create_file(drv, &driver_attr_version);		// SYSFS: drv.version 	
	ret= driver_create_file(drv, &driver_attr_command);		// SYSFS: drv.command 		
	ret= driver_create_file(drv, &driver_attr_state);		// SYSFS: drv.state 		
//	ret= driver_create_file(drv, &driver_attr_error);		// SYSFS: drv.error 		
};

void paa_mux_attr_del(struct device_driver *drv){
	driver_remove_file(drv, &driver_attr_json);
	driver_remove_file(drv, &driver_attr_json_schema);
	driver_remove_file(drv, &driver_attr_version);
	driver_remove_file(drv, &driver_attr_command);
	driver_remove_file(drv, &driver_attr_state);	
//	driver_remove_file(drv, &driver_attr_error);	
};
