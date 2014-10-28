/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		10.10.2011
 	
 	Description: MODBUS-Command-Implementation for mct_paa_ao4-Driver

 *******************************************************************************/
#include "mct_ao4_objs.h"			// objects
#include "mct_ao4_modbus_cfg.h"		// Konfiguration
#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[23] = \
			{1,0,0,3,0,3,'B','T','R',1,6,'M','R','-','A','O','4',2,4,'V','1','.','0'};

#define CREATE_DRV_PTR 		struct tao4_drv *drv = (struct tao4_drv *)(l2->pdata)
#define CREATE_ODEV_PTR		struct tao4_dev_outp  * odev = NULL
#define CREATE_L1_PTR 		struct mod_l1  *  l1  = &(l2->l1)

// Hier: Geräte 0-3 (OUTPUT's)
#define GET_DEV_OUTPUT16(channel,val)	{	\
	odev = drv->Op_Outputs[channel];	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	val = odev->Op_Value->P_Value.value;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

// Mit Limitüberwachung 10bit
#define SET_DEV_OUTPUT16(channel,val)	{	\
	if(val < 0)	\
		val = 0;	\
	else if (val > SIZE_MAX_10bit)	\
		val = SIZE_MAX_10bit;	\
	odev = drv->Op_Outputs[channel];	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	odev->Op_Value->P_Value.value = val;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID
/// ------------------------------------------------------------------
int ao4_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;
	
	MODBUS_CMD_TRACE("ao4_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

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
/// ------------------------------------------------------------------
int ao4_mod_sl2_cmd_rsp_03(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;
	CREATE_L1_PTR;
   	unsigned char i, n, p;
	u16 quan;
    s16 val16;

	MODBUS_CMD_TRACE("ao4_mod_sl2_cmd_rsp_03()\n");
	
		/// EXCEPTION CODE 03	
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5];
//	printk("QUANTITY: %d <> %d\n", quan,QUANTITY_READ_HOLDING_REGISTERS_MAX_LIMIT);	 
	if ((l1->rec.cnt < 8) ||					/* Quantity muss da sein	*/
		(quan == 0) ||							/* Quantity Limit's 		*/
		(quan > QUANTITY_READ_HOLDING_REGISTERS_MAX_LIMIT))	{
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	/// EXCEPTION CODE 02	
	else if ((l1->rec.buf[2] != 0) ||			/* Adr-High muss 0 sein		*/
			(l1->rec.buf[3] > 3) ||				/* Adr-Low muss 0..3 sein	*/
			(quan + l1->rec.buf[3] > 4))	{	/* Endadr muss passen		*/
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
			GET_DEV_OUTPUT16(i,val16);
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
/// OVERLOAD Function06		Write Single Register
/// ------------------------------------------------------------------
int ao4_mod_sl2_cmd_rsp_06(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;
	CREATE_L1_PTR;
	unsigned char i,p;
    s16 val16;
	
	MODBUS_CMD_TRACE("ao4_mod_sl2_cmd_rsp_06()\n");

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
		
		SET_DEV_OUTPUT16(i,val16);
		
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
/// ------------------------------------------------------------------
int ao4_mod_sl2_cmd_rsp_16(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;
	CREATE_L1_PTR;
   	unsigned char i, n, p;
	u16 quan;
    s16 val16;

	MODBUS_CMD_TRACE("ao4_mod_sl2_cmd_rsp_16()\n");

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
			SET_DEV_OUTPUT16(i,val16);				// die Analogwerte merken
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



