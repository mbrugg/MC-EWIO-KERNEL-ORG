/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	25.08.2011
 	
 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als S0
					Imulszähler-Modul darstellen.

					mode_x = MODE_COUNTER

					COUNTER-Register (Storage0, == input_x)
					- read		(Register lesen)
					- standard 	(Register = Wert, Kurzform des write/replace)
					- write		(Register = Wert, Langform des write/replace)
					- add		(Register + Wert, Addition)
					- clear		(Register löschen)

					STATE_OF_PARAMETER_ACTIVATION (Storage1 == Einrichtung)
					- read		(Register lesen)
					- clear		(Register löschen)

					TIMEPOINT	(Storage1 == Zeitstempel)
					- read		(Register lesen)
					- write		(Register = Wert, Langform des write/replace)

					FREEZE	(Storage1 == Frostwert/Einrichtung)
					- read		(Register lesen)
					- standard 	(Register = Wert, Kurzform des write/replace)
					- write		(Register = Wert, Langform des write/replace)
					- add		(Register + Wert, Addition)
					- clear		(Register löschen)
					- freeze	(COUNTER-Register sichern)

 *******************************************************************************/
#ifndef MCT_PIN_DI_MBUS_S0_H_
	#define MCT_PIN_DI_MBUS_S0_H_

#include "../if/mbus_def.h"

// ------------------------------------------------------------------
// COUNTER   (entspricht dem Treiber-Parameter: input_x)
// ------------------------------------------------------------------
#define DSCR_S0_IMP_STANDARD_LEN		0x03
#define DSCR_S0_IMP_ACTION_LEN			0x04

#define DSCR_S0_IMP_BCC	\
	(	(	DIF_STORAGE0 | DIF_DATA_64_BIT_INT) +	\
		 	VIF_LINEAR_EXTENTION_FD + \
		 	VIFE_FD_CUMULATION_COUNTER)

/// read 			(rsp_ud2_data)
/// write/replace 	(standard-mode)
// 0x07,0xfd,0x61
const unsigned char dscr_S0_imp_standard[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_CUMULATION_COUNTER };
/// write/replace (long-mode)
// 0x07,0xfd,0xe1,0x00
const unsigned char dscr_S0_imp_write[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),	\
		VIFE_OA_WRITE_AND_REPLACE };
/// add
// 0x07,0xfd,0xe1,0x01
const unsigned char dscr_S0_imp_add[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),	\
		VIFE_OA_ADD };
/// clear
// 0x07,0xfd,0xe1,0x07
const unsigned char dscr_S0_imp_clear[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),	\
		VIFE_OA_CLEAR};

// ------------------------------------------------------------------
// STATE_OF_PARAMETER_ACTIVATION  (S0-Einrichtung-Impulszähler an/aus)
// ------------------------------------------------------------------
#define DSCR_S0_STA_STANDARD_LEN		0x03
#define DSCR_S0_STA_ACTION_LEN			0x04

#define DSCR_S0_STA_BCC	\
	(	(DIF_STORAGE1 | DIF_DATA_8__BIT_INT) +	\
		VIF_LINEAR_EXTENTION_FD +	\
		VIFE_FD_STATE_OF_PARAMETER_ACTIVATION)

/// read (rsp_ud2_data)
// 0x41,0xfd,0x66
const unsigned char dscr_S0_sta_standard[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_8__BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_STATE_OF_PARAMETER_ACTIVATION };
/// clear
// 0x41,0xfd,0xe6,0x07
const unsigned char dscr_S0_sta_clear[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_8__BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		( EXTENSION_BIT | VIFE_FD_STATE_OF_PARAMETER_ACTIVATION),	\
		VIFE_OA_CLEAR };

// ------------------------------------------------------------------
// TIMEPOINT 	(Zeitstempel, anwendbar z.B bei Abrufsynchronisation) 
// ------------------------------------------------------------------
#define DSCR_S0_TIMEPOINT_ACTION_LEN		0x03
#define DSCR_S0_TIMEPOINT_STANDARD_LEN		0x02

#define DSCR_S0_TIMEPOINT_BCC	\
	(	(DIF_STORAGE1 | DIF_DATA_32_BIT_INT) +	\
		VIF_TIMEPOINT_TYPE_F)

/// read (rsp_ud2_data)
// 0x44,0x6d
const unsigned char dscr_S0_timepoint_standard[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_32_BIT_INT),	\
		VIF_TIMEPOINT_TYPE_F };

/// write/replace	(long-mode)
// 0x44,0xed,0x00
const unsigned char dscr_S0_timepoint_write[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_32_BIT_INT),	\
		(EXTENSION_BIT | VIF_TIMEPOINT_TYPE_F),	\
		VIFE_OA_WRITE_AND_REPLACE };

// ------------------------------------------------------------------
// FREEZE	(gefrorener Impulswert / S0-Einrichtung-Impulszähler) 
// ------------------------------------------------------------------
#define DSCR_S0_FRZ_ACTION_LEN			0x04
#define DSCR_S0_FRZ_STANDARD_LEN		0x03

#define DSCR_S0_FRZ_BCC	\
	(	(DIF_STORAGE1 | DIF_DATA_64_BIT_INT) +	\
		VIF_LINEAR_EXTENTION_FD + \
		VIFE_FD_CUMULATION_COUNTER)

/// read 			(rsp_ud2_data)
/// write/replace 	(standard-mode)
// 0x47,0xfd,0x61
const unsigned char dscr_S0_frz_standard[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_CUMULATION_COUNTER };
/// write/replace (long-mode)
// 0x47,0xfd,0xe1,0x00
const unsigned char dscr_S0_frz_write[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),
		VIFE_OA_WRITE_AND_REPLACE };
/// add
// 0x47,0xfd,0xe1,0x01
const unsigned char dscr_S0_frz_add[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),
		VIFE_OA_ADD };
/// clear
// 0x47,0xfd,0xe1,0x07
const unsigned char dscr_S0_frz_clear[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_64_BIT_INT),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),
		VIFE_OA_CLEAR};
/// freeze, no data
// 0x40,0xfd,0xe1,0x0B
const unsigned char dscr_S0_frz_freeze[] =	\
	{	DIF_STORAGE1,	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_CUMULATION_COUNTER),
		VIFE_OA_FREEZE };

#endif	//MCT_PIN_DI_MBUS_S0_H_