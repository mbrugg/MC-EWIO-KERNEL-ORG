/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		21.09.2011
 	
 	Description:  MODBUS-LAYER: "INTERFACE" (only SLAVE) 

*********************************************************************************/
#include <linux/fs.h>				//sys und udev filesystem support
#include <linux/platform_device.h>
#include <linux/sched.h>

#include "../mct_types.h"
#include "modbus_def.h"
#include "modbus_sl1.h"
#include "modbus_sl2.h"
#include "modbus_app.h"

static inline struct  device *  to_dev(struct kobject *obj) {
	return  container_of(obj, struct device, kobj);
};

///*****************
/// MODBUS-Empfänger
///*****************
// Telegramme vom Master ...
static ssize_t modbus_W(	struct file * fp,\
							struct kobject *kobj,\
							struct bin_attribute *attr,\
							char *buf,\
							loff_t offset,\
							size_t size) {		
	struct device 	*dev= to_dev(kobj);					// device ermitteln
	struct mod_app 	*la = dev_get_drvdata(dev);			// Layer mod_app holen
	struct mod_l2	*l2	= &(la->l2);					// Layer mod_l2 holen
	tpl_drv 		*drv= (tpl_drv *)(l2->pdata);
	
//	printk("Interface MODBUS write size: %d offset: %lld\n",size, offset);

	// Test: Status des Treibers im Running-Mode ?
	if(drv->Op_Control->P_Control.value[1] != e_running)	{
		// Nein, dann darf das Interface keine Telegramme
		// bearbeiten ... 
		printk("MODBUS: Closed!\n");
		return size;	
	}	
	if(size > 0)	{
		/// LAYER 1
		if(mod_sl1_record(&buf[0],size,&(l2->l1.rec)))	{	
			/// LAYER APP
			mod_app_task_rec(la);
		}
	}
	return size;
};

///***************
/// MODBUS-Sender
///***************
// Telegramme zum Master ....
static ssize_t modbus_R(struct file * fp,\
						struct kobject *kobj,\
						struct bin_attribute *attr,\
						char *buf,\
						loff_t offset,\
						size_t size) {	
	struct device  *dev = to_dev(kobj);			// Device
	struct mod_app *la 	= dev_get_drvdata(dev);	// M-Bus-Layer Application
	struct mod_l2  *l2  = &(la->l2);
//	printk("Interface MODBUS read: size: %d offset: %lld\n", size, offset);

	if(l2->l1.snd.cnt <= size)	{
		memcpy(&buf[0], &(l2->l1.snd.buf[offset]),l2->l1.snd.cnt);
		size = l2->l1.snd.cnt;
		l2->l1.snd.cnt =0;	
//		return l2->l1.snd.cnt;
	}
	return size;	// Maximal bearbeitete Zeichen ..
}

static struct bin_attribute	modbus_attr =  {
	.attr = {.name = "modbus", .mode = S_IRUGO|S_IWUGO },
	.size = 	0,
	.read = 	modbus_R,
	.write =	modbus_W,
};

int modbus_if_create_bin_file(struct kobject * kobj) {
	int ret;
	printk("Interface MODBUS: %s\n",__FUNCTION__);
	ret = sysfs_create_bin_file(kobj,&modbus_attr);
	return ret;
};

int modbus_if_remove_bin_file(struct kobject * kobj) {
	printk("Interface MODBUS: %s\n",__FUNCTION__);
	sysfs_remove_bin_file(kobj,&modbus_attr);
	return 0;
};

