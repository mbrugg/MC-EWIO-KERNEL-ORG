/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		10.10.2011
 	
 	Description: MODBUS-Interface for mct_paa_ao4-Driver

 *******************************************************************************/
#include <linux/fs.h>				//sys und udev filesystem support
#include <linux/platform_device.h>
#include "../../mct_debug.h"
#include "../../mct_versions.h"

#include "../mct_paa.h"
#include "mct_ao4_objs.h"			// objects
#include "mct_ao4_modbus_cmd.h"
#include "mct_ao4_modbus_cfg.h"
#include "../../if/modbus_if.h"
#include "../../if/modbus_app.h"
//#include "mct_ao4_modbus_test.h"	// TEST-Sequencen

/// MODBUS-PLATTFORM DEVICE - Zeiger
static struct platform_device * pdev;
/// MODBUS-DEVICE Interface Name
char ao4_dev_if_modbus[100];

static DECLARE_COMPLETION( modbus_device_is_free );

/// la (typ: mod_app) mit Zeiger auf ao4_O_Mct (typ:tao4_drv)
// Hier wird das M_APP-Object mit dem zentalen Treiber-Objekt
// "verbunden". Die Anbindung erfolgt unter void * data in der
// MODBUS-l2-Substruktur. Dadurch ist es möglich auch mit Funktionen
// der MODBUS-l2-Substruktur auf alle Elemente des zentralen Treiber-
// Objektes zuzugreifen. Das wird in der Regel dann in den OVERLOAD-
// Funktionen realisiert.
static struct mod_app la = {
//	.fake_line			= &TLGs[0],
	.l2 = {
	.slvs_cfg= MODBUS_SLVS,
	.slvs[MODBUS_SLV_0].pri_addr	= MODBUS_RTU_TO_LBUS_BASE_ADDR + DRIVER_PAA_AO4_IDENT_NUM,
	.cmd_rsp_03						= ao4_mod_sl2_cmd_rsp_03,
	.cmd_rsp_06						= ao4_mod_sl2_cmd_rsp_06,	
	.cmd_rsp_16						= ao4_mod_sl2_cmd_rsp_16,
	.cmd_rsp_mei_14_code			= ao4_mod_sl2_cmd_rsp_mei_14_code,
	.pdata							= &ao4_O_Mct,
	},	
};

static void modbus_dev_release(struct device * dev)	{
	complete(&modbus_device_is_free);
};

/// MODBUS-DEVICE erzeugen
int  modbus_build(struct platform_driver * drv,unsigned char slot) {	
	int ret = 0;
	int lnk_err;
	char slotstr[2] = {0x00,0x00};

	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	
	/// Overload MODBUS-Addr 
	la.l2.slvs[MODBUS_SLV_0].pri_addr += (slot*10);

	/// INTERFACE-NAME
	strcpy(ao4_dev_if_modbus,MODBUS_DEVICE_INTERFACE);
	slotstr[0]= slot +0x30;
	strcat(ao4_dev_if_modbus,slotstr);

	pdev = platform_device_alloc(ao4_dev_if_modbus,0);
	pdev->dev.release = modbus_dev_release;	
	platform_device_add(pdev);						// MODBUS-Platf.Dev registrieren	
	platform_set_drvdata(pdev,&la); 				// MODBUS-Platf.Dev. Datenzeiger
	pdev->dev.driver = &drv->driver;
	lnk_err = device_bind_driver(&pdev->dev);		// symbolischen Link
	ret= modbus_if_create_bin_file(&pdev->dev.kobj);
	return ret;
};

/// MODBUS-DEVICE entfernen
void modbus_destroy(void) {
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	platform_set_drvdata(pdev,NULL);				// MODBUS-Platf.Dev. Datenzeiger frei
	platform_device_unregister(pdev);				// MODBUS-Platf.Dev deregistrieren
	modbus_if_remove_bin_file(&pdev->dev.kobj);
	wait_for_completion(&modbus_device_is_free);		
};