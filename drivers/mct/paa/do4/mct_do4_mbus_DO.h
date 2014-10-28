/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als 
					Digitales-Ausgangs-Modul darstellen.
	
	M-BUS GERÄT:	0	(index=0)
				
	BINARY_1	(OUTPUT)-Register (Storage0, == digitale Ausgänge)
		- read		(Register lesen)
		- write		(Register = Wert, Langform des write/replace)
		- or		(Register data OR old data)
		- and		(Register data AND old data)
		- or		(Register data XOR old data)		toggle bits
		- andnot	(Register NOT data AND old data)	clear bits
		- clear		(Register löschen)

	BINARY_1	(MASK)-Register (Storage1, == Handmaske)
		- read		(Register lesen)
	
	Die Handmaske darf per M-Bus nur gelesen werden, das Löschen ist nicht
	möglich! 

 *******************************************************************************/
#ifndef MCT_PAA_DO4_MBUS_DO_H_
	#define MCT_PAA_DO4_MBUS_DO_H_

#include "../../if/mbus_def.h"

// ------------------------------------------------------------------
// BINARY_1 (OUTPUT)  (entspricht den digitalen Ausgängen)
// ------------------------------------------------------------------
#define DSCR_DO_STANDARD_LEN		0x04
#define DSCR_DO_ACTION_LEN			0x05

#define DSCR_DO_BCC	\
		((DIF_STORAGE0  | DIF_DATA_VARIABLE_LENGTH) +	\
		VIF_LINEAR_EXTENTION_FD +	\
		VIFE_FD_DIGITAL_OUTPUT_BINARY +	\
		LVAR_BINARY_NUMBER_O1_BYTES )

/// read 			(rsp_ud2_data)
/// write/replace 	(standard-mode)
// 0x0d,0xfd,0x1a,0xe1
const unsigned char dscr_DO_standard[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_DIGITAL_OUTPUT_BINARY,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

/// write/replace (long-mode)
// 0x0d,0xfd,0x9a,0x00,0xe1
const unsigned char dscr_DO_write[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		(VIF_LINEAR_EXTENTION_FD),	\
		(EXTENSION_BIT | VIFE_FD_DIGITAL_OUTPUT_BINARY),	\
		VIFE_OA_WRITE_AND_REPLACE,	\
		LVAR_BINARY_NUMBER_O1_BYTES };
/// or
// 0x0d,0xfd,0x9a,0x03,0xe1
const unsigned char dscr_DO_or[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		(VIF_LINEAR_EXTENTION_FD),	\
		(EXTENSION_BIT | VIFE_FD_DIGITAL_OUTPUT_BINARY),	\
		VIFE_OA_OR,	\
		LVAR_BINARY_NUMBER_O1_BYTES };
/// and
// 0x0d,0xfd,0x9a,0x04,0xe1
const unsigned char dscr_DO_and[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		(VIF_LINEAR_EXTENTION_FD),	\
		(EXTENSION_BIT | VIFE_FD_DIGITAL_OUTPUT_BINARY),	\
		VIFE_OA_AND,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

/// xor (toggle bits)
// 0x0d,0xfd,0x9a,0x05,0xe1
const unsigned char dscr_DO_xor[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		(VIF_LINEAR_EXTENTION_FD),	\
		(EXTENSION_BIT | VIFE_FD_DIGITAL_OUTPUT_BINARY),	\
		VIFE_OA_XOR,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

/// and not (clear bits)
// 0x0d,0xfd,0x9a,0x06,0xe1
const unsigned char dscr_DO_andnot[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		(VIF_LINEAR_EXTENTION_FD),	\
		(EXTENSION_BIT | VIFE_FD_DIGITAL_OUTPUT_BINARY),	\
		VIFE_OA_AND_NOT,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

/// clear
// 0x0d,0xfd,0x9a,0x07,0xe1
const unsigned char dscr_DO_clear[] =		\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		VIF_LINEAR_EXTENTION_FD,	\
		(EXTENSION_BIT | VIFE_FD_DIGITAL_OUTPUT_BINARY),	\
		VIFE_OA_CLEAR,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

// ------------------------------------------------------------------
// BINARY_1 (MASK)  (entspricht der Handmaske für Outputs)
// ------------------------------------------------------------------
#define DSCR_MASK_STANDARD_LEN		0x04
#define DSCR_MASK_ACTION_LEN		0x05

#define DSCR_MASK_BCC	\
		((DIF_STORAGE1  | DIF_DATA_VARIABLE_LENGTH) +	\
		VIF_LINEAR_EXTENTION_FD +	\
		VIFE_FD_DIGITAL_INPUT_BINARY +	\
		LVAR_BINARY_NUMBER_O1_BYTES)

/// read 			(rsp_ud2_data)
// 0x4d,0xfd,0x1b,0xe1
const unsigned char dscr_MASK_standard[] =	\
	{	(DIF_STORAGE1 | DIF_DATA_VARIABLE_LENGTH),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_DIGITAL_INPUT_BINARY,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

#endif	//MCT_SPI_DIO_MBUS_DO_H_