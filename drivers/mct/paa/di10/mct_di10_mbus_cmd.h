/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011
 	
 	Description:	MBUS-Command-Implemetation for mct_paa_di10-Driver

	Reset-Subcodes:	- APPLICATION_RESET_USER_DATA

 *******************************************************************************/
#ifndef MCT_PAA_DI10_MBUS_CMD_H_
	#define MCT_PAA_DI10_MBUS_CMD_H_

#include "../../if/mbus_cl2.h"
#include "../../if/mbus_app.h"

///	OVERLOAD mbus_cmd_rsp_ud_data2()
/// Hinweise:
// keine ...
extern int mbus_cmd_rsp_ud_data2 (struct m_app * la);


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


#endif	//MCT_SPI_DIO_MBUS_CMD_H_