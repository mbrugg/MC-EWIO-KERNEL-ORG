/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	06.10.2011
 	
 	Description:  MODBUS-Command-Implementation for mct_paa_di4-Driver

 *******************************************************************************/
#include "mct_di4_objs.h"			// objects
#include "mct_di4_modbus_cfg.h"	// Konfiguration
#include "../../if/modbus_sl2.h"
#include "../../if/modbus_app.h"
	
// Trace-Makro
#ifdef CONFIG_MODBUS_CMD_TRACE
	#define MODBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MODBUS_CMD_TRACE(args...);
#endif

const char id_code[23] = \
	{1,0,0,3,0,3,'B','T','R',1,6,'M','R','-','D','I','4',2,4,'V','1','.','0'};

#define CREATE_L1_PTR 		struct mod_l1  *  l1  = &(l2->l1)

/// ------------------------------------------------------------------
/// OVERLOAD Function43	- SubCode14 - Code01 =  READ_DEVICE_ID
/// ------------------------------------------------------------------
int di4_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	CREATE_L1_PTR;
	unsigned char i;
	
	MODBUS_CMD_TRACE("di4_mod_sl2_cmd_rsp_mei_14_code() code: %d\n", code);

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
/// OVERLOAD Function02		Read Discrete Inputs
/// ------------------------------------------------------------------
int di4_mod_sl2_cmd_rsp_02(struct mod_l2 * l2)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;
	unsigned char a,n,b,m;
	u16 quan;

	MODBUS_CMD_TRACE("di4_mod_sl2_cmd_rsp_02()\n");

		/// EXCEPTION CODE 03
	quan = (l1->rec.buf[4] << 8) | l1->rec.buf[5]; 
	if(	(l1->rec.cnt != 8) ||							/* Quantity muss da sein	*/
		(quan == 0) ||									/* Quantity Limit's 		*/
		(quan > QUANTITY_READ_DISCRETE_INPUTS_MAX_LIMIT))	{
		l2->exc = EXC_ILLEGAL_DATA_VALUE;
	} /// EXCEPTION CODE 02
	else if	(	(l1->rec.buf[2] != 0) ||				/* Adr-High muss 0 sein		*/
				(l1->rec.buf[3] > 3) 	||				/* Adr-Low muss 0..3 sein	*/
				(quan + l1->rec.buf[3]  > 4))	{		/* Endadr muss passen		*/
		l2->exc = EXC_ILLEGAL_DATA_ADDRESS;
	} 
	else	{
	/// WERT...
		OBJS_IDEV_RD_INPUT(idev,b);	// Bits 
		a = l1->rec.buf[3];      	// Adresse
		n = l1->rec.buf[5];     	// Anzahl Bits 
		if ((a == 0) && (n == 4)) {	// schneller Spezialfall
			b &= 0x0F;	// alle 4 Bit
		}
		else {						// langsamer Allgemeinfall
			m = ~(0xFF << n);		// Maske
			b >>= a;				// an Adresse schieben
			b &= m;					// Bits maskiert
		}
	/// PROTOKOLL...
		l1->snd.buf[2] = 1;			// 1 Byte-Count
		l1->snd.buf[3] = b;
		mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt +=2;
	}
	return 1;
};


