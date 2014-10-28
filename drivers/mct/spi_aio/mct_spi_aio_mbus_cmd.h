/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:	MBUS Command-Implemetation for mct_spi_aio-Driver

	Reset-Subcodes:	- APPLICATION_RESET_USER_DATA

 *******************************************************************************/
#ifndef MCT_SPI_AIO_MBUS_CMD_H_
	#define MCT_SPI_AIO_MBUS_CMD_H_

#include "../if/mbus_cl2.h"
#include "../if/mbus_app.h"

///	OVERLOAD mbus_cmd_rsp_ud_data2()
/// Hinweise:
// keine ...
extern int mbus_cmd_rsp_ud_data2 (struct m_app * la);

///	OVERLOAD mbus_cmd_snd_ud_usr_data()
/// Hinweise:
//	Es werden hier nur fixe Datenstrukturen erwartet die ab DIF1-Feld 
//	ausgewertet werden. Die einzelnen DIF-Sequenzen können in beliebiger
//	Reihenfolge im Telegramm angeordnet (shuffle-Mode) sein. Die einzelnen
//	DIF-Sequencen werden entsprechend der Anordnung im Telegramm 
//	immer sequentiell abgearbeitet!
 	
//	Der Wert l1->rec.stack darf nur manipuliert werden, wenn alle 
//	Bedingungen für den DATA-RECORD erfüllt werden!
//	Sonst ist l1->rec.stack zu belassen und mit return(0) zu beenden!
	
//	Der Parameter mask ist eine BIT-Maske, welche die Channel-Info 
//	über die aktiven Channel beinhaltet.
//	Es werden bis zu MBUS_CHANNEL_MAX Channel unterstützt, wobei
//	die Variable l2->chan_cfg die aktuell konfigurierte Anzahl angibt!
extern int mbus_cmd_snd_ud_usr_data	(struct m_l2 * l2, unsigned char mask);

/// OVERLOAD  mbus_cmd_snd_ud_application_reset()
/// Hinweise:
// support: "APPLICATION_RESET_USER_DATA"
extern int mbus_cmd_snd_ud_application_reset(struct m_l2 * l2, unsigned char mask);

/// OVERLOAD  mbus_cmd_snd_ud_global_readout()
/// Hinweise:
// DIF  = 0x7F 	
// VIF	= 0x7E	(optional)
// Alle DatenRecords werden markiert zum einmaligen auslesen, bei REQ_UD2
// aber nur dann, wenn der Wert noch nicht zur readout-Liste addiert wurde!
extern int mbus_cmd_snd_ud_global_readout(struct m_l2 * l2, unsigned char mask);

#endif	//MCT_SPI_AIO_MBUS_CMD_H_