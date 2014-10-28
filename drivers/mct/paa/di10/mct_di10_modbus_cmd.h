/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	05.10.2011
 	
 	Description: MODBUS-Command-Implementation for mct_paa_di10-Driver

				02	= FUNC_RD_DISCRETE_INPUTS
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

 *******************************************************************************/
#ifndef MCT_PAA_DI10_MODBUS_CMD_H_
	#define MCT_PAA_DI10_MODBUS_CMD_H_

#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"

extern int di10_mod_sl2_cmd_rsp_02(struct mod_l2 * l2);
extern int di10_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);


#endif	//MCT_PAA_DI10_MODBUS_CMD_H_