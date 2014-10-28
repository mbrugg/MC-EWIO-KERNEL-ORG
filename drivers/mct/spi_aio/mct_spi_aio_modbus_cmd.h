/*********************************************************************************
Copyright 	MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		15.02.2012

 	Description:  MODBUS Command-Implementation for mct_spi_aio-Driver

				03	= FUNC_RD_HOLDING_REGISTERS
				04	= FUNC_RD_INPUT_REGISTERS
				06	= FUNC_WR_SINGLE_REGISTER
				16	= FUNC_WR_MULTIPLE_REGISTERS
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

				- Die Parametrierung (Widerstand/Spannung) des AIO über das 
				Modbus-Interface wird nicht unterstützt!
	
 *******************************************************************************/
#ifndef MCT_SPI_AIO_MODBUS_CMD_H_
	#define MCT_SPI_AIO_MODBUS_CMD_H_

#include "../if/modbus_sl2.h"
#include "../if/modbus_app.h"

extern int aio_mod_sl2_cmd_rsp_04(struct mod_l2 * l2);
extern int aio_mod_sl2_cmd_rsp_03(struct mod_l2 * l2);
extern int aio_mod_sl2_cmd_rsp_06(struct mod_l2 * l2);
extern int aio_mod_sl2_cmd_rsp_16(struct mod_l2 * l2);
extern int aio_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);

#endif	//MCT_SPI_AIO_MODBUS_CMD_H_