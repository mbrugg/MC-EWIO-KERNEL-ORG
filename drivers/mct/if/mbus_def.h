/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
	
	Description:	MBUS basics

	Schalter:		... siehe unten ...
					#define/#undef MBUS_APP_TRACE - Trace Client-Layer App
					#define/#undef MBUS_CL2_TRACE - Trace Client-Layer 2
					#define/#undef MBUS_CL1_TRACE - Trace Client-Layer 1

	
	23.06.2011  VIFE OBJECT ACTIONS - Tabelle 
	01.07.2011  VIFE RECORD ERRORS - Tabelle 
	19.07.2011	VIF-FD-Table vollständig

*********************************************************************************/
#ifndef __MBUS_DEF_H__
#define __MBUS_DEF_H__

// *** MBUS-Client-Code-Trace ***
#undef MBUS_APP_TRACE		// Trace Client-Layer App
#undef MBUS_CL2_TRACE		// Trace Client-Layer 2
#undef MBUS_CL1_TRACE		// Trace Client-Layer 1
//#define MBUS_APP_TRACE
//#define MBUS_CL2_TRACE
//#define MBUS_CL1_TRACE

// *** C = CONTROL-FIELD ***
// meaning of bits
// F == FCB bit	 (calling	direction)							S == ShortFrame
// A == ACD bit	 (reply		direction)							C == ConrolFrame
// V == FCV bit	 (calling	direction)							L == LongFrame
// D == DFC bit	 (reply		direction)
#define FCB_BIT											1<<5
#define ADC_BIT											1<<5
#define FCV_BIT             							1<<4
#define DFC_BIT             							1<<4
#define C_SND_NKE										0x40	// S	initialisation of slave
#define C_SND_UD										0x43	// L,C	send user data to slave ohne FCV-Bit ohne FCB-Bit
#define C_SND_UD_F										0x63	// L,C  ohne FCV-Bit mit  FCB-Bit
#define C_SND_UD_V										0x53	// L,C	mit  FCV-Bit ohne FCB-Bit
#define C_SND_UD_VF										0x73	// L,C	mit  FCV-Bit mit  FCB-Bit
#define C_REQ_UD2										0x4B	// S	request for class2 data
#define C_REQ_UD2_F										0x6B	// S
#define C_REQ_UD2_V										0x5B	// S
#define C_REQ_UD2_VF									0x7B	// S
#define C_REQ_UD1										0x4A	// S request for class1 data (see alarm protokoll)
#define C_REQ_UD1_F										0x6A	// S
#define C_REQ_UD1_V										0x5A	// S	
#define C_REQ_UD1_VF									0x7A	// S
#define C_RSP_UD										0x08	// L,C	data transfer from slave to
#define C_RSP_UD_A										0x18	// L,C	lbnsp.	master after request
#define C_RSP_UD_D										0x28	// L,C	lbnsp.
#define C_RSP_UD_AD										0x38	// L,C	lbnsp.


// *** A = ADDRESS-FIELD ***
// address field: serves to address the recipient in calling direction and
// identify the sender of the recieving direction
#define A_UNCONFIGURED									0x00	// unconfigured slaves given these
// 0x01 ... 0xfa	 frame of adresses 1 ... 250 reserved for slaves
#define A_FUTURE_USE2									0xfb	// for future applictions
#define A_FUTURE_USE1									0xfc	// for future applictions
#define A_NETWORK_LAYER									0xfd	// adressing peformed in network layer
#define A_BROADCAST_REPLY								0xfe	// all of the slaves reply
#define A_BROADCAST_NOREPLY								0xff	// none of the slaves reply


// *** CI = CONTROL INFORMATION-FIELD ***
// meaning of bits
// M	== MODE2 bit, for future applications not recommends
// REC	== direction master --> slave		
// SND	== direction slave  --> master

#define MODE_BIT										1<<2
//________________________________________________________________________________________________________
//		   DIR						MODE2							STATE	Definition in:
//_______________________________________________________________________________________________________
													// 0x00-0x4F	// unused     			
#define CI_REC_APPLICATION_RESET					0x50			// usergroup march 1994
#define CI_REC_DATA_SEND							0x51			// EN1434-3
#define CI_REC_SELECTION_OF_SLAVES					0x52			// usergroup july  1993 
													// 0x53			// unused
#define CI_REC_SYNCRONIZE_ACTION					0x54			// suggestion
#define CI_REC_DATA_SEND_M							0x55			// not recom.EN1434-3
#define CI_REC_SELECTION_OF_SLAVES_M				0x56			// not recom.usergroup july  1993
													// 0x57-0x6F	// unused
#define CI_SND_GENERAL_APPL_ERRORS					0x70			// usergroup march 1994
#define CI_SND_REPORT_ALARM_STATUS					0x71			// usergroup march 1994
#define CI_SND_VARIABLE_DATA						0x72			// EN1434-3
#define CI_SND_FIXED_DATA							0x73			// EN1434-3
													// 0x74			// unused
													// 0x75			// unused
#define CI_SND_VARIABLE_DATA_M						0x76			// EN1434-3
#define CI_SND_FIXED_DATA_M							0x77			// EN1434-3
													// 0x90-0x97	// obsolete
													// 0x98-0xAF	// unused
#define CI_REC_MANUFACTURER_EEPROM_BYTE_WRITE		0xB0 			// userspecified RIA (temporary)	
#define CI_REC_MANUFACTURER_EEPROM_BYTE_VERIFY		0xB1 			// userspecified RIA (temporary)
#define CI_REC_SEND_USER_DATA_						0xB2			// suggestion	Techem
#define CI_REC_INITIALIZE_TEST_CALIBRATION_MODE		0xB3			// usergroup july  1993
#define CI_REC_EEPROM_READ							0xB4			// suggestion 	Techem
													// 0xB5			// unused
#define CI_REC_START_SOFTWARE_TEST					0xB6			// suggestion	Techem
													// 0xB7			// unused
#define CI_REC_SET_BAUDRATE_00300					0xB8			// usergroup july  1993
#define CI_REC_SET_BAUDRATE_00600					0xB9			// usergroup july  1993
#define CI_REC_SET_BAUDRATE_01200					0xBA			// usergroup july  1993
#define CI_REC_SET_BAUDRATE_02400					0xBB			// usergroup july  1993
#define CI_REC_SET_BAUDRATE_04800					0xBC			// usergroup july  1993
#define CI_REC_SET_BAUDRATE_09600					0xBD			// usergroup july  1993
#define CI_REC_SET_BAUDRATE_19200					0xBE			// suggestion
#define CI_REC_SET_BAUDRATE_38400					0xBF			// suggestion
													// 0xC0-0xFF	// unused 
// _____________________________________________________________________________________________________


// --- CI_REC_APPLICATION RESET  + one of these bytes ---
#define APPLICATION_RESET_ALL							0x00
#define APPLICATION_RESET_USER_DATA						0x10
#define APPLICATION_RESET_SIMPLE_BILLING				0x20
#define APPLICATION_RESET_ENHANCED_BILLING				0x30
#define APPLICATION_RESET_MULTI_TARIF_BILLING			0x40
#define APPLICATION_RESET_INSTANEOUS_VALUES				0x50
#define APPLICATION_RESET_LOAD_MANAGEMENT_VALUES		0x60
														// 0x70 RESERVED	
#define APPLICATION_RESET_INSTALLATION_AND_STARTUP		0x80
#define APPLICATION_RESET_TESTING						0x90
#define APPLICATION_RESET_CALIBRATION					0xa0
#define APPLICATION_RESET_MANUFACTURING					0xb0
#define APPLICATION_RESET_DEVELOPMENT					0xc0
#define APPLICATION_RESET_SELFTEST						0xd0
														// 0xe0	RESERVED
														// 0xf0	RESERVED

// ________________________________________________________________________________________________
//
// --- TABLES for FIXED DATA STRUCTURE ---
// ________________________________________________________________________________________________

// To identify the fixed data structure, the CI-numbers of
// 0x73/0x77	== CI_SND_FIXED_DATA / CI_SND_FIXED_DATA_M are used
//
// Fixed data structure in reply direction!
//
//	-----------------------------------------------------------------------
// | Identific.No | AccessNo |	Status | Medium/Unit | Counter1 | Counter2 |
//  -----------------------------------------------------------------------
// | 4Byte		   | 1Byte	  | 1Byte  | 2Byte		 | 4Byte	| 4Byte	   |
//	-----------------------------------------------------------------------
//		
// - Identification Number is a serial number allocated during manufactuere, coded with 8 BCD packed
//	 digits (4 Byte), and wich runs from 00000000-99999999.	
// - Access Number has unsigned binary coding, and is increased by one after each RSP_UD from slave.
// - Status various information about the status of counters and fault with have occured.	

// --- STATUS of FIXED DATA STRUCTURE ---
#define FIXED_DATA_STATUS_COUNTER_1_2_CODED_SIGNED_BINARY	1<<0
#define FIXED_DATA_STATUS_COUNTER_1_2_CODED_BCD				0x00
#define FIXED_DATA_STATUS_COUNTER_1_2_STORED_AT_FIXED_DATE	1<<1
#define FIXED_DATA_STATUS_COUNTER_1_2_ARE_ACTUAL_VALUES		0x00
#define FIXED_DATA_STATUS_POWER_LOW						    1<<2
#define FIXED_DATA_STATUS_PERMANENT_ERROR					1<<3
#define FIXED_DATA_STATUS_TEMPORARY_ERROR					1<<4
#define FIXED_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT5		1<<5
#define FIXED_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT6		1<<6
#define FIXED_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT7		1<<7

// --- UNIT/MEDIUM coding of FIXED DATA STRUCTURE
//	 --------------------------------------------------------------------------------------
//  | BYTE | ByteNo.8 (byte 2 of Medium/Unit	   |  ByteNo.7 (byte 2 of Medium/Unit	   |
//	 --------------------------------------------------------------------------------------
//	| BIT  | 16 | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |
//	 --------------------------------------------------------------------------------------
//		   | Medium  | physical unit of counter2   | Medium  | physical unit of counter1   |
//			-------------------------------------------------------------------------------
//		   |MSB |	 |MSB |	   |	|	 |	  |	   |MSB |    |MSB |    |    |    |    |    |
//			-------------------------------------------------------------------------------	

#define MED_BIT_OFF			0x00
#define MED_BIT16_ON		1<<15
#define MED_BIT15_ON		1<<14
#define MED_BIT8_ON			1<<7
#define MED_BIT7_ON			1<<6
												// BIT16		BIT15		BIT8		BIT7
#define FIXED_DATA_MEDIUM_OTHER					MED_BIT_OFF  | MED_BIT_OFF  | MED_BIT_OFF | MED_BIT_OFF
#define FIXED_DATA_MEDIUM_OIL					MED_BIT_OFF  | MED_BIT_OFF  | MED_BIT_OFF | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_ELECTRICITY			MED_BIT_OFF  | MED_BIT_OFF  | MED_BIT8_ON | MED_BIT_OFF
#define FIXED_DATA_MEDIUM_GAS					MED_BIT_OFF  | MED_BIT_OFF  | MED_BIT8_ON | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_HEAT         			MED_BIT_OFF  | MED_BIT15_ON | MED_BIT_OFF | MED_BIT_OFF
#define FIXED_DATA_MEDIUM_STEAM					MED_BIT_OFF  | MED_BIT15_ON | MED_BIT_OFF | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_HOT_WATER				MED_BIT_OFF  | MED_BIT15_ON | MED_BIT8_ON | MED_BIT_OFF
#define FIXED_DATA_MEDIUM_WATER					MED_BIT_OFF  | MED_BIT15_ON | MED_BIT8_ON | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_HEAT_COAST_ALLOC		MED_BIT16_ON | MED_BIT_OFF  | MED_BIT_OFF | MED_BIT_OFF
// reserved                                     MED_BIT16_ON | MED_BIT_OFF  | MED_BIT_OFF | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_GAS_MODE2				MED_BIT16_ON | MED_BIT_OFF  | MED_BIT8_ON | MED_BIT_OFF
#define FIXED_DATA_MEDIUM_HEAT_MODE2			MED_BIT16_ON | MED_BIT_OFF  | MED_BIT8_ON | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_HOT_WATER_MODE2		MED_BIT16_ON | MED_BIT15_ON | MED_BIT_OFF | MED_BIT_OFF
#define FIXED_DATA_MEDIUM_WATER_MODE2			MED_BIT16_ON | MED_BIT15_ON | MED_BIT_OFF | MED_BIT7_ON
#define FIXED_DATA_MEDIUM_HEAT_COAST_ALLOC_2    MED_BIT16_ON | MED_BIT15_ON | MED_BIT8_ON | MED_BIT_OFF
// reserved                                 	MED_BIT16_ON | MED_BIT15_ON | MED_BIT8_ON | MED_BIT7_ON

// __________________________________________________________________________________________________
//
// --- TABLES for VARIABLE DATA STRUCTURE ---
// __________________________________________________________________________________________________

// To identify the variable data structure, the CI-numbers of
// 0x72/0x76	== CI_SND_VARIABLE_DATA / CI_SND__VARIABLE_DATA_M are used
//
// Variable data structure in reply direction!
//
//	-------------------------------------------------------------------------------
// | Fixed Data Header | Variable Data Blocks (Records) | MDH | Mfg. specific data |
//  -------------------------------------------------------------------------------
// | 12Byte            |	   variable number           |1Byte| variable number   |
//  -------------------------------------------------------------------------------
//

// --- FIXED DATA HEADER of VARIABLE DATA STRUCTURE ---
//  -----------------------------------------------------------------------
// | Ident.No | Manufr. | Version | Medium | AccessNo | Status | Signature |
//	-----------------------------------------------------------------------
// | 4Byte    | 2Byte   | 1Byte   | 1Byte  | 1Byte	   | 1Byte	| 2Byte    |
//	-----------------------------------------------------------------------
// - Identification Number is a serial number allocated during manufactuere, coded with 8 BCD packed
//   digits (4 Byte), and wich runs from 00000000-99999999.	
//   It can be present at fabrication time with a unique number, but could be changeable afterwards,
//   especially if in adition an unique and not changeable fabrication number (DIF = 0x0C, VIF = 0x78)
//   is provided
// - Access Number the same as in fixed data structure.
// - Manufacturer is coded unsigned binary with 2 bytes. This is calculated like EN 61107 manufacturer ID
//   (three uppercase letters)
//        
//   IEC 870 Man. ID = [ASCII (1. letter) - 64] * 32 * 32
//                   + [ASCII (1. letter) - 64] * 32
//                   + [ASCII (1. letter) - 64]
// - Version specifies the generation or version of these counter and depends on manufacturer.
// - Medium is coded of a hole byte.
// - Status field are used to indicate application errors. Apart from this, the significance of
//    the individual bits of the status field is the same as that of fixed data structure.
// - Signature reserved for future encryption applications. (Until that allocate the value 0x00 0x00)

// --- MEDIUM of FIXED DATA HEADER of VARIABLE DATA STRUCTURE
#define VARIABLE_DATA_MEDIUM_OTHER						0x00
#define VARIABLE_DATA_MEDIUM_OIL						0x01
#define VARIABLE_DATA_MEDIUM_ELECTRICITY				0x02
#define VARIABLE_DATA_MEDIUM_GAS						0x03
#define VARIABLE_DATA_MEDIUM_HEAT_OUTLET				0x04
#define VARIABLE_DATA_MEDIUM_STEAM						0x05
#define VARIABLE_DATA_MEDIUM_HOT_WATER					0x06
#define VARIABLE_DATA_MEDIUM_WATER						0x07
#define VARIABLE_DATA_MEDIUM_HEAT_COAST_ALLOCATOR		0x08
#define VARIABLE_DATA_MEDIUM_COMPRESSED_AIR				0x09
#define VARIABLE_DATA_MEDIUM_COOLING_LOAD_METER_OUTLET	0x0a
#define VARIABLE_DATA_MEDIUM_COOLING_LOAD_METER_INLET	0x0b
#define VARIABLE_DATA_MEDIUM_HEAT_INLET					0x0c
#define VARIABLE_DATA_MEDIUM_HEAT_COOLING_LOAD_METER	0x0d	
#define VARIABLE_DATA_MEDIUM_BUS_SYSTEM					0x0e
#define VARIABLE_DATA_MEDIUM_UNKNOWN_MEDIUM				0x0f
// reserved												0x10 ... 0x15
#define VARIABLE_DATA_MEDIUM_COLD_WATER					0x16
#define VARIABLE_DATA_MEDIUM_DUAL_WATER					0x17
#define VARIABLE_DATA_MEDIUM_PRESSURE					0x18
#define VARIABLE_DATA_MEDIUM_AD_CONVERTER				0x19
// reserved												0x20 ... 0xff

// --- STATUS of FIXED DATA HEADER of VARIABLE DATA STRUCTURE --
#define STATUS_BIT0										1<<0
#define STATUS_BIT1										1<<1
#define VARIABLE_DATA_STATUS_POWER_LOW					FIXED_DATA_STATUS_POWER_LOW
#define VARIABLE_DATA_STATUS_PERMANENT_ERROR			FIXED_DATA_STATUS_PERMANENT_ERROR
#define VARIABLE_DATA_STATUS_TEMPORARY_ERROR			FIXED_DATA_STATUS_TEMPORARY_ERROR
#define VARIABLE_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT5	FIXED_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT5
#define VARIABLE_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT6	FIXED_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT6
#define VARIABLE_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT7	FIXED_DATA_STATUS_MANUFACTURER_SPECIFIC_BIT7

/*
#define VARIABLE_DATA_STATUS_NO_ERROR(state)	\
	{	(state) &= (~STATUS_BIT1);	\
		(state) &= (~STATUS_BIT0);	}

#define VARIABLE_DATA_STATUS_APPLICATION_BUSY(state)	\
	{	(state) &= (~STATUS_BIT1);	\
		(state) |= STATUS_BIT0; 	}

#define VARIABLE_DATA_STATUS_ANY_APPLICATION_ERROR(state)	\
	{	(state) |= STATUS_BIT1;	\
    (state) &= (~STATUS_BIT0);	}
*/


// --- SIGNATURE of FIXED DATA HEADER of VARIABLE DATA STRUCTURE --
// ... add what's needed


// --- RECORD of VARIABLE DATA STRUCTURE ---
//
//  Structure of Data Record (transmitted from left to right)
//
//	----------------------------------------------------------------------
// | DIF   | DIFE              | VIF   | VIFE	           | DATA         |
//  ----------------------------------------------------------------------
// | 1Byte | 0-10 (1Byte each) | 1Byte | 0-10 (1Byte each) | 0-N Byte     |
//	----------------------------------------------------------------------
// | DataInformationBlock DIB  | ValueInformationBlock VIB |
//  ------------------------------------------------------- 
// |        Data Record Header DRH                         |
//  -------------------------------------------------------
//
// Each data record contains one value with its description as shown - a
// data record, witch consists of data record header (DRH) and the actual data.
// The DRH in turn consists of the data information block (DIB) to describe 
// the length, type and coding the data, and the value information block (VIB)
// to give the value of the unit and the multiplier.

// --- DIF/DIFE/VIF/VIFE  SUMMARY of RECORD of VARIABLE DATA STRUCTURE
#define EXTENSION_BIT                                   1<<7


// --- DIF-FIELD of RECORD of VARIABLE DATA STRUCTURE
//
//  Coding of the data information field (DIF)!
//
//	-------------------------------------------------------------------
// | BIT7       | BIT6       | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0 |
//  -------------------------------------------------------------------
// | Extension  |LSB of sto- | Function    |    Data Field             | 
// | Bit        |rage number | Field       | Length and coding of Data |
//  -------------------------------------------------------------------
//
#define DIF_STORAGE0		0x00
#define DIF_STORAGE1		0x40

//
//  Coding of the function field!
//
#define DIF_BIT_OFF								0x00
#define DIF_BIT5_ON								1<<4
#define DIF_BIT4_ON								1<<3
#define DIF_FUNCTION_INSTANTANEOUS_VALUE		DIF_BIT_OFF | DIF_BIT_OFF
#define DIF_FUNCTION_MAXIMUM_VALUE				DIF_BIT_OFF | DIF_BIT4_ON
#define DIF_FUNCTION_MINIMUM_VALUE				DIF_BIT5_ON | DIF_BIT_OFF
#define DIF_FUNCTION_VALUE_DURING_ERROR_STATE	DIF_BIT5_ON | DIF_BIT4_ON

// 
// Coding of data field!
//
// For detailed description of data types see appendix of MBUS document. 
// BCD      = Type A
// Integer  = Type B
// Real     = Type H 
#define DIF_DATA_NO_DATA					  	0x00	// no data
#define DIF_DATA_8__BIT_INT						0x01	// 8	bit integer	
#define DIF_DATA_16_BIT_INT						0x02	// 16	bit integer 
#define DIF_DATA_24_BIT_INT						0x03	// 24	bit integer
#define DIF_DATA_32_BIT_INT						0x04	// 32	bit integer
#define DIF_DATA_32_BIT_REAL					0x05	// 32	bit real
#define DIF_DATA_48_BIT_INT						0x06	// 48	bit integer
#define DIF_DATA_64_BIT_INT						0x07	// 64	bit integer
#define DIF_DATA_SEL_FOR_READOUT	 			0x08
#define DIF_DATA_2__DIG_BCD						0x09	// 2	digit bcd	
#define DIF_DATA_4__DIG_BCD						0x0a	// 4	digit bcd	
#define DIF_DATA_6__DIG_BCD						0x0b	// 6	digit bcd	
#define DIF_DATA_8__DIG_BCD						0x0c	// 8	digit bcd		
#define DIF_DATA_VARIABLE_LENGTH				0x0d
#define DIF_DATA_12_DIG_BCD						0x0c	// 12	digit bcd
#define DIF_DATA_SPECIAL_FUNCTIONS				0x0f	// see high-nibble	

// Coding of data field (DIF_DATA_VARIABLE_LENGTH) 
//
// with data field DIF_D_VARIABLE_LENGTH serveral length can be used. The length of
// data is given by the first byte of data:
//
// LVAR = 0x00 ... 0xbf			ASCII string with LVAR characters
// LVAR = 0xc0 ... 0xcf			positive BCD number with 	(LVAR-0xc0) * 2digits
// LVAR = 0xd0 ... 0xdf			negative BCD number with 	(LVAR-0xd0) * 2digits
// LVAR = 0xe0 ... 0xef			binary number with 		 	(LVAR-0xe0) bytes
// LVAR = 0xf0 ... 0xfa			floating point number with	(LVAR-0xf0) bytes [to be defined]
// LVAR = 
#define LVAR_BINARY_NUMBER_WITH_ASCII			0x00
#define LVAR_BINARY_NUMBER_WITH_POSITIVE_BCD	0xc0
#define LVAR_BINARY_NUMBER_WITH_NEGATIVE_BCD	0xd0
#define LVAR_BINARY_NUMBER_WITH_BYTES			0xe0
#define LVAR_BINARY_NUMBER_00_BYTES	    		LVAR_BINARY_NUMBER_WITH_BYTES
#define LVAR_BINARY_NUMBER_O1_BYTES     		LVAR_BINARY_NUMBER_WITH_BYTES + 1
#define LVAR_BINARY_NUMBER_O2_BYTES     		LVAR_BINARY_NUMBER_WITH_BYTES + 2
#define LVAR_BINARY_NUMBER_O3_BYTES     		LVAR_BINARY_NUMBER_WITH_BYTES + 3
#define LVAR_BINARY_NUMBER_O4_BYTES     		LVAR_BINARY_NUMBER_WITH_BYTES + 4
#define LVAR_BINARY_NUMBER_O5_BYTES     		LVAR_BINARY_NUMBER_WITH_BYTES + 5
#define LVAR_BINARY_NUMBER_O6_BYTES     		LVAR_BINARY_NUMBER_WITH_BYTES + 6
#define LVAR_BINARY_NUMBER_WITH_FLOATING_POINT	0xf0


// Coding of data field (DIF_DATA_SPECIAL_FUNCTIONS) 
#define DIF_MANUFACTURER_SPECIFIC_DATA_RECORDS		0x0f
#define DIF_MANUFACTURER_SPECIFIC_DATA_MORE_RECORDS	0x1f
#define DIF_IDLE_FILLER								0x2f
// reserved 										0x3f...0x6f
#define DIF_GLOBAL_READOUT_REQUEST					0x7f	


// --- DIFE-FIELD of RECORD of VARIABLE DATA STRUCTURE
//
//  Coding of the data information field extension (DIFE)!
//
//	-----------------------------------------------------------------
// | BIT7       | BIT6     | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0 |
//  -----------------------------------------------------------------
// | Extension  | (Device) | Tariff    	 |  Storage Number           | 
// | Bit        | | Unit   | 			 |							 |	
//  -----------------------------------------------------------------
//
#define DIFE_UNIT1		0x40
#define DIFE_UNIT2		0x80,0x40
#define DIFE_UNIT3		0xC0,0x40
#define DIFE_UNIT4		0x80,0x80,0x40
#define DIFE_UNIT5		0xC0,0x80,0x40
#define DIFE_UNIT5_BCC  0xC0+0x80+0x40 

#define DIF_STORAGE2	0x01
#define DIF_STORAGE3	0x02
#define DIF_STORAGE4	0x03
#define DIF_STORAGE5	0x04
#define DIF_STORAGE6	0x05
#define DIF_STORAGE7	0x06
#define DIF_STORAGE8	0x07
#define DIF_STORAGE9	0x08

//-----------------------------------------------------------------
//	Coding of Ranges if nn BIT0 and BIT1 only Dimension-Identifier's
//-----------------------------------------------------------------
#define DIM_nn_1o0							0x03	// 1.0
#define DIM_nn_0o1							0x02	// 0.1
#define DIM_nn_0o01							0x01	// 0.01
#define DIM_nn_0o001						0x00	// 0.001

//-----------------------------------------------------------------
//	Coding of Ranges if nn BIT0 and BIT1 only Time-Identifier's
//-----------------------------------------------------------------
#define DIM_nn_seconds						0x00
#define DIM_nn_minutes						0x01
#define DIM_nn_hours						0x02
#define DIM_nn_days							0x03

//-----------------------------------------------------------------
//	Coding of Ranges if pp BIT0 and BIT1 only Time-Identifier's
//-----------------------------------------------------------------
#define DIM_pp_hours						0x00
#define DIM_pp_days							0x01
#define DIM_pp_months						0x02
#define DIM_pp_years						0x03


// --- VIF-FIELD of RECORD of VARIABLE DATA STRUCTURE
//
//  Coding of the value information field (VIF)!
//
//	-------------------------------------------------------------------
// | BIT7       | BIT6       | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0 |
//  -------------------------------------------------------------------
// | Extension  |LSB of sto- | Function    |    Data Field             | 
// | Bit        |rage number | Field       | Length and coding of Data |
//  -------------------------------------------------------------------
//	not all implemented ... nnn Dimensions!

	//-------------------------------------------
	// integral values
	//-------------------------------------------
//#define VIF_PRIMARY							0x00	// Primary Type's ...appendix 8
#define VIF_ENERGY_Wh_nnn						0x00	// 0.001 Wh to 10.000 Wh
#define VIF_ENERGY_J_nnn						0x08	// 0.001 kJ to 10.000 kJ
#define VIF_VOLUME_m3_nnn						0x10	// 0.001 l  to 10.000 l
#define VIF_MASS_kg_nnn							0x18	// 0.001 kg to 10.000 kg
#define VIF_ON_TIME_SECONDS						0x20 | DIM_nn_seconds
#define VIF_ON_TIME_MINUTES						0x20 | DIM_nn_minutes
#define VIF_ON_TIME_HOURS						0x20 | DIM_nn_hours
#define VIF_ON_TIME_DAYS						0x20 | DIM_nn_days
#define VIF_OPERATING_TIME_SECONDS				0x24 | DIM_nn_seconds
#define VIF_OPERATING_TIME_MINUTES				0x24 | DIM_nn_minutes
#define VIF_OPERATING_TIME_HOURS				0x24 | DIM_nn_hours
#define VIF_OPERATING_TIME_DAYS					0x24 | DIM_nn_days
	//-------------------------------------------
	// averaged values
	//-------------------------------------------
#define VIF_POWER_W_nnn							0x28	// 0.001 W to 10.000 W
#define VIF_POWER_J_h_nnn						0x30	// 0.001 kJ/h to 10.000 kJ/h
#define VIF_VOLUME_FLOW_m3_h_nnn				0x38	// 0.001 l/h to 10.000 l/h
#define VIF_VOLUME_FLOW_EXT_m3_min_nnn			0x40	// 0.001 l/min to 10.000 l/min
#define VIF_VOLUME_FLOW_EXT_m3_sec_nnn			0x48	// 0.001 l/s to 10.000 l/s
#define VIF_MASS_FLOW_kg_h_nnn					0x50	// 0.001 kg/h to 10.000 kg/h
	//-------------------------------------------
	// instantaneous values
	//-------------------------------------------
#define VIF_FLOW_TEMPERATURE_C_nn				0x58	// 0.000°C to 1°C
#define VIF_RETURN_TEMPERATURE_C_nn				0x5C	// 0.000°C to 1°C
#define VIF_TEMPERATURE_DIFFERENCE_K_nn			0x60	// 1 mK to 1000 mK
#define VIF_EXTERNAL_TEMPERATURE_nn				0x64	// 0.000°C to 1°C
#define VIF_PRESSURE_bar_nn						0x68	// 1 mbar to 1000 mbar
#define VIF_TIMEPOINT_TYPE_G					0x6c	// Time
#define VIF_TIMEPOINT_TYPE_F					0x6d	// Date + Time
#define VIF_UNITS_FOR_HCA						0x6e	// Heat Coast Allocator
//... Reserved									0x6f 
	//-------------------------------------------
	// parameters
	//-------------------------------------------
#define VIF_AVERAGING_DURATION_SECONDS			0x70 | DIM_nn_seconds
#define VIF_AVERAGING_DURATION_MINUTES			0x70 | DIM_nn_minutes
#define VIF_AVERAGING_DURATION_HOURS			0x70 | DIM_nn_hours
#define VIF_AVERAGING_DURATION_DAYS				0x70 | DIM_nn_days
#define VIF_ACTUALITY_DURATION					0x74 | DIM_nn_seconds	
#define VIF_ACTUALITY_DURATION_MINUTES			0x74 | DIM_nn_minutes
#define VIF_ACTUALITY_DURATION_HOURS			0x74 | DIM_nn_hours
#define VIF_ACTUALITY_DURATION_DAYS				0x74 | DIM_nn_days
#define VIF_FABRICATION_NUMBER					0x78
#define VIF_PRIMARY_IDENTIFICATION_RECORD		0x79	// DIF=0x0c == IdentNo/DIF=0x07 == complete
#define VIF_PRIMARY_ADDRESS_RECORD				0x7a	// DIF = 0x01 
												// 0x7b (nicht verwendet weil tabelle 0xFB?)	
#define VIF_PLAINTEXT							0x7c
												// 0x7d (nicht verwendet weil tabelle 0xFD?)	
#define VIF_ANY									0x7e
#define VIF_MANUFACTURER						0x7f
// ...
#define VIF_LINEAR_EXTENTION_FD					0xfd
#define VIF_LINEAR_EXTENTION_FB					0xfb
// ...

// **********************
/// **** VIFE  - Table
// **********************
// ... full implemented
												//0x00 Reservations .... 
												//0x1F  for object actions (master to slave)
												//		or for error codes (slave  to master)
#define VIFE_PER_SECOND							0x20	// pro Sekunde			[s]
#define VIFE_PER_MINUTE							0x21	// pro Minute			[min]
#define VIFE_PER_HOUR							0x22	// pro Stunde			[h]
#define VIFE_PER_DAY							0x23	// pro Tag			
#define VIFE_PER_WEEK							0x24	// pro Woche						
#define VIFE_PER_MONTH							0x25	// pro Monat
#define VIFE_PER_YEAR							0x26	// pro Jahr
#define VIFE_PER_REVOLUTION_OR_MEASUREMENT		0x27	// pro Runde/Messung
#define VIFE_INCREMENT_PER_INPUT_PULSE_CHAN_0	0x28	// on input channel 0	
#define VIFE_INCREMENT_PER_INPUT_PULSE_CHAN_1	0x29	// on input channel 1	
#define VIFE_INCREMENT_PER_OUTPUT_PULSE_CHAN_0	0x2a	// on output channel 0
#define VIFE_INCREMENT_PER_OUTPUT_PULSE_CHAN_1	0x2b	// on output channel 1
#define VIFE_PER_L								0x2c	// pro Liter			[l]
#define VIFE_PER_M3								0x2d	// pro Kubikmeter		[m³]
#define VIFE_PER_KG								0x2e	// pro Kilogramm		[kg]
#define VIFE_PER_K								0x2f	// pro Kelvin			[K]
#define VIFE_PER_KWH							0x30	// pro Kilowattstunde 	[kWh]
#define VIFE_PER_GJ								0x31	// pro Gigajoule 		[GJ]
#define VIFE_PER_KW								0x32	// pro Kilowatt			[kW]
#define VIFE_PER_KL								0x33	// pro Kelvin*Liter 	[K*l]
#define VIFE_PER_V								0x34	// pro Volt				[V]
#define VIFE_PER_A								0x35	// pro Ampere			[A]
#define VIFE_MULTIPLIED_BY_SEK					0x36
#define VIFE_MULTIPLIED_BY_SEK_PER_VOLT			0x37
#define VIFE_MULTIPLIED_BY_SEK_PER_AMPERE		0x38
#define VIFE_START_DATE_TIME_OF					0x39
#define VIFE_VIF_CONTAINS_UNCORRECTED_UNIT		0x3a	// _instead_of_corrected_unit
#define VIFE_ACCUMULATION_ONLY_IF_POSITIVE		0x3b	// _contributions
#define VIFE_ACCUMULATION_OF_ABS_VALUE			0x3c	// _if_only_negative_contributions
//... Reserved									0x3d 
//... Reserved									0x3e 
//... Reserved									0x3f 

#define VIFE_LOWER_LIMIT_VALUE					0x40
#define VIFE_OF_EXEEDS_OF_LOWER_LIMIT			0x41
#define VIFE_DATE_TIME_OF_BEGIN_OF_FIRST_LOWER	0x42
#define VIFE_DATE_TIME_OF_END_OF_FIRST_LOWER	0x43
//... Reserved									0x44
//... Reserved									0x45
#define VIFE_DATE_TIME_OF_BEGIN_OF_LAST_LOWER	0x46
#define VIFE_DATE_TIME_OF_END_OF_LAST_LOWER		0x47

#define VIFE_UPPER_LIMIT_VALUE					0x48
#define VIFE_OF_EXEEDS_OF_UPPER_LIMIT			0x49
#define VIFE_DATE_TIME_OF_BEGIN_OF_FIRST_UPPER	0x4a
#define VIFE_DATE_TIME_OF_END_OF_FIRST_UPPER	0x4b
//... Reserved									0x4c
//... Reserved									0x4d
#define VIFE_DATE_TIME_OF_BEGIN_OF_LAST_UPPER	0x4e
#define VIFE_DATE_TIME_OF_END_OF_LASE_UPPER		0x4f

#define VIFE_DURATION_OF_FIRST_LOWER_SECONDS	0x50 | DIM_nn_seconds
#define VIFE_DURATION_OF_FIRST_LOWER_MINUTES	0x50 | DIM_nn_minutes
#define VIFE_DURATION_OF_FIRST_LOWER_HOURS		0x50 | DIM_nn_hours
#define VIFE_DURATION_OF_FIRST_LOWER_DAYS		0x50 | DIM_nn_days
#define VIFE_DURATION_OF_LAST_LOWER_SECONDS		0x54 | DIM_nn_seconds
#define VIFE_DURATION_OF_LAST_LOWER_MINUTES		0x54 | DIM_nn_minutes
#define VIFE_DURATION_OF_LAST_LOWER_HOURS		0x54 | DIM_nn_hours
#define VIFE_DURATION_OF_LAST_LOWER_DAYS		0x54 | DIM_nn_days
#define VIFE_DURATION_OF_FIRST_UPPER_SECONDS	0x58 | DIM_nn_seconds
#define VIFE_DURATION_OF_FIRST_UPPER_MINUTES	0x58 | DIM_nn_minutes
#define VIFE_DURATION_OF_FIRST_UPPER_HOURS		0x58 | DIM_nn_hours
#define VIFE_DURATION_OF_FIRST_UPPER_DAYS		0x58 | DIM_nn_days
#define VIFE_DURATION_OF_LAST_UPPER_SECONDS		0x5c | DIM_nn_seconds
#define VIFE_DURATION_OF_LAST_UPPER_MINUTES		0x5c | DIM_nn_minutes
#define VIFE_DURATION_OF_LAST_UPPER_HOURS		0x5c | DIM_nn_hours
#define VIFE_DURATION_OF_LAST_UPPER_DAYS		0x5c | DIM_nn_days
#define VIFE_DURATION_OF_FIRST_SECONDS			0x60 | DIM_nn_seconds
#define VIFE_DURATION_OF_FIRST_MINUTES			0x60 | DIM_nn_minutes
#define VIFE_DURATION_OF_FIRST_HOURS			0x60 | DIM_nn_hours
#define VIFE_DURATION_OF_FIRST_DAYS				0x60 | DIM_nn_days
#define VIFE_DURATION_OF_LAST_SECONDS			0x64 | DIM_nn_seconds
#define VIFE_DURATION_OF_LAST_MINUTES			0x64 | DIM_nn_minutes
#define VIFE_DURATION_OF_LAST_HOURS				0x64 | DIM_nn_hours
#define VIFE_DURATION_OF_LAST_DAYS				0x64 | DIM_nn_days
//... Reserved									0x68
//... Reserved									0x69
#define VIFE_DATE_TIME_BEGIN_OF_FIRST			0x6a
#define VIFE_DATE_TIME_END_OF_FIRST				0x6b
//... Reserved									0x6c
//... Reserved									0x6d
#define VIFE_DATE_TIME_BEGIN_OF_LAST			0x6e
#define VIFE_DATE_TIME_END_OF_LAST				0x6f
// 0.000000 ...1 	0x70-0x77
#define VIFE_MULTIPLICATIVE_CORRECTION_CONSTANT_nnn 0x70 
// 0.001...1  		0x78-0x7b 
#define VIFE_ADDITIVE_CORRECTION_CONSTANT_nn	0x78
//... Reserved									0x7c
#define VIFE_MULTIPLICATIVE_CORRECTION_FACTOR	0x7d	// * 1000
#define VIFE_FUTURE_VALUE						0x7e
/// use  VIF_MANUFACTURER


/// VIFE OBJECT ACTIONS  (Master To Slave)
// ... full implemented
#define VIFE_OA_WRITE_AND_REPLACE				0x00
#define VIFE_OA_ADD								0x01
#define VIFE_OA_SUB								0x02
#define VIFE_OA_OR								0x03
#define VIFE_OA_AND								0x04
#define VIFE_OA_XOR								0x05	// toggle bits
#define VIFE_OA_AND_NOT							0x06	// clear bits
#define VIFE_OA_CLEAR							0x07
#define VIFE_OA_ADD_ENTRY						0x08
#define VIFE_OA_DELETE_ENTRY					0x09
// reserved										0x0a
#define VIFE_OA_FREEZE							0x0b
#define VIFE_OA_ADD_TO_READOUTLIST				0x0c
#define VIFE_OA_DELETE_FROM_READOUTLIST			0x0d
//reserved										0x0e
//reserved										0x0f


/// VIFE RECORD ERRORS	(Slave To Master)						Error-Group
// ... full implemented
#define VIFE_ERR_NONE							0x00	// DIF-Errors
#define VIFE_ERR_TO_MANY_DIFES					0x01
#define VIFE_ERR_STORAGE_NUMBER_NOT_IMPLEMENTED	0x02
#define VIFE_ERR_UNIT_NUMBER_NOT_IMPLEMENTED	0x03
#define VIFE_ERR_TARIF_NUMBER_NOT_IMPLEMENTED	0x04
#define VIFE_ERR_FUNCTION_NOT_IMPLEMENTED		0x05
#define VIFE_ERR_DATA_CLASS_NOT_IMPLEMENTED		0x06
#define VIFE_ERR_DATA_SIZE_NOT_IMPLEMENTED		0x07
// reseved 										0x08 ... 0x0A
#define VIFE_ERR_TO_MANY_VIFES					0x0b	// VIF-Errors
#define VIFE_ERR_ILLEGAL_VIF_GROUP				0x0c
#define VIFE_ERR_ILLEGAL_VIF_EXPONENT			0x0d
#define VIFE_ERR_VIF_DIF_MISMATCH				0x0e
#define VIFE_ERR_UNIMPLEMENTED_ACTION			0x0f
// reserved										0x10 ... 0x14 
#define VIFE_ERR_NO_DATA_AVAILABLE				0x15	// Data-Errors
#define VIFE_ERR_DATA_OVERFLOW					0x16
#define VIFE_ERR_DATA_UNDERFLOW					0x17
#define VIFE_ERR_DATA_ERROR						0x18
// reserved 									0x19 ... 0x1b
#define VIFE_ERR_PREMATURE_END_OF_RECORD		0x1c	// Other-Errors
// reserved 0x1d ... 0x1f


// **********************
/// **** VIFE  FB - Table
// **********************
// ... not all implemented
// ...
#define VIFE_FB_EXTERNAL_TEMPERATURE_nn			0x64	// External Temp  0.001°F to 1°F
// ...

// **********************
/// **** VIFE  FD - Table
// **********************
// full implemented
	//-------------------------------------------
	// Currency Units
	//-------------------------------------------
#define  VIFE_FD_CREDIT_OF_NOMINAL_LOCAL_0001			0x00 | DIM_nn_0o001
#define  VIFE_FD_CREDIT_OF_NOMINAL_LOCAL_001			0x00 | DIM_nn_0o01
#define  VIFE_FD_CREDIT_OF_NOMINAL_LOCAL_01				0x00 | DIM_nn_0o1
#define  VIFE_FD_CREDIT_OF_NOMINAL_LOCAL_1				0x00 | DIM_nn_1o0
#define  VIFE_FD_DEBIT_OF_NOMINAL_LOCAL_0001			0x00 | DIM_nn_0o001
#define  VIFE_FD_DEBIT_OF_NOMINAL_LOCAL_001				0x00 | DIM_nn_0o01
#define  VIFE_FD_DEBIT_OF_NOMINAL_LOCAL_01				0x00 | DIM_nn_0o1
#define  VIFE_FD_DEBIT_OF_NOMINAL_LOCAL_1				0x00 | DIM_nn_1o0
#define  VIFE_FD_ACCES_NUMBER							0x08	// transmission count
	//-------------------------------------------
	// Enhanced Identification
	//-------------------------------------------
#define  VIFE_FD_MEDIUM									0x09	// as in fixed header
#define  VIFE_FD_MANUFACTURER							0x0a	// as in fixed header
#define  VIFE_FD_PARAMETER_SET_IDENTIFICATION			0x0b
#define  VIFE_FD_MODEL_VERSION							0x0c
#define  VIFE_FD_HARDWARE_VERSION						0x0d
#define  VIFE_FD_FIRMWARE_VERSION						0x0e
#define  VIFE_FD_SOFTWARE_VERSION						0x0f
	//-------------------------------------------
	// Implement of all TC294 requirements
	//-------------------------------------------
#define  VIFE_FD_CUSTOMER_LOCATION						0x10
#define  VIFE_FD_CUSTOMER								0x11
#define  VIFE_FD_ACCESS_CODE_USER						0x12
#define  VIFE_FD_ACCESS_CODE_OPERATOR					0x13
#define  VIFE_FD_ACCESS_CODE_SYSTEM_OPERATOR			0x14
#define  VIFE_FD_ACCESS_CODE_DEVELOPER					0x15
#define  VIFE_FD_PASSWORD								0x16
#define  VIFE_FD_ERROR_FLAGS_BINARY						0x17
#define  VIFE_FD_ERROR_MASK								0x18
//... Reserved											0x19
#define  VIFE_FD_DIGITAL_OUTPUT_BINARY					0x1a
#define  VIFE_FD_DIGITAL_INPUT_BINARY					0x1b
#define  VIFE_FD_BAUDRATE								0x1c
#define  VIFE_FD_RESPONSE_DELAY_TIME					0x1d	// bittimes
#define  VIFE_FD_RETRY									0x1e
//... Reserved											0x1f
	//-------------------------------------------
	// Enhanced storage management
	//-------------------------------------------
#define  VIFE_FD_FIRST_STORAGE_FOR_CYCLIC_STORAGE		0x20
#define  VIFE_FD_LAST_STORAGE_FOR_CYCLIC_STORAGE		0x21
#define  VIFE_FD_SIZE_OF_STORAGE_BLOCK					0x22
//... Reserved											0x23	
#define  VIFE_FD_STORAGE_INTERVAL_SECONDS				0x24 | DIM_nn_seconds
#define  VIFE_FD_STORAGE_INTERVAL_MINUTES				0x24 | DIM_nn_minutes
#define  VIFE_FD_STORAGE_INTERVAL_HOURS					0x24 | DIM_nn_hours
#define  VIFE_FD_STORAGE_INTERVAL_DAYS					0x24 | DIM_nn_days
#define  VIFE_FD_STORAGE_INTERVAL_MONTHS				0x28
#define  VIFE_FD_STORAGE_INTERVAL_YEARS					0x29
//... Reserved											0x2a	
//... Reserved											0x2b
#define  VIFE_FD_DUARTION_SINCE_LAST_READOUT_SECONDS	0x2c | DIM_nn_seconds
#define  VIFE_FD_DUARTION_SINCE_LAST_READOUT_MINUTES	0x2c | DIM_nn_minutes
#define  VIFE_FD_DUARTION_SINCE_LAST_READOUT_HOURS		0x2c | DIM_nn_hours
#define  VIFE_FD_DUARTION_SINCE_LAST_READOUT_DAYS		0x2c | DIM_nn_days
	//-------------------------------------------
	// Enhanced tarif management 
	//-------------------------------------------
#define  VIFE_FD_START_DATE_TIME_OF_TARIFF				0x30
#define  VIFE_FD_DURATION_OF_TARIFF_MINUTES				0x30 | DIM_nn_minutes
#define  VIFE_FD_DURATION_OF_TARIFF_HOURS				0x30 | DIM_nn_hours
#define  VIFE_FD_DURATION_OF_TARIFF_DAYS				0x30 | DIM_nn_days
#define  VIFE_FD_PERIOD_OF_TARIFFS_SECONDS				0x34 | DIM_nn_seconds
#define  VIFE_FD_PERIOD_OF_TARIFFS_MINUTESS				0x34 | DIM_nn_minutes
#define  VIFE_FD_PERIOD_OF_TARIFFS_HOURS				0x34 | DIM_nn_hours
#define  VIFE_FD_PERIOD_OF_TARIFFS_DAYS					0x34 | DIM_nn_days
#define  VIFE_FD_PERIOD_OF_TARIFFS_MONTH				0x38
#define  VIFE_FD_PERIOD_OF_TARIFFS_YEARS				0x39
#define  VIFE_FD_NO_VIF									0x3a
//... Reserved											0x3b	
//... Reserved											0x3c-3f
	//-------------------------------------------
	// Electrical  units 
	//-------------------------------------------
#define VIFE_FD_VOLTS_1nV								0x40
#define VIFE_FD_VOLTS_10nV								0x41
#define VIFE_FD_VOLTS_100nV								0x42
#define VIFE_FD_VOLTS_1yV								0x43
#define VIFE_FD_VOLTS_10yV								0x44
#define VIFE_FD_VOLTS_100yV								0x45
#define VIFE_FD_VOLTS_1mV								0x46
#define VIFE_FD_VOLTS_10mV								0x47
#define VIFE_FD_VOLTS_100mV								0x48
#define VIFE_FD_VOLTS_1V								0x49
#define VIFE_FD_VOLTS_10V								0x4a
#define VIFE_FD_VOLTS_100V								0x4b
#define VIFE_FD_VOLTS_1kV								0x4c
#define VIFE_FD_VOLTS_10kV								0x4d
#define VIFE_FD_VOLTS_100kV								0x4e
#define VIFE_FD_VOLTS_1MV								0x4f
#define VIFE_FD_AMPERE_1pA								0x50
#define VIFE_FD_AMPERE_10pA								0x51
#define VIFE_FD_AMPERE_100pA							0x52
#define VIFE_FD_AMPERE_1nA								0x53
#define VIFE_FD_AMPERE_10nA								0x54
#define VIFE_FD_AMPERE_100nA							0x55
#define VIFE_FD_AMPERE_1yA								0x56
#define VIFE_FD_AMPERE_10yA								0x57
#define VIFE_FD_AMPERE_100yA							0x58
#define VIFE_FD_AMPERE_1mA								0x59
#define VIFE_FD_AMPERE_10mA								0x5a
#define VIFE_FD_AMPERE_100mA							0x5b
#define VIFE_FD_AMPERE_1A								0x5c
#define VIFE_FD_AMPERE_10A								0x5d
#define VIFE_FD_AMPERE_100A								0x5e
#define VIFE_FD_AMPERE_1kA								0x5f

#define VIFE_FD_RESET_COUNTER							0x60
#define VIFE_FD_CUMULATION_COUNTER						0x61
#define VIFE_FD_CONTROL_SIGNAL							0x62
#define VIFE_FD_DAY_OF_WEEK								0x63
#define VIFE_FD_WEEK_NUMBER								0x64
#define VIFE_FD_TIMEPOINT_OF_DAY_CHANGE					0x65
#define VIFE_FD_STATE_OF_PARAMETER_ACTIVATION			0x66
#define VIFE_FD_CONTROL_SPECIAL_SUPPLIER_INFORMATION	0x67
#define VIFE_FD_DURATION_SINCE_LAST_CUMULATION_HOURS	0x68 | DIM_pp_hours
#define VIFE_FD_DURATION_SINCE_LAST_CUMULATION_DAYS		0x68 | DIM_pp_days
#define VIFE_FD_DURATION_SINCE_LAST_CUMULATION_MONTHS	0x68 | DIM_pp_months
#define VIFE_FD_DURATION_SINCE_LAST_CUMULATION_YEARS	0x68 | DIM_pp_years
#define VIFE_FD_OPERATING_TIME_BATTERY_HOURS			0x6c | DIM_pp_hours
#define VIFE_FD_OPERATING_TIME_BATTERY_DAYS				0x6c | DIM_pp_days
#define VIFE_FD_OPERATING_TIME_BATTERY_MONTHS			0x6c | DIM_pp_months
#define VIFE_FD_OPERATING_TIME_BATTERY_YEARS			0x6c | DIM_pp_years
#define VIFE_FD_DATE_AND_TIME_OF_BATTERY_CHANGE			0x70
//... Reserved											0x71 ... 0x7f	



	
//  GENERAL_APPL_ERRORS (CI 0x70h + Data)
#define GENERAL_APPL_ERRORS_UNSPECIFIED					0x00
#define GENERAL_APPL_ERRORS_UNIMPLEMENTED_CI			0x01
#define GENERAL_APPL_ERRORS_BUFFER_TO_LONG				0x02
#define GENERAL_APPL_ERRORS_TO_MANY_RECORDS				0x03
#define GENERAL_APPL_ERRORS_PREMATURE_END_OF_RECORDS	0x04
#define GENERAL_APPL_ERRORS_MORE_10_DIFS				0x05
#define GENERAL_APPL_ERRORS_MORE_10_VIFS				0x06
#define GENERAL_APPL_ERRORS_RESERVED					0x07
#define GENERAL_APPL_ERRORS_APPL_TO_BUSY				0x08
#define GENERAL_APPL_ERRORS_TO_MANY_READOUTS			0x09



// *** IEC 870-5-1 "Transmission frame formats" ***
// 
// FT1.2  = Format Telegramm 1.2 mit einer Hammingdistance von 4
#define FT_1_2_FIX								0x10	// Festes Telegramm		= short frame
#define FT_1_2_VAR								0x68	// Variables Telegramm	= long/control frame
#define FT_1_2_STP								0x16	// Stopzeichen			= frame ende
#define FT_1_2_CONTROL1							0xe5	// Einzelsteuerzeichen	= confirm
#define FT_1_2_CONTROL2							0xa2	// Einzelsteuerzeichen	= z. Zeit unbenutzt!

// define some generic Data-Types
#define UI3										3
#define UI4										4
#define UI5										5
#define UI6										6
#define UI7										7
#define B1										1

// *** IEC 870-5-4 "Definition and coding of application information elements!"***
// Mark = used
//              Type 1       = Positive ganze Zahl           (UI)            (en. Unsigned integer)
//              Type 1.1     = Positive ganze Dualzahl       (UIi)           (en. Unsigned binary)
//  x           Type 1.2     = Positive ganze BCD-Zahl       (nUI4[1...4])   (en. Unsigned binary coded decimal integer)    
//
//

// COMPOUND 32 Bit, TYPE F
// elements:
#define CP32_TYPE_F_MIN_BITS			UI6						
#define CP32_TYPE_F_HOUR_BITS			UI5 
#define CP32_TYPE_F_DAY_BITS			UI5
#define CP32_TYPE_F_MONTH_BITS 			UI4
#define CP32_TYPE_F_YEAR_PART1_BITS		UI3			// part1 + part2 = UI7 !!
#define CP32_TYPE_F_YEAR_PART2_BITS		UI4
#define CP32_TYPE_F_TV_BITS				B1
#define CP32_TYPE_F_SU_BITS				B1
#define CP32_TYPE_F_RES1_BITS			B1
#define CP32_TYPE_F_RES2_BITS			B1
#define CP32_TYPE_F_RES3_BITS			B1

// positions:		Attention modified for index access
#define CP32_TYPE_F_MIN_POS			 1-1		//  1... 6						
#define CP32_TYPE_F_HOUR_POS		 9-1 		//  9...13
#define CP32_TYPE_F_DAY_POS			17-1		// 17...21
#define CP32_TYPE_F_MONTH_POS		25-1		// 25...28
#define CP32_TYPE_F_YEAR_PART1_POS	22-1		// 22...24
#define CP32_TYPE_F_YEAR_PART2_POS	29-1		// 29...32
#define CP32_TYPE_F_TV_POS			 8-1		//  8		1= time valid, 0 = time invalid	
#define CP32_TYPE_F_SU_POS			16-1		// 16		1= summer time 0 = standard time 
#define CP32_TYPE_F_RES1_POS		 7-1		//  7		unused
#define CP32_TYPE_F_RES2_POS		14-1		// 14		unused
#define CP32_TYPE_F_RES3_POS		15-1		// 15		unused

// Coding of Data Records
// M-BUS Rev. 4.8  data types in application layer
#define	DATA_TYPE_E_LEN 			2		// (TYPE_E)  
#define DATA_TYPE_F_LEN 			4		// (TYPE_F)
#define	DATA_TYPE_G_LEN 			2		// (TYPE_G)  
#define	DATA_TYPE_H_LEN 			4		// (TYPE_H)
// 	0x0A ... 255 reserved  


typedef struct
{
	unsigned char	fld[DATA_TYPE_F_LEN];				// aktuelle Sekunde	
} CP32_TYPE_F;

																		
#endif // __MBUS_DEF_H__
