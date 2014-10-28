/*********************************************************************************
	Copyright 	MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		15.02.2012

 	Description:  MODBUS Command-Implementation for mct_spi_aio-Driver

 *******************************************************************************/
#include "mct_spi_aio_objs.h"			// objects
#include "mct_spi_aio_modbus_cfg.h"		// Konfiguration
#include "../if/modbus_sl2.h"
#include "../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[23] = \
			{1,0,0,3,0,3,'B','T','R',1,6,'M','R','-','A','I','O',2,4,'V','1','.','0'};

#define CREATE_L1_PTR 	struct mod_l1  *  l1  = &(l2->l1)

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID
/// ------------------------------------------------------------------
int aio_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;
	MODBUS_CMD_TRACE("aio_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

	/// PRÜFUNG...
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
/// OVERLOAD Function03		Read Holding Registers
///
/// Only for OUTPUT's !
/// ------------------------------------------------------------------
int aio_mod_sl2_cmd_rsp_03(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR_NULL;
   	unsigned char i, n, p;
	u16 quan;
    s16 val16;

	MODBUS_CMD_TRACE("aio_mod_sl2_cmd_rsp_03()\n");
	
		/// EXCEPTION CODE 03	
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5];
//	printk("QUANTITY: %d <> %d\n", quan,QUANTITY_READ_HOLDING_REGISTERS_MAX_LIMIT);	 
	if ((l1->rec.cnt < 8) ||					// Quantity muss da sein
		(quan == 0) ||							// Quantity Limit's
		(quan > QUANTITY_READ_HOLDING_REGISTERS_MAX_LIMIT))	{
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	/// EXCEPTION CODE 02	
	else if ((l1->rec.buf[2] != 0) ||			// Adr-High muss 0 sein	
			(l1->rec.buf[3] > 3) ||				// Adr-Low muss 0..3 sein
			(quan + l1->rec.buf[3] > 4))	{	// Endadr muss passen
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}	
	else	{
		/// WERT...
		i = l1->rec.buf[3];						// Index = Adresse
		n = l1->rec.buf[5];						// Anzahl Werte
		l1->snd.buf[2] = n << 1;				// Byte-Count
		l1->snd.cnt +=1;						// Pufferlänge erweitern
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC

		for(p=0; p<n; p++)	{

			OBJS_CREATE_ODEV_PTR(i);
			OBJS_ODEV_RD_OUTPUT(odev,val16);
//			printk("VALUE: %d\n",val16);
			i++;
			l1->snd.buf[2*p+3] = (char)((val16 >> 8) & 0xff);	// Register ValueHi	
			l1->snd.buf[2*p+4] = (char)(val16 & 0xff);		// Register ValueLo 	
			mod_sl1_crc(l1->snd.buf[2*p+3],&l1->snd.CrcHi,&l1->snd.CrcLo);
			mod_sl1_crc(l1->snd.buf[2*p+4],&l1->snd.CrcHi,&l1->snd.CrcLo);
		}
		l1->snd.cnt += l1->snd.buf[2];			// Pufferlänge erweitern
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function04		Read Input Registers
///
/// Only for INPUT's !
/// Anzahl der Input Register == 8 Register je 16bit == 4 Inputs je 32bit
/// ------------------------------------------------------------------
int aio_mod_sl2_cmd_rsp_04(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR_NULL;
	unsigned char i,a,p=0;
	uint32_t value32 = 0;

	u16 quan;

	MODBUS_CMD_TRACE("aio_mod_sl2_cmd_rsp_04()\n");

		///	EXCEPTION 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||						// Quantity muss da sein
		(quan == 0) ||								// Quantity Limit's	1...125
		(quan > QUANTITY_READ_INPUT_REGISTERS_MAX_LIMIT)) {
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02
	else if ((l1->rec.buf[2] != 0) ||				// Adr-High muss 0 sein
			(l1->rec.buf[3] > 7) ||					// Adr-Low muss 0...7 sein
			_bit_test(l1->rec.buf[3],0) ||			// Adr-Low gerade	
			_bit_test(l1->rec.buf[5],0)||			// Quantity gerade
			(quan + l1->rec.buf[3] > 8))	{		// Endadr muss passen
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}
	else {
		/// WERTe...
		i = (l1->rec.buf[3]) >> 1;		// Index eines Messwertes 0...7
		a	= (l1->rec.buf[5]) >> 1;	// Anzahl der Messwerte	mit je 2 Register, 16bit
		l1->snd.buf[2] = a << 2;		// Byte Count
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC
		l1->snd.cnt +=1;
		p = 0;
		while(a) {					// gewünschte Anzahl an

			OBJS_CREATE_IDEV_PTR(i);
			OBJS_IDEV_RD_INPUT(idev,value32); // Messwerten holen

			l1->snd.buf[4*p+6] = (value32 & 0xff);			
			l1->snd.buf[4*p+5] = ((value32>>8) & 0xff);
			l1->snd.buf[4*p+4] = ((value32>>16) & 0xff);
			l1->snd.buf[4*p+3] = ((value32>>24) & 0xff);	
			mod_sl1_crc(l1->snd.buf[4*p+3],&l1->snd.CrcHi,&l1->snd.CrcLo);
			mod_sl1_crc(l1->snd.buf[4*p+4],&l1->snd.CrcHi,&l1->snd.CrcLo);
			mod_sl1_crc(l1->snd.buf[4*p+5],&l1->snd.CrcHi,&l1->snd.CrcLo);
			mod_sl1_crc(l1->snd.buf[4*p+6],&l1->snd.CrcHi,&l1->snd.CrcLo);
			l1->snd.cnt +=4;
			p++;
			i++;
			a--;
		}
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function06		Write Single Register
///
/// Only for OUTPUT's !
/// ------------------------------------------------------------------
int aio_mod_sl2_cmd_rsp_06(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR_NULL;
	unsigned char i,p;
    s16 val16;
	
	MODBUS_CMD_TRACE("aio_mod_sl2_cmd_rsp_06()\n");

		///	EXCEPTION 03
	if(l1->rec.cnt != 8)	{						/* Wert muss da sein		*/
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02
	else if ((l1->rec.buf[2] != 0) ||				/* Adr-High muss 0 sein		*/
			(l1->rec.buf[3] > 3))	{				/* Adr-Low muss 0..3 sein	*/
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}
	else {
		/// WERT...
		i = l1->rec.buf[3];						// Index = Adresse
		val16 = l1->rec.buf[4] & 0xff;
		val16 = (val16 << 8) | l1->rec.buf[5];	
		
		OBJS_CREATE_ODEV_PTR(i);
		OBJS_ODEV_WR_OUTPUT_xBit(odev,val16);		/// Intern auf 10/11 bit !
		
		for(p=0; p<4; p++) {
			l1->snd.buf[2+p] = l1->rec.buf[2+p];
			mod_sl1_crc(l1->snd.buf[2+p],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC
		}	// Pufferinhalt zurückschicken
		l1->snd.cnt +=4;
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function16		Write Multiple Registers
///
/// Only for OUTPUT's !
/// ------------------------------------------------------------------
int aio_mod_sl2_cmd_rsp_16(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR_NULL;
   	unsigned char i, n, p;
	u16 quan;
    s16 val16;

	MODBUS_CMD_TRACE("aio_mod_sl2_cmd_rsp_16()\n");

		/// EXCEPTION CODE 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt < 9) ||						/* Byte-Count muss da sein 	*/
		(quan == 0) ||								/* Quantity Limit's	1...123 */
		(quan > QUANTITY_WRITE_MULTIPLE_REGISTERS_MAX_LIMIT) ||	\
		(l1->rec.buf[6] != (quan<<1)) ||			/* Byte-Count stimmt 		*/
		(l1->rec.cnt != l1->rec.buf[6] + 9))	{	/* alle Daten vorhanden 	*/
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	} 	/// EXCEPTION CODE 02
	else if ((l1->rec.buf[2] != 0) ||				/* Adr-High muss 0 sein 	*/
			(l1->rec.buf[3] > 3) ||					/* Adr-Low muss 0..3 sein 	*/
			(quan + l1->rec.buf[3] > 4))	{		/* Endadr muss passen 		*/
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}
	else	{
	/// WERT...
		i = l1->rec.buf[3];							// Index = Adresse
		n = l1->rec.buf[5];							// Anzahl Werte
		for(p=0;p<n;p++)	{ 						// je nach Anzahl ...
			val16 = l1->rec.buf[2*p+7] & 0xff;
			val16 = (val16 << 8) | l1->rec.buf[2*p+8];	

			OBJS_CREATE_ODEV_PTR(i);
			OBJS_ODEV_WR_OUTPUT_xBit(odev,val16);	/// Intern auf 10/11 bit !

			i++;
        }
		// Pufferanfang mit Adresse, Quantity zurückschicken
		for(p=0; p<4; p++) {
			l1->snd.buf[2+p] = l1->rec.buf[2+p];
			mod_sl1_crc(l1->snd.buf[2+p],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC
		}	// Pufferinhalt zurückschicken
		l1->snd.cnt +=4;
	}
	return 1;
};