/*********************************************************************************
	Copyright 	MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		15.02.2012
 
	Description: MODBUS-Interface for mct_spi_aio-Driver
	
 *******************************************************************************/
#include <linux/fs.h>				//sys und udev filesystem support
#include <linux/platform_device.h>
#include "../mct_debug.h"
#include "../mct_versions.h"

#include "mct_spi_aio_objs.h"			// objects
#include "mct_spi_aio_modbus_cmd.h"
#include "mct_spi_aio_modbus_cfg.h"
#include "../if/modbus_if.h"
#include "../if/modbus_app.h"

//#include "mct_spi_aio_modbus_test.h"	// TEST-Sequencen

/// MODBUS-PLATTFORM DEVICE - Zeiger
static struct platform_device * pdev;
/// MODBUS-DEVICE Interface Name
char aio_dev_if_modbus[100];

static DECLARE_COMPLETION( modbus_device_is_free );

/// la (typ: mod_app) mit Zeiger auf aio_O_Mct (typ:taio_drv)
// Hier wird das M_APP-Object mit dem zentalen Treiber-Objekt
// "verbunden". Die Anbindung erfolgt unter void * data in der
// MODBUS-l2-Substruktur. Dadurch ist es möglich auch mit Funktionen
// der MODBUS-l2-Substruktur auf alle Elemente des zentralen Treiber-
// Objektes zuzugreifen. Das wird in der Regel dann in den OVERLOAD-
// Funktionen realisiert.
static struct mod_app la = {
//	.fake_line	= &MODBUS_TLGs[0],
	.l2 = {
	.slvs_cfg= MODBUS_SLVS,
	.slvs[MODBUS_SLV_0].pri_addr	= MODBUS_RTU_TO_INTERN_BASE_ADDR + DRIVER_PIN_AIO_IDENT_NUM * 10,
	.cmd_rsp_03						= aio_mod_sl2_cmd_rsp_03,	// Outputs
	.cmd_rsp_04						= aio_mod_sl2_cmd_rsp_04,	// Inputs
	.cmd_rsp_06						= aio_mod_sl2_cmd_rsp_06,	// Outputs
	.cmd_rsp_16						= aio_mod_sl2_cmd_rsp_16,	// Outputs
	.cmd_rsp_mei_14_code			= aio_mod_sl2_cmd_rsp_mei_14_code,
	.pdata							= &aio_O_Mct,
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
	
	/// INSTANCE overload PriAddr 
	// la.l2.slvs[MODBUS_SLV_0].pri_addr = slot *16;
	/// INTERFACE-NAME
	strcpy(aio_dev_if_modbus,MODBUS_DEVICE_INTERFACE);
	slotstr[0]= slot +0x30;
	strcat(aio_dev_if_modbus,slotstr);
	pdev = platform_device_alloc(aio_dev_if_modbus,0);
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