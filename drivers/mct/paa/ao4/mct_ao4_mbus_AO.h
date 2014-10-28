/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	02.09.2011
 	
 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als 
					Digitales-Ausgangs-Modul darstellen.
	
	M-BUS GERÄTE:	0-3	(index=0 bis 3)

	DATA_16	(OUTPUT)-Register (Storage0, == analoger Ausgang)
		- read		(Register lesen)
		- write		(Register = Wert, Langform des write/replace)
		.-clear	

 *******************************************************************************/
#ifndef MCT_PAA_AO4_MBUS_AO_H_
	#define MCT_PAA_AO4_MBUS_AO_H_

#include "../../if/mbus_def.h"
// ------------------------------------------------------------------
// DATA_16 (OUTPUT)  (entspricht den analogen Ausgängen)
// ------------------------------------------------------------------
#define DSCR_AO_STANDARD_LEN		0x03
#define DSCR_AO_ACTION_LEN			0x04

#define DSCR_AO_BCC	\
		((DIF_STORAGE0  | DIF_DATA_16_BIT_INT) +	\
		(VIF_LINEAR_EXTENTION_FD ) +	\
		(VIFE_FD_VOLTS_10mV))
/// read 			(rsp_ud2_data)
/// write/replace 	(standard-mode)
// 0x02,0xfd,0x47
const unsigned char dscr_AO_standard[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_16_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_VOLTS_10mV};

/// write/replace (long-mode)
// 0x02,0xfd,0xc7,0x00
const unsigned char dscr_AO_write[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_16_BIT_INT),	\
		(VIF_LINEAR_EXTENTION_FD),	\
		(EXTENSION_BIT | VIFE_FD_VOLTS_10mV),	\
		VIFE_OA_WRITE_AND_REPLACE };

/// clear
// 0x02,0xfd,0xc7,0x07
const unsigned char dscr_AO_clear[] =		\
	{	(DIF_STORAGE0 | DIF_DATA_16_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_VOLTS_10mV),	\
		VIFE_OA_CLEAR};

#endif	//MCT_PAA_AO8_MBUS_AO_H_