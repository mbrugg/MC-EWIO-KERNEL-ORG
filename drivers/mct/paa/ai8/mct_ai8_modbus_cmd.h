/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		12.10.2011
 	
 	Description: MODBUS-Command-Implementation for mct_paa_ai8-Driver

				04	= FUNC_RD_INPUT_REGISTERS
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

				- Die Parametrierung (Widerstand/Spannung) des AI8 über das 
				Modbus-Interface wird nicht unterstützt!
	
 *******************************************************************************/
#ifndef MCT_PAA_AI8_MODBUS_CMD_H_
	#define MCT_PAA_AI8_MODBUS_CMD_H_

#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"

extern int ai8_mod_sl2_cmd_rsp_04(struct mod_l2 * l2);
extern int ai8_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);

#endif	//MCT_PAA_AI8_MODBUS_CMD_H_