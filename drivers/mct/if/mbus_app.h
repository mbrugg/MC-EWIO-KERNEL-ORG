/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS CIENT APPLICATION - DEVICE

	// §08.07.2011  254 REQ_UD2 für mehrere Channel NICHT zulässig Collision-Melden!
					254 SND_UD	für mehrere Channel IST zulässig. Jeder Channel
								verarbeitet die Daten die "er" kann. Es folgt 1*NKE
								für das gültige Telegramm. 
								

 *******************************************************************************/
#ifndef __MBUS_APP_H__
	#define __MBUS_APP_H__

#include "mbus_def.h"				// basics
#include "mbus_cl1.h"				// client layer1 
#include "mbus_cl2.h"				// client layer2 

#define FAKE_BUF_SIZE		255

// defines für m_app.app_readout 
// Aufbau:		0000 0000
// lower  nibble = selection without specified data field
// higher nibble = selection with specified data field (Object Actions)	 
#define SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_ALL					0x0F
#define SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION		1<<0
#define SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_FIRMWAREVERSION		1<<1
// ... unused
#define SEL_WITH_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION		1<<4
#define SEL_WITH_SPECIFIED_DATAFIELD_APP_FIRMWAREVERSION		1<<5

struct m_app {
	unsigned char interface;
	unsigned char eeprom_protect;
	unsigned char eeprom_verify;
	unsigned char select; 					// Applikation-Select mit 253 Netzwerk!
	unsigned char last_ud2_fcb_bits;	 	// prim.addr, 254,255, 
	unsigned char last_ud2_fcb_bits_253; 	// pseudo prim.addr 253 + sek.addr
	int (*cmd_rsp_ud_data2) (struct m_app * la);
#ifdef INCLUDE_SUPPORT_SWITCH_MBUS_REQ_UD1
	unsigned char last_ud1_fcb_bits;		// prim.addr, 254,255, 
	int (*cmd_rsp_ud_data1) (struct m_app * la);
#endif
	struct m_l2 l2;
//	char	(*fake_line)[FAKE_BUF_SIZE];
};

extern void 			m_app_task_rec		(struct m_app * la);
extern int  			m_app_rsp_ud_data2	(struct m_app * la);	// STUB
extern int  			m_app_rsp_ud_data1	(struct m_app * la);	// STUB

#endif // __MBUS_APP_H__
