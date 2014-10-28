/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		12.10.2011
 	
 	Description:  MODBUS-Command-Implementation for mct_paa_ai8-Driver
	
 *******************************************************************************/
#include "mct_ai8_objs.h"			// objects
#include "mct_ai8_modbus_cfg.h"		// Konfiguration
#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[23] = \
			{1,0,0,3,0,3,'B','T','R',1,6,'M','R','-','A','I','8',2,4,'V','1','.','0'};

#define CREATE_DRV_PTR 		struct tai8_drv *drv = (struct tai8_drv *)(l2->pdata)
#define CREATE_IDEV_PTR		struct tai8_dev_inp   * idev = NULL
#define CREATE_L1_PTR 		struct mod_l1  *  l1  = &(l2->l1)

// Hier: Geräte 0-7 (INPUT's)
#define GET_DEV_INPUT32(channel,val)	{	\
	idev = drv->Op_Inputs[(channel)];	\
	spin_lock_bh(&idev->Op_Value->lock);	\
	val = idev->Op_Value->P_Value.value;	\
	spin_unlock_bh(&idev->Op_Value->lock);}

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID
/// ------------------------------------------------------------------
int ai8_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;
	MODBUS_CMD_TRACE("ai8_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

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
/// OVERLOAD Function04		Read Input Registers
/// ------------------------------------------------------------------
int ai8_mod_sl2_cmd_rsp_04(struct mod_l2 * l2)	{
	CREATE_DRV_PTR;
	CREATE_IDEV_PTR;
	CREATE_L1_PTR;
	unsigned char 	i,a,p=0;
	uint32_t 		value32 	= 0;

	u16 quan;

	MODBUS_CMD_TRACE("ai8_mod_sl2_cmd_rsp_04()\n");

		///	EXCEPTION 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||						/* Quantity muss da sein 	*/
		(quan == 0) ||								/* Quantity Limit's	1...125 */
		(quan > QUANTITY_READ_INPUT_REGISTERS_MAX_LIMIT)) {
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	}	///	EXCEPTION 02
	else if ((l1->rec.buf[2] != 0) ||				/* Adr-High muss 0 sein 	*/
			(l1->rec.buf[3] > 15) ||				/* Adr-Low muss 0..15 sein 	*/
			_bit_test(l1->rec.buf[3],0) ||			/* Adr-Low gerade 			*/	
			_bit_test(l1->rec.buf[5],0)||			/* Quantity gerade		 	*/	
			(quan + l1->rec.buf[3] > 16))	{		/* Endadr muss passen 		*/
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
			GET_DEV_INPUT32(i,value32)	// Messwerten holen
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

