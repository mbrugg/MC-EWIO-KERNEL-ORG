/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		01.09.2011
 	
 	Description: MBUS-Interface for mct_paa_ai8-Driver

 *******************************************************************************/
#include <linux/fs.h>				//sys und udev filesystem support
#include <linux/platform_device.h>
#include "../../mct_debug.h"
#include "../../mct_versions.h"

#include "../mct_paa.h"
#include "mct_ai8_objs.h"		// objects
#include "mct_ai8_mbus_cmd.h"
#include "mct_ai8_mbus_cfg.h"
#include "../../if/mbus_if.h"

/// MBUS-PLATTFORM DEVICE - Zeiger
static struct platform_device * pdev;
/// MBUS-DEVICE Interface Name
char ai8_dev_if_mbus[100];

static DECLARE_COMPLETION( mbus_device_is_free );

/// la (typ: m_app) mit Zeiger auf ai8_O_Mct (typ:tai8_drv)
// Hier wird das M_APP-Object mit dem zentalen Treiber-Objekt
// "verbunden". Die Anbindung erfolgt unter void * data in der
// MBUS-l2-Substruktur. Dadurch ist es möglich auch mit Funktionen
// der MBUS-l2-Substruktur auf alle Elemente des zentralen Treiber-
// Objektes zuzugreifen. Das wird in der Regel dann in den OVERLOAD-
// Funktionen realisiert.
static struct m_app la = {
	.cmd_rsp_ud_data2  			= mbus_cmd_rsp_ud_data2,
	.l2 = {
	.slvs_cfg	= MBUS_SLVS,
	.slvs[MBUS_SLV_IN0].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN1].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN2].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN3].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN4].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN5].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN6].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN7].sec_addr = {0x00,DRIVER_PAA_AI8_IDENT_NUM,0x00,0x00,MBUS_MANU_HEX,MBUS_GEN_HEX,MBUS_MED_HEX},
	.slvs[MBUS_SLV_IN0].access_cnt = 1,
	.slvs[MBUS_SLV_IN1].access_cnt = 1,
	.slvs[MBUS_SLV_IN2].access_cnt = 1,
	.slvs[MBUS_SLV_IN3].access_cnt = 1,
	.slvs[MBUS_SLV_IN4].access_cnt = 1,
	.slvs[MBUS_SLV_IN5].access_cnt = 1,
	.slvs[MBUS_SLV_IN6].access_cnt = 1,
	.slvs[MBUS_SLV_IN7].access_cnt = 1,
	.slvs[MBUS_SLV_IN0].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN1].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN2].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN3].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN4].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN5].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN6].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.slvs[MBUS_SLV_IN7].state = VARIABLE_DATA_STATUS_POWER_LOW,
	.cmd_snd_ud_application_reset 	= mbus_cmd_snd_ud_application_reset,
	.cmd_snd_ud_global_readout 		= mbus_cmd_snd_ud_global_readout,
	.pdata							= &ai8_O_Mct,
	},
};


static void mbus_dev_release(struct device * dev)	{
	complete(&mbus_device_is_free);
};

/// MBUS-DEVICE erzeugen
int  mbus_build(struct platform_driver * drv, unsigned char slot) {	
	int ret = 0;
	int lnk_err;
	unsigned char mbus_slvs;
	char slotstr[2] = {0x00,0x00};

	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	for (mbus_slvs= 0; mbus_slvs < MBUS_SLVS; mbus_slvs++ ) {
		/// OVERLOAD SecAddr[0-3]
		ret= mbus_if_secaddr_by_macaddr(&la.l2.slvs[mbus_slvs].sec_addr[1]);
		la.l2.slvs[mbus_slvs].sec_addr[0] = (slot *16) + mbus_slvs;
	};
	/// INTERFACE-NAME
	strcpy(ai8_dev_if_mbus,MBUS_DEVICE_INTERFACE);
	slotstr[0]= slot +0x30;
	strcat(ai8_dev_if_mbus,slotstr);

	pdev = platform_device_alloc(ai8_dev_if_mbus,0);
	pdev->dev.release = mbus_dev_release;	
	platform_device_add(pdev);						// MBUS-Platf.Dev registrieren	
	platform_set_drvdata(pdev,&la); 				// MBUS-Platf.Dev. Datenzeiger
	pdev->dev.driver = &drv->driver;
	lnk_err = device_bind_driver(&pdev->dev);		// symbolischen Link
	ret= mbus_if_create_bin_file(&pdev->dev.kobj);
	return ret;
};

/// MBUS-DEVICE entfernen
void mbus_destroy(void) {
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	platform_set_drvdata(pdev,NULL);				// MBUS-Platf.Dev. Datenzeiger frei
	platform_device_unregister(pdev);				// MBUS-Platf.Dev deregistrieren
	mbus_if_remove_bin_file(&pdev->dev.kobj);
	wait_for_completion(&mbus_device_is_free);		
};