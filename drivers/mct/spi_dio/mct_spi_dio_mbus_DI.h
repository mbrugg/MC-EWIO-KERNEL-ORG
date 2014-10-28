/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
 	
 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als 
					Digitales-Eingangs-Modul darstellen.
	
	M-BUS GERÄT:	1	(index=0)
			
	BINARY_1	(INPUT)-Register (Storage0)	digitale Eingänge
		- read		(Register lesen)
	
 *******************************************************************************/
#ifndef MCT_SPI_DIO_MBUS_DI_H_
	#define MCT_SPI_DIO_MBUS_DI_H_

#include "../if/mbus_def.h"

// ------------------------------------------------------------------
// BINARY_1BYTE (INPUT)  (entspricht den digitalen Eingängen)
// ------------------------------------------------------------------
#define DSCR_DI_STANDARD_LEN		0x04
#define DSCR_DI_ACTION_LEN			0x05

#define DSCR_DI_BCC	\
		((DIF_STORAGE0  | DIF_DATA_VARIABLE_LENGTH) +	\
		VIF_LINEAR_EXTENTION_FD +	\
		VIFE_FD_DIGITAL_INPUT_BINARY + \
		LVAR_BINARY_NUMBER_O1_BYTES)

/// read 			(rsp_ud2_data)
// 0x0d,0xfd,0x1b,0xe1
const unsigned char dscr_DI_standard[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_DIGITAL_INPUT_BINARY,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

#endif	//MCT_SPI_DIO_MBUS_DI_H_