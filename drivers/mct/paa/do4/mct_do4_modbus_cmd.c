/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	29.09.2011
 	
 	Description: MODBUS-Command-Implementation for mct_paa_do4-Driver

	15.05.2012 	Write Multiple Coils: Grenzwert = 4 Outputs,
			BugFix: bisher war auf 8 Outputs, dadurch keine Modbus-Exception!
			
 *******************************************************************************/
#include "mct_do4_objs.h"			// objects
#include "mct_do4_modbus_cfg.h"		// Konfiguration
#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[23] = \
			{1,0,0,3,0,3,'B','T','R',1,6,'M','R','-','D','O','4',2,4,'V','1','.','0'};

#define CREATE_DRV_PTR 		struct tdo4_drv *drv = (struct tdo4_drv *)(l2->pdata)
#define CREATE_ODEV_PTR		struct tdo4_dev_outp  * odev = drv->Op_Outputs
#define CREATE_L1_PTR 		struct mod_l1  * l1  = &(l2->l1)

// Hinweis: OUTPUT-Feedback-Variable lesen
#define GET_DEV_OUTPUT(val)	{	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	val = odev->Op_Value->P_ValueIn.value;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

#define SET_DEV_OUTPUT(val)	{	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	odev->Op_Value->P_ValueOut.value= val;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

#define GET_DEV_MASK(val)	{	\
	spin_lock_bh(&odev->Op_Mask->lock);	\
	val = odev->Op_Mask->P_Value.value;	\
	spin_unlock_bh(&odev->Op_Mask->lock);}

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID
/// ------------------------------------------------------------------
int do4_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;
	
	MODBUS_CMD_TRACE("do4_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

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
/// OVERLOAD Function01		Read Coils
/// ------------------------------------------------------------------
int do4_mod_sl2_cmd_rsp_01(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;	// ist initialisiert! 
	CREATE_L1_PTR;
	unsigned char a,n,relay,mask;
	u16	quan;

	MODBUS_CMD_TRACE("do4_mod_sl2_cmd_rsp_01()\n");
	
		///	EXCEPTION 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||				/* Quantity muss da sein	*/
		(quan == 0) ||					/* Quantity Limit's 		*/
		(quan > QUANTITY_READ_COILS_MAX_LIMIT))	{
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02				Handschalter werden beachtet!	
	else if	((l1->rec.buf[2] != 0) ||			/* Adr-High muss 0 sein		*/
		 (l1->rec.buf[3] > 3) 	||			/* Adr-Low muss 0..3 sein	*/
		 (l1->rec.buf[3] +  l1->rec.buf[5] > 4))	{/* Endadr muss passen		*/
		    l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	} 
	else	{
	/// WERT...
		a = l1->rec.buf[3];      	// Adresse
		n = l1->rec.buf[5];     	// Anzahl Bits
		GET_DEV_OUTPUT(relay);		// Ausgänge (Feedback) einlesen
		GET_DEV_MASK(mask)		// Handmaske einlesen
//		printk("READ MASK  : 0x%02x\n",mask);	
		mask = mask << 4;
		relay = relay | mask;		// Output- und Handmasken-bits	
		l1->snd.buf[2]	= 1;		// Byte-Count
		if ((a == 0) && (n == 8)) 	// schneller Spezialfall
			l1->snd.buf[3] = relay;	// (alle bits lesen)
		else {						// langsamer Allgemeinfall
			n = ~(0xFF << n);		// Maske
			relay >>= a;			// an Adresse schieben
			l1->snd.buf[3] = relay & n;  // Bits maskiert
		}
//		printk("READ OUTPUT: 0x%02x\n",l1->snd.buf[3]);	
	/// PROTOKOLL...
		// ByteCount (1Byte), Coil-Status (1Byte) zurücksenden
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
	}
	return 1;
};

/// ------------------------------------------------------------------
/// OVERLOAD Function05		Write Single Coil
/// ------------------------------------------------------------------
int do4_mod_sl2_cmd_rsp_05(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;
	CREATE_L1_PTR;
	unsigned char n,relay;

	MODBUS_CMD_TRACE("do4_mod_sl2_cmd_rsp_05()\n");

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
		GET_DEV_OUTPUT(relay);
//		printk("READ OUTPUT: 0x%02x\n",relay);
		n = 1 << l1->rec.buf[3];	// Adresse in Bitposition wandeln
		relay = (relay & ~n) | (l1->rec.buf[4] & n);
//		printk("WRITE OUTPUT: 0x%02x\n",relay);
		SET_DEV_OUTPUT(relay);
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
/// OVERLOAD Function15		Write Multiple Coils
/// ------------------------------------------------------------------
int do4_mod_sl2_cmd_rsp_15(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;
	CREATE_L1_PTR;
	unsigned char a,n,b,relay;
	u16	quan;

	MODBUS_CMD_TRACE("do4_mod_sl2_cmd_rsp_15()\n");
	
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
		GET_DEV_OUTPUT(relay);
//		printk("READ OUTPUT: 0x%02x\n",relay);
		a = l1->rec.buf[3];			// Adresse
        n = l1->rec.buf[5];			// Anzahl Bits
        b = l1->rec.buf[7];			// Bits
		if ((a == 0) && (n == 4))	// schneller Spezialfall
			relay = b & 0x0F;
		else {						// langsamer Allgemeinfall
			n = ~(0xFF << n);		// Maske
			b &= n;					// Bits maskiert
			n <<= a;				// an Adresse schieben
			b <<= a;
			relay = (relay & ~n) | b;
		}
		SET_DEV_OUTPUT(relay);
//		printk("WRITE OUTPUT: 0x%02x\n",relay);
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



