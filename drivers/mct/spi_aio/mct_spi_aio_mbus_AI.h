/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als 
					Digitales-Eingangs-Modul darstellen.
	
	M-BUS GERÄTE:	1...4	(index=0 bis 3)

	DATA_32	(INPUT)-Register (Storage0)	analoger Eingang
		- read		(Register lesen)

	23.01.2012		durch Konfiguration möglich:	
		Kanal 0-1: Volt, Ohm, Grad Celsius, unkonfiguriert
		Kanal 0-2: Ampere, unkonfiguriert

 *******************************************************************************/
#ifndef MCT_SPI_AIO_MBUS_AI_H_
	#define MCT_SPI_AIO_MBUS_AI_H_

#include "../if/mbus_def.h"

// ------------------------------------------------------------------
// DATA_32 (INPUT)  (entspricht den analogen Eingängen)
//					als VIF wird noch der Treiber-Parameter in_mode_x
//					angehängt - "evitcani" == "inactive"
// ------------------------------------------------------------------
#define DSCR_AI_UNDEF_LEN	0x05	// Einheit:			= ? 
#define DSCR_AI_OHM_LEN		0x06	// Einheit: VIF_PLAIN		= 1 Ohm
#define DSCR_AI_VOLT_LEN	0x03	// Einheit: VIF_FD 		= 1 Volt 
#define DSCR_AI_TEMP_LEN	0x02	// Einheit: VIF			= 1 °C
#define DSCR_AI_AMPERE_LEN	0x03	// Einheit: VIF_FD		= 1 Ampere

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

//--------------------------------------------------
// STROM
//--------------------------------------------------
#define DSCR_AI_AMPERE_BCC	\
		((DIF_STORAGE0  | DIF_DATA_32_BIT_REAL) +	\
		VIF_LINEAR_EXTENTION_FD + \
		 VIFE_FD_AMPERE_1A)

/// read 	(rsp_ud2_data)
// 0x05,0xfd,0x5c (in Ampere)
const unsigned char dscr_AI_ampere[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_32_BIT_REAL),	\
		VIF_LINEAR_EXTENTION_FD,\
		VIFE_FD_AMPERE_1A};

//--------------------------------------------------
// TEMPERATUR 
//--------------------------------------------------
#define DSCR_AI_TEMP_BCC	\
	(	(DIF_STORAGE0  | DIF_DATA_32_BIT_REAL) +	\
		(VIF_EXTERNAL_TEMPERATURE_nn | DIM_nn_1o0))

/// read 	(rsp_ud2_data)
// 0x05,0xfd,0x67 (in Grad Celsius)
const unsigned char dscr_AI_temp[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_32_BIT_REAL),	\
		(VIF_EXTERNAL_TEMPERATURE_nn | DIM_nn_1o0)};

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


#endif	//MCT_SPI_AIO_MBUS_AI_H_