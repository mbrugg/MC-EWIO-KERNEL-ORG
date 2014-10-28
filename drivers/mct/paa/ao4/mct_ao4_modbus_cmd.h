/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		10.10.2011
 	
 	Description:  MODBUS-Command-Implementation for mct_paa_ao4-Driver

				03	= FUNC_RD_HOLDING_REGISTERS
				06	= FUNC_WR_SINGLE_REGISTER
				16	= FUNC_WR_MULTIPLE_REGISTERS
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

 *******************************************************************************/
#ifndef MCT_PAA_AO4_MODBUS_CMD_H_
	#define MCT_PAA_AO4_MODBUS_CMD_H_

#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"

extern int ao4_mod_sl2_cmd_rsp_03(struct mod_l2 * l2);
extern int ao4_mod_sl2_cmd_rsp_06(struct mod_l2 * l2);
extern int ao4_mod_sl2_cmd_rsp_16(struct mod_l2 * l2);
extern int ao4_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);


#endif	//MCT_PAA_AO4_MODBUS_CMD_H_