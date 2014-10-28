/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS CIENT LAYER "INTERFACE"

*********************************************************************************/
#include <linux/fs.h>				//sys und udev filesystem support
#include <linux/platform_device.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

#include "../mct_types.h"
#include "mbus_def.h"
#include "mbus_cl1.h"
#include "mbus_cl2.h"
#include "mbus_app.h"

static inline struct  device *  to_dev(struct kobject *obj) {
	return  container_of(obj, struct device, kobj);
};

///****************
/// MBUS-Empfänger
///****************
// Telegramme vom Master ...
static ssize_t mbus_W(	struct file * fp,\
						struct kobject *kobj,\
						struct bin_attribute *attr,\
						char *buf,\
						loff_t offset,\
						size_t size) {	
	struct device * dev = to_dev(kobj);					// device ermitteln
	struct m_app * la 	= dev_get_drvdata(dev);			// Layer app holen
	struct m_l2  * l2  	= &(la->l2);
	tpl_drv * drv 		= (tpl_drv *)(l2->pdata);
//	printk("Interface MBUS write size: %d offset: %lld\n",size, offset);
	// Test: Status des Treibers im Running-Mode ?
	if(drv->Op_Control->P_Control.value[1] != e_running)	{
		// Nein, dann darf das Interface keine Telegramme
		// bearbeiten ... 
		printk("MBUS: Closed!\n");
		return size;	
	}	

	if(size)	{	
		/// LAYER 1
		if(m_cl1_record(&buf[0],size,&(l2->l1.rec)))	{	
			/// LAYER 2
			m_app_task_rec(la);
		}
	}
	return size;
};

///************
/// MBUS-Sender
///************
// Telegramme zum Master .... 
static ssize_t mbus_R(	struct file * fp,\
						struct kobject *kobj,\
						struct bin_attribute *attr,\
						char *buf,\
						loff_t offset,\
						size_t size) {	

	struct device  *dev 	= to_dev(kobj);			// Device
	struct m_app   *la 		= dev_get_drvdata(dev);	// M-Bus-Layer Application
	struct m_l2  * 	l2  	= &(la->l2);
//	printk("Interface MBUS read: size: %d offset: %lld\n", size, offset);

	if(!l2->l1.snd.cnt)				// keine Zeichen
		return 0;
	
	if(l2->l1.snd.cnt <= size)	{		// Zeichen passen in die Puffergröße rein
		memcpy(&buf[0], &(l2->l1.snd.buf[offset]),l2->l1.snd.cnt);
		size = l2->l1.snd.cnt;
		l2->l1.snd.cnt =0;
		return size;					// alles auf einen Rutsch ...
	}
	return -ENOMEM;						// Puffergröße ist zu klein ...
}

static struct bin_attribute	mbus_attr =  {
	.attr = {.name = "mbus", .mode = S_IRUGO|S_IWUGO },
	.size = 	0,
	.read = 	mbus_R,
	.write =	mbus_W,
};

int mbus_if_create_bin_file(struct kobject * kobj) {
	int ret;
	printk("Interface MBUS: %s\n",__FUNCTION__);
	ret = sysfs_create_bin_file(kobj,&mbus_attr);
	return ret;
};

int mbus_if_remove_bin_file(struct kobject * kobj) {
	printk("Interface MBUS: %s\n",__FUNCTION__);
	sysfs_remove_bin_file(kobj,&mbus_attr);
	return 0;
};


/// MAC-Handling
//-----------------------------------------------------------------------
#define MAC_STRING_SIZE 17
#define MAC_ADDRESS_FILE	"/sys/class/net/eth0/address"		

static int hex_char_to_binary( char ch, char* ret ) {	
      if(((ch >= 'a') && (ch <= 'f')) || ((ch >= '0') && (ch <= '9'))) {
		if((ch >= '0') && (ch <= '9'))
				*ret = ch - '0'; 
		else if( ch >= 'a' && ch <= 'f' )
				*ret = ( ch - 'a' ) + 10;
		else	
			return *ret = 0;
	}
	else {
			return *ret = 0;	
	}
	return 1;
}

/*---------------------------------------------------------------
 Sample: 00:50:c2:3f:47:02 - nur die letzten 2 Bytes verwendet!

 MAC-Adresse Anordnung 
 "xx:xx:xx:xx:xx:xx"
              || ||_____ 2
              || ||_____ 0
              ||________ 7
              |_________ 4
----------------------------------------------------------------*/
long mbus_if_secaddr_by_macaddr(char * secaddr) {
	struct file *filp;
 	loff_t pos = 0;
	char hex = 0;
	long dec = 0;
	char mac_string[MAC_STRING_SIZE];
	unsigned char bcd[4] = {0,0,0,0};

	mm_segment_t fs = get_fs();
	set_fs(get_ds());

	/// Öffnen
	filp = filp_open(MAC_ADDRESS_FILE, O_RDONLY, 0);
	if (IS_ERR(filp))	{
		printk("Unable to load '%s'.\n", MAC_ADDRESS_FILE);
		set_fs(fs);
		return 0;
	}

//	l = filp->f_path.dentry->d_inode->i_size;
//	printk("MAC-Ether 0 Len %ld:\n", l);
	
	/// Lesen
	pos = 0;
	if (vfs_read(filp, mac_string,MAC_STRING_SIZE, &pos) != MAC_STRING_SIZE)	{
		filp_close(filp, current->files);
		return 0;
	}
	else {
		// DEC bis maximal 65.535   siehe Sample: 0x4702 => 18178
		hex_char_to_binary(mac_string[12],&hex); 
		dec = (dec | hex) << 4; 
		hex_char_to_binary(mac_string[13],&hex);
		dec = (dec | hex) << 4; 
		hex_char_to_binary(mac_string[15],&hex); 
		dec = (dec | hex) << 4; 
		hex_char_to_binary(mac_string[16],&hex); 
		dec |= hex;

		// DEC auf BCD[5 Nibble] 	siehe Sample:
		bcd[1] = dec % 10;			// x08
		bcd[1] = bcd[1] << 4;		// x80
		dec = dec / 10;				// Neu: 1817
		bcd[2] = dec % 10;			// x07
		dec = dec / 10;				// Neu: 181
		bcd[2] = bcd[2] | ((dec % 10) << 4); // x17
		dec = dec / 10;				// Neu: 18
		bcd[3] = dec % 10;			// x08
		dec = dec / 10;				// Neu: 1
		bcd[3] = bcd[3] | ((dec % 10) << 4); // x17

		// High-Part Sec-M-Bus Adresse
		*secaddr = *secaddr | bcd[1];
		secaddr++;
		*secaddr = *secaddr | bcd[2];
		secaddr++;
		*secaddr = *secaddr | bcd[3];
	}
	filp_close(filp, current->files);
	set_fs(fs);
    return 1;
}