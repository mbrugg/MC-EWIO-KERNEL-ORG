/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		20.09.2011
	
	Description:	MODBUS basics
			Modbus Application Protocol Specification V1.1a 04.06.2004 
			from Modbus-IDA

	Schalter:	... siehe unten ...
			#define/#undef MODBUS_APP_TRACE - Trace Client-Layer App
			#define/#undef MODBUS_SL2_TRACE - Trace Client-Layer 2
			#define/#undef MODBUS_SL1_TRACE - Trace Client-Layer 1

*********************************************************************************/
#ifndef __MODBUS_DEF_H__
#define __MODBUS_DEF_H__

// *** MODBUS-Client-Code-Trace ***
//#define MODBUS_APP_TRACE		// Trace Client-Layer App
//#define MODBUS_SL2_TRACE		// Trace Client-Layer 2
//#define MODBUS_SL1_TRACE		// Trace Client-Layer 1

#undef MODBUS_APP_TRACE			// Trace Client-Layer App
#undef MODBUS_SL2_TRACE			// Trace Client-Layer 2
#undef MODBUS_SL1_TRACE			// Trace Client-Layer 1

#define MOD_A_BROADCAST_NOREPLY	0	// Broadcast ohne Antwort

#define MOD_FLDA_POS		0
#define MOD_FUNC_POS		1	// Function Code
#define MOD_ADDH_POS		2	// Adresse High-Part
#define MOD_ADDL_POS		3	// Adresse Low-Part
#define MOD_DATH_POS		4	// Daten High-Part
#define MOD_DATL_POS		5	// Daten Low-Part

#define MOD_QUAH_POS		4	// Anzahl High-Part
#define MOD_QUAL_POS		5	// Anzahl Low-Part
					// nur FuncCode = 08 (Diagnostic)
#define MOD_SUBH_POS		2	// Subfunction Code High-Part
#define MOD_SUBL_POS		3	// Subfunction Code Low-Part
					// nur FuncCode = 43 Ecapsulation
#define MOD_MEIT_POS		2	// MEI Type
#define MOD_MEIC_POS		3	// MEI Code
#define MOD_MEIO_POS		4	// MEI Object ID
#define MOD_FLDA_LEN		1	// Längenelement 1 Byte Adresse
#define MOD_FUNC_LEN		1	// Längenelement 1 Byte Function	
#define MOD_SUB_LEN		2	// Längenelement 1 Byte Subfunction	
#define MOD_CRC_LEN		2	// Längenelement 2 Byte CRC
// Längenelement: 	Adresse + Functioncode + Subfunctioncode + CrC
#define MOD_FLDA_FUNC_SUB_CRC_LEN	MOD_FLDA_LEN + MOD_FUNC_LEN + MOD_SUB_LEN +  MOD_CRC_LEN


#define MOD_ADU_RS485_MAX	256		
#define MOD_ADU_TCP_MAX		260	

// *** MODBUS Function Code Categories ***
// -----------------------------------------------------
// |	111-127		|	PUBLIC function codes 	[3]
// -----------------------------------------------------
// |	100-110		|	USER function codes 	[2]
// -----------------------------------------------------
// |	 73- 99		|	PUBLIC function codes 	[2]
// -----------------------------------------------------
// |	 65- 72		|	USER function codes 	[1]
// -----------------------------------------------------
// |	  1- 65		|	PUBLIC function codes 	[1]
// -----------------------------------------------------
//


// *** PUBLIC Function Code Definitions [1] ***
//--------------
// DATA	ACCESS	|
//--------------------------------------------------
// BIT-Access	|	Physical Discrete Inputs 	|
//--------------------------------------------------
#define FUNC_RD_DISCRETE_INPUTS		2	// see: Section 6.2
//--------------------------------------------------
// BIT-Access	|	Internal Bits Or Physical Coils	|
//--------------------------------------------------
#define FUNC_RD_COILS			1	// see: Section 6.1
#define FUNC_WR_SINGLE_COIL		5	// see: Section 6.5
#define FUNC_WR_MULTIPLE_COILS		15	// see: Section 6.11
//--------------------------------------------------
// 16 BIT-Access|	Physical Input Registers	|
//--------------------------------------------------
#define FUNC_RD_INPUT_REGISTERS		4	// see: Section 6.4
//--------------------------------------------------
// 16 BIT-Access	|Internal Registers		|
//			|	OR			|
//			|Physical Output Registers	|
//--------------------------------------------------
#define FUNC_RD_HOLDING_REGISTERS	3	// see: Section 6.3
#define FUNC_WR_SINGLE_REGISTER		6	// see: Section 6.6
#define FUNC_WR_MULTIPLE_REGISTERS	16	// see: Section 6.12
#define FUNC_RD_WR_MULTIPLE_REGISTERS	23	// see: Section 6.17
#define FUNC_MASK_WR_REGISTER		22	// see: Section 6.16
#define FUNC_RD_FIFO_QUEUE		24	// see: Section 6.18
//--------------------------------------------------
// FILE-Access	|	Record				|
//--------------------------------------------------
#define FUNC_RD_FILE_RECORD		20	// see: Section 6.14
						// SubCode: 6
#define FUNC_WR_FILE_RECORD		21	// see: Section 6.15
						// SubCode: 6
//--------------------------------------------------
// DIAGNOSTICS						|
//--------------------------------------------------
#define FUNC_RD_EXCEPTION_STATUS	7	// see: Section 6.7 (Serial Line only!)
#define FUNC_DIAGNOSTIC			8	// see: Section 6.7 (Serial Line only!)
						// SubCode: 00-18, 20	separat Table
#define FUNC_GET_COM_EVENT_COUNTER	11	// see: Section 6.9
#define FUNC_GET_COM_EVENT_LOG		12	// see: Section 6.10
#define FUNC_REPORT_SLAVE_ID		17	// see: Section 6.13
#define FUNC_ENCAPSULATED_INTERFACE_TRANSPORT	43	// see: Section 6.19

//--------------------------------------------------
// FUNC: 43  and MEI-Types				|
//--------------------------------------------------
						// SubCode: 14
#define MEI_TYPE_READ_DEVICE_ID		14	// see: Section 6.21
#define MEI_TYPE_CANopen_GENERAL_REFERENCE	13	// see: Section 6.20

#define MEI_READ_DEVICE_ID_CODE_1	1 
#define MEI_READ_DEVICE_ID_CODE_2	2 
#define MEI_READ_DEVICE_ID_CODE_3	3 
#define MEI_READ_DEVICE_ID_CODE_4	4


// *** MODBUS EXCEPTION Codes ***
#define EXC_ILLEGAL_FUNCTION		0x01
#define EXC_ILLEGAL_DATA_ADDRESS	0x02
#define EXC_ILLEGAL_DATA_VALUE		0x03
#define EXC_SLAVE_DEVICE_FAILURE	0x04
#define EXC_ACKNOWLEDGE			0x05
#define EXC_SLAVE_DEVICE_BUSY		0x06
// unused				0x07
#define EXC_MEMORY_PARITY_ERROR		0x08
// unused				0x09
#define EXC_GATEWAY_PATH_UNAVAILABLE	0x0a
#define EXC_GATEWAY_FAILED_RESPOND	0x0b


// *** SUB-Functions Code SERIAL LINE only!***
// list of sub-function-codes supported by serial line devices
#define FUNC_SUB_RETURN_QUERY_DATA			0x00
#define FUNC_SUB_RESTART_COMMUNICATIONS_OPTIONS		0x01
#define FUNC_SUB_RETURN_DIAGNOSTIC_REGISTER		0x02
#define FUNC_SUB_CHANGE_ASCII_INPUT_DELIMITER		0x03
#define FUNC_SUB_FORCE_LISTEN_ONLY_MODE			0x04
// reserved: 						0x05 ... 0x09
#define FUNC_SUB_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER	0x0a
#define FUNC_SUB_RETURN_BUS_MESSAGE_COUNT		0x0b
#define FUNC_SUB_RETURN_BUS_COMMUNICATION_ERROR_COUNT	0x0c
#define FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT	0x0d
#define FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT		0x0e
#define FUNC_SUB_RETURN_SLAVE_NO_RESPONSE_COUNT		0x0f
#define FUNC_SUB_RETURN_SLAVE_NAK_COUNT			0x10
#define FUNC_SUB_RETURN_SLAVE_BUSY_COUNT		0x11
#define FUNC_SUB_RETURN_BUS_CHARACTER_OVERRUN_COUNT	0x12
// reserved:						0x13
#define FUNC_SUB_CLEAR_OVERRUN_COUNTER_AND_FLAG		0x14
// reserved:						0x15 ... 0xffff

// QUANTITY LIMITS
#define QUANTITY_READ_COILS_MAX_LIMIT			0x07d0	// Func01:	2000
#define QUANTITY_READ_DISCRETE_INPUTS_MAX_LIMIT		0x07d0	// Func02:	2000
#define QUANTITY_READ_HOLDING_REGISTERS_MAX_LIMIT	0x7d	// Func03:	 125
#define QUANTITY_READ_INPUT_REGISTERS_MAX_LIMIT		0x7d	// Func04:	 125
								// no quantities Func05...Func14
#define QUANTITY_WRITE_MULTIPLE_COILS_MAX_LIMIT		0x7b0	// Func15:	1968
#define QUANTITY_WRITE_MULTIPLE_REGISTERS_MAX_LIMIT	0x7b	// Func16:   123

#endif // __MODBUS_DEF_H__
