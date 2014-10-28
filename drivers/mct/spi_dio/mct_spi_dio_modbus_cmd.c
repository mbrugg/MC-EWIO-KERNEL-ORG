/*********************************************************************************
	Copyright MCQ TECH GmbH 2012

	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		06.01.2012
	
	Description:  MODBUS Command-Implementation for spi_dio-Driver

	01	= FUNC_RD_COILS			(Lesen max 4 Relais)
	02	= FUNC_RD_DISCRETE_INPUTS	(Lesen max 8 Optokoppler)
	05	= FUNC_WR_SINGLE_COIL		(Relais 0,1,2,3 schalten)
	15	= FUNC_WR_MULTIPLE_COILS	(Relais 0-3 schalten)
	43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

	21.04.12 BugFix Bei FUNC_WR_MULTIPLE_COILS war auf 2 Relais begrenzt!
			- jetzt können alle 4 Relais geschaltete werden
 *******************************************************************************/
#include "mct_spi_dio_objs.h"			// objects
#include "mct_spi_dio_modbus_cfg.h"		// Konfiguration
#include "../if/modbus_sl2.h"
#include "../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[25] = \
			{1,0,0,3,0,3,'S','Y','S',1,8,'M','R','-','D','I','O','8','4',2,4,'V','1','.','0'};

#define CREATE_L1_PTR 		struct mod_l1  *  l1  = &(l2->l1)

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID
/// ------------------------------------------------------------------
int dio_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;
	
	MODBUS_CMD_TRACE("dio_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

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
/// OVERLOAD Function01	Read Coils - is READ ONLY (max. 4 Relais)
/// ------------------------------------------------------------------
int dio_mod_sl2_cmd_rsp_01(struct mod_l2 * l2)	{
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR;	// ist initialisiert! 
	CREATE_L1_PTR;
	unsigned char a,n,relay;
	u16	quan;

	MODBUS_CMD_TRACE("dio_mod_sl2_cmd_rsp_01()\n");
	
		///	EXCEPTION 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||				/* Quantity muss da sein	*/
		(quan == 0) ||					/* Quantity Limit's 		*/
		(quan > QUANTITY_READ_COILS_MAX_LIMIT))	{
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02	
	else if	(	(l1->rec.buf[2] != 0) ||		/* Adr-High muss 0 sein		*/
				(l1->rec.buf[3] > 3) 	||	/* Adr-Low muss 0..3 sein	*/
				(l1->rec.buf[3] +  l1->rec.buf[5] > 4))	{/* Endadr muss passen		*/
			l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	} 
	else	{
	/// WERT...
		a = l1->rec.buf[3];      	// Adresse
		n = l1->rec.buf[5];     	// Anzahl Bits
		OBJS_ODEV_RD_OUTPUT(odev,relay); // Ausgänge einlesen (nur virtuell möglich)

		l1->snd.buf[2]	= 1;		// Byte-Count
		
		n = ~(0xFF << n);		// Bit-Maske
		relay >>=a;			// an Adresse schieben
		l1->snd.buf[3] = relay & n; 	// Bits maskiert
//		printk("READ OUTPUTS: 0x%02x\n",l1->snd.buf[3]);	
	/// PROTOKOLL...
		// ByteCount (1Byte), Coil-Status (1Byte) zurücksenden
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function02	Read Discrete Inputs (max. 8 Optokoppler)
/// ------------------------------------------------------------------
int dio_mod_sl2_cmd_rsp_02(struct mod_l2 * l2)	{
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;
	CREATE_L1_PTR;
	unsigned char a,n,b,m;
	u16 quan;

	MODBUS_CMD_TRACE("dio_mod_sl2_cmd_rsp_02()\n");

		/// EXCEPTION CODE 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||					/* Quantity muss da sein	*/
		(quan == 0) ||						/* Quantity Limit's 		*/
		(quan > QUANTITY_READ_DISCRETE_INPUTS_MAX_LIMIT)){
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	} /// EXCEPTION CODE 02
	else if	(	(l1->rec.buf[2] != 0) ||			/* Adr-High muss 0 sein		*/
				(l1->rec.buf[3] > 7) 	||		/* Adr-Low muss 0..7 sein	*/
				(quan + l1->rec.buf[3]  > 4))	{	/* Endadr muss passen		*/
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	} 
	else	{
	/// WERT...
		OBJS_IDEV_RD_INPUT(idev,b);	// Bits 
		a = l1->rec.buf[3];      	// Adresse
		n = l1->rec.buf[5];     	// Anzahl Bits 
		m = ~(0xFF << n);		// Maske
		b >>= a;			// an Adresse schieben
		b &= m;				// Bits maskiert
//		printk("READ INPUTS: 0x%02x\n",b);	
	/// PROTOKOLL...
		l1->snd.buf[2] = 1;		// 1 Byte-Count
		l1->snd.buf[3] = b;
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function05	Write Single Coil (Relais 0...3)
/// ------------------------------------------------------------------
int dio_mod_sl2_cmd_rsp_05(struct mod_l2 * l2)	{
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR;
	CREATE_L1_PTR;
	unsigned char n,relay;

	MODBUS_CMD_TRACE("dio_mod_sl2_cmd_rsp_05()\n");

		///	EXCEPTION 03
	if((l1->rec.cnt != 8) ||				/* Wert muss da sein		*/
		(l1->rec.buf[5] != 0) ||			/* 0x0000 oder 0xff00 		*/
		((unsigned char)(l1->rec.buf[4]+1) > 1)) {
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02
	else if ((l1->rec.buf[2] != 0) ||			/* Adr-High muss 0 sein		*/
			(l1->rec.buf[3] > 3))	{		/* Adr-Low muss 0..3 sein	*/
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}
	else {
	/// WERT...
		OBJS_ODEV_RD_OUTPUT(odev,relay);
//		printk("GET OUTPUT: 0x%02x\n",relay);
		n = 1 << l1->rec.buf[3];	// Adresse in Bitposition wandeln
		relay = (relay & ~n) | (l1->rec.buf[4] & n);
//		printk("SET OUTPUT: 0x%02x\n",relay);
		OBJS_ODEV_WR_OUTPUT_4Bit(odev,relay);
	/// PROTOKOLL...
		// Adresse zurücksenden (H,L)
		l1->snd.buf[MOD_ADDH_POS] = l1->rec.buf[MOD_ADDH_POS];
		mod_sl1_crc(l1->snd.buf[MOD_ADDH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.buf[MOD_ADDL_POS] = l1->rec.buf[MOD_ADDL_POS];
		mod_sl1_crc(l1->snd.buf[MOD_ADDL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
		// Daten zurücksenden (H,L)
		l1->snd.buf[MOD_DATH_POS] = l1->rec.buf[MOD_DATH_POS];
		mod_sl1_crc(l1->snd.buf[MOD_DATH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.buf[MOD_DATL_POS] = l1->rec.buf[MOD_DATL_POS];
		mod_sl1_crc(l1->snd.buf[MOD_DATL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function15	Write Multiple Coils (max. 4 Relais)
/// ------------------------------------------------------------------
int dio_mod_sl2_cmd_rsp_15(struct mod_l2 * l2)	{
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR;
	CREATE_L1_PTR;
	unsigned char a,n,b,relay;
	u16	quan;

	MODBUS_CMD_TRACE("dio_mod_sl2_cmd_rsp_15()\n");
	
		/// EXCEPTION CODE 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt < 9) ||					/* Byte-Count muss da sein 	*/
		(quan == 0) ||						/* Quantity Limit's		*/
		(quan > QUANTITY_WRITE_MULTIPLE_COILS_MAX_LIMIT) ||	\
		(l1->rec.buf[6] != (quan+7)>>3) ||			/* Byte-Count stimmt 		*/
		(l1->rec.cnt != l1->rec.buf[6] + 9))	{		/* alle Daten vorhanden 	*/
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	} 	/// EXCEPTION CODE 02
	else if ((l1->rec.buf[2] != 0) ||				/* Adr-High muss 0 sein 	*/
			(l1->rec.buf[3] > 3) ||				/* Adr-Low muss 0..3 sein 	*/
			(quan + l1->rec.buf[3] > 4))	{		/* Endadr muss passen 		*/
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	}
	else	{
	/// WERT...
		OBJS_ODEV_RD_OUTPUT(odev,relay);
//		printk("GET OUTPUTS: 0x%02x\n",relay);
		a = l1->rec.buf[3];		// Adresse
        n = l1->rec.buf[5];			// Anzahl Bits
        b = l1->rec.buf[7];			// Bits
		n = ~(0xFF << n);		// Maske
		b &= n;				// Bits maskiert
		n <<= a;			// an Adresse schieben
		b <<= a;
		relay = (relay & ~n) | b;
		OBJS_ODEV_WR_OUTPUT_4Bit(odev,relay);
//		printk("WRITE OUTPUTS: 0x%02x\n",relay);
	/// PROTOKOLL...
		// Adresse zurücksenden (H,L)
		l1->snd.buf[MOD_ADDH_POS] = l1->rec.buf[MOD_ADDH_POS];
		mod_sl1_crc(l1->snd.buf[MOD_ADDH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.buf[MOD_ADDL_POS] = l1->rec.buf[MOD_ADDL_POS];
		mod_sl1_crc(l1->snd.buf[MOD_ADDL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
		// Quantity zurücksenden (H,L)
		l1->snd.buf[MOD_QUAH_POS] = l1->rec.buf[MOD_QUAH_POS];
		mod_sl1_crc(l1->snd.buf[MOD_QUAH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.buf[MOD_QUAL_POS] = l1->rec.buf[MOD_QUAL_POS];
		mod_sl1_crc(l1->snd.buf[MOD_QUAL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
	}
	return 1;
};



