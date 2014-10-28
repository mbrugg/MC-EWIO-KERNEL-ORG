/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		16.01.2012
 	
 	Description: MODBUS-Command-Implementation for mct_pin_di-Driver

				02	= FUNC_RD_DISCRETE_INPUTS
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

 *******************************************************************************/
#include "mct_pin_di_objs.h"			// objects
#include "mct_pin_di_modbus_cfg.h"
#include "../if/modbus_sl2.h"
#include "../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[23] = \
	{1,0,0,3,0,3,'S','Y','S',1,6,'M','R','-','P','I','N',2,4,'V','1','.','0'};

#define CREATE_L1_PTR 	struct mod_l1  *  l1  = &(l2->l1)

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID

/// ------------------------------------------------------------------
int di_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;

	MODBUS_CMD_TRACE("di_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

	// Object-ID muss da sein und muss 0 sein!	
	if((l1->rec.cnt != 7) || (l1->rec.buf[MOD_MEIO_POS] != 0))	{
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}	 // Read-Device-ID-Code 
	else if (code != 1)	{
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}
	else {	
		// Antwort in den Puffer
		// 0: Adresse, 1: Funktion, 2: MEI, 3: Read Dev ID Code
		for (i=0; i < sizeof(id_code); i++) {
			l1->snd.buf[4+i] = id_code[i];
			mod_sl1_crc(l1->snd.buf[MOD_ADDL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		}
		l1->snd.cnt +=sizeof(id_code);
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function02		Read Discrete Inputs
/// 
/// Only - Direkt-Mode
/// ------------------------------------------------------------------
int di_mod_sl2_cmd_rsp_02(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;
	unsigned int mode = 0;		// Input,Counter
	unsigned char a,n,b;
	u16 quan;
	MODBUS_CMD_TRACE("di_mod_sl2_cmd_rsp_02()\n");

	/// MODUL-MODE ermitteln
	OBJS_IDEV_RD_MODE(idev,mode);

	if(mode == MODE_DIRECT )	{
		MODBUS_CMD_TRACE("mode: direkt\n");
		/// EXCEPTION CODE 03
		quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
		if(	(l1->rec.cnt != 8) ||					/* Quantity muss da sein	*/
			(quan == 0) ||						/* Quantity Limit's 		*/
			(quan > QUANTITY_READ_DISCRETE_INPUTS_MAX_LIMIT)){
			l2->exc = EXC_ILLEGAL_DATA_VALUE;
		} /// EXCEPTION CODE 02
		else if	(	(l1->rec.buf[2] != 0) ||			/* Adr-High muss 0 sein		*/
					(l1->rec.buf[3] != 0) 	||		/* Adr-Low muss 0 sein		*/
					(quan + l1->rec.buf[3]  > 1))	{	/* Endadr muss passen		*/
			l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
		} 
		else	{
			/// WERT...
			OBJS_IDEV_RD_VALUE(idev,b);	// Bits 
			a = l1->rec.buf[3];      	// Adresse
			n = l1->rec.buf[5];     	// Anzahl Bits 
			b &= 0x01;			// 1 Bit
			/// PROTOKOLL...
			l1->snd.buf[2] = 1;		// 1 Byte-Count
			l1->snd.buf[3] = b;
			mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
			mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
			l1->snd.cnt +=2;
		}
	}
	else {
		MODBUS_CMD_TRACE("mode: S0 - TODO!!!\n");
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function04		Read Input Registers
/// 
/// Only - S0-Mode
/// 4 Register (16 bit) ==  64bit S0
/// ------------------------------------------------------------------
int di_mod_sl2_cmd_rsp_04(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;
	uint64_t value64 = 0;
	u16 quan;

	MODBUS_CMD_TRACE("di_mod_sl2_cmd_rsp_04()\n");
		///	EXCEPTION 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||				// Quantity muss da sein
		(quan == 0) ||					// Quantity Limit's	1...125
		(quan > QUANTITY_READ_INPUT_REGISTERS_MAX_LIMIT)) {
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02
	else if ((l1->rec.buf[2] != 0) ||			// Adr-High muss 0 sein
			(l1->rec.buf[3] > 0) ||			// Adr-Low muss 0 sein
			(l1->rec.buf[5] != 4) ||		// Quantity muß 4 sein
			(quan + l1->rec.buf[3] > 4))	{	// Endadr muss auch passen
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}
	else {	//
		OBJS_IDEV_RD_VALUE(idev,value64);
		l1->snd.buf[2] = 8;		// Byte Count
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC
		l1->snd.cnt +=1;
		l1->snd.buf[10] = (value64 & 0xff);			
		l1->snd.buf[9] = ((value64>>8) & 0xff);
		l1->snd.buf[8] = ((value64>>16) & 0xff);
		l1->snd.buf[7] = ((value64>>24) & 0xff);	
		l1->snd.buf[6] = ((value64>>32) & 0xff);	
		l1->snd.buf[5] = ((value64>>40) & 0xff);	
		l1->snd.buf[4] = ((value64>>48) & 0xff);	
		l1->snd.buf[3] = ((value64>>56) & 0xff);	
		mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[4],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[5],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[6],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[7],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[8],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[9],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[10],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=8;
	}
	return 1;
}