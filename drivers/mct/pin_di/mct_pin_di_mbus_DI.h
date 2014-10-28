/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	25.08.2011
 	
 	Description:  	Alle Defines und Beschreiber, die das MBus-Gerät als 
					Digitales-Eingangs-Modul darstellen.
					
					mode_x = MODE_DIRECT
	
					DATA_8		-Register (Storage0, == input_x)
					- read		(Register lesen)
					- clear		nicht implementiert
					Der clear-Befehl für diesen Mode wurde nicht implementiert,
					da ein Signal am PIN bereits nach wenigen ms sofort das 
					Register aktualisiert und auf 1 setzt.
 
 *******************************************************************************/
#ifndef MCT_PIN_DI_MBUS_DI_H_
	#define MCT_PIN_DI_MBUS_DI_H_

#include "../if/mbus_def.h"

// ------------------------------------------------------------------
// DATA_8 - (entspricht dem Treiber-Parameter: input_x)
// ------------------------------------------------------------------
#define DSCR_DI_IMP_STANDARD_LEN		0x04
#define DSCR_DI_IMP_ACTION_LEN			0x05

#define DSCR_DI_IMP_BCC	\
		((DIF_STORAGE0  | DIF_DATA_VARIABLE_LENGTH) +	\
		VIF_LINEAR_EXTENTION_FD +	\
		VIFE_FD_DIGITAL_INPUT_BINARY +	\
		LVAR_BINARY_NUMBER_O1_BYTES)

/// read 			(rsp_ud2_data)
// 0x0d,0xfd,0x1b,0xe1
const unsigned char dscr_DI_imp_standard[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_DIGITAL_INPUT_BINARY,	\
		LVAR_BINARY_NUMBER_O1_BYTES};

#endif	//MCT_PIN_DI_MBUS_DI_H_