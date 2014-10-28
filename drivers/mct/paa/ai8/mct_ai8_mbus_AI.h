/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	01.09.2011
 	
 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als 
					Digitales-Eingangs-Modul darstellen.
	
	M-BUS GERÄTE:	0...7	(index=0 bis 7)

	DATA_32	(INPUT)-Register (Storage0)	analoger Eingang
		- read		(Register lesen)

 		Durch Konfiguration möglich: Volt, Ohm, unkonfiguriert
 *******************************************************************************/
#ifndef MCT_PAA_AI8_MBUS_AI_H_
	#define MCT_PAA_AI8_MBUS_AI_H_

#include "../../if/mbus_def.h"

// ------------------------------------------------------------------
// DATA_32 (INPUT)  (entspricht den analogen Eingängen)
//					als VIF wird noch der Treiber-Parameter in_mode_x
//					angehängt - "evitcani" == "inactive"
// ------------------------------------------------------------------
#define DSCR_AI_UNDEF_LEN	0x05	// Einheit:					= ? 
#define DSCR_AI_OHM_LEN		0x06	// Einheit: VIF_PLAIN		= 1 Ohm
#define DSCR_AI_VOLT_LEN	0x03	// Einheit: VIF_FD 			= 1 Volt 

//--------------------------------------------------
// UNKONFIGURIERT
//--------------------------------------------------
#define DSCR_AI_UNDEF_BCC	\
	((	DIF_STORAGE0  | DIF_DATA_32_BIT_REAL) +	\
	 (	EXTENSION_BIT | VIF_PLAINTEXT) +	\
		0x01 + '?' + \
		VIFE_ERR_NO_DATA_AVAILABLE)

/// read 	(rsp_ud2_data)
// 0x05,0xfc,0x01'?',0x15 	 (Keine Daten vorhanden)
const unsigned char dscr_AI_undef[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_32_BIT_REAL),	\
		(EXTENSION_BIT | VIF_PLAINTEXT),	\
		0x01,'?',	\
		VIFE_ERR_NO_DATA_AVAILABLE};

//--------------------------------------------------
// WIDERSTAND
//--------------------------------------------------
#define DSCR_AI_OHM_BCC	\
		((DIF_STORAGE0  | DIF_DATA_32_BIT_REAL) +	\
		VIF_PLAINTEXT +	0x03 + 'm' + 'h' + 'O')

/// read 	(rsp_ud2_data)
// 0x05,0xfd,0x7c,(in Ohm)
const unsigned char dscr_AI_ohm[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_32_BIT_REAL),	\
		VIF_PLAINTEXT,0x03,'m','h',	'O'};

//--------------------------------------------------
// SPANNUNG 
//--------------------------------------------------
#define DSCR_AI_VOLT_BCC	\
		((DIF_STORAGE0  | DIF_DATA_32_BIT_REAL) +	\
		VIF_LINEAR_EXTENTION_FD + \
		 VIFE_FD_VOLTS_1V)

/// read 	(rsp_ud2_data)
// 0x05,0xfd,0x46 (in Volt)
const unsigned char dscr_AI_volt[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_32_BIT_REAL),	\
		VIF_LINEAR_EXTENTION_FD,\
		VIFE_FD_VOLTS_1V};

//--------------------------------
// DIF_DATA_VARIABLE_LENGTH (SENSOR)
// -------------------------------
#define DSCR_AI_SENSOR_LEN			0x03 
#define DSCR_AI_SENSOR_BCC	\
	(	DIF_DATA_VARIABLE_LENGTH +	\
		VIF_LINEAR_EXTENTION_FD	+ \
		VIFE_FD_NO_VIF )

/// read 	(rsp_ud2_data)
const unsigned char dscr_AI_sensor[] =	\
	{	DIF_DATA_VARIABLE_LENGTH,	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_NO_VIF };


#endif	//MCT_PAA_AI8_MBUS_AI_H_