/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		16.01.2012

 	Description: MODBUS-Command-Implementation for mct_pin_di-Driver

 *******************************************************************************/
#ifndef MCT_PIN_DI_MODBUS_CMD_H_
	#define MCT_PIN_DI_MODBUS_CMD_H_

#include "../if/modbus_sl2.h"
#include "../if/modbus_app.h"

extern int di_mod_sl2_cmd_rsp_02(struct mod_l2 * l2);
extern int di_mod_sl2_cmd_rsp_04(struct mod_l2 * l2);
extern int di_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);


#endif	//MCT_PIN_DI_MODBUS_CMD_H_