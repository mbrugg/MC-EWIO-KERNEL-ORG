/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	29.09.2011
 	
 	Description: MODBUS-Command-Implementation for mct_paa_do4-Driver

				01	= FUNC_RD_COILS
				05	= FUNC_WR_SINGLE_COIL
				15	= FUNC_WR_MULTIPLE_COILS
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

 *******************************************************************************/
#ifndef MCT_PAA_DO4_MODBUS_CMD_H_
	#define MCT_PAA_DO4_MODBUS_CMD_H_

#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"

extern int do4_mod_sl2_cmd_rsp_01(struct mod_l2 * l2);
extern int do4_mod_sl2_cmd_rsp_05(struct mod_l2 * l2);
extern int do4_mod_sl2_cmd_rsp_15(struct mod_l2 * l2);
extern int do4_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);


#endif	//MCT_PAA_DO4_MODBUS_CMD_H_