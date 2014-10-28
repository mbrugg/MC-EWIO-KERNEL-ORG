/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		21.09.2011
 	
 	Description:  MODBUS SLAVE LAYER2


 *******************************************************************************/
#include "modbus_def.h"		// basics
#include "modbus_sl2.h"		// slave layer2

// Bei der Diagnose wird geprüft ob das Datenfeld 0,0 enthält!
// Return:
// 1 = 	Datenfeld hat 0,0
// 0 = 	Datenfeld nicht 0,0 oder die Länge stimmt nicht
//		l2->exec wird dann gesetzt!
unsigned char mod_sl2_diagn_00(struct mod_l2 * l2) {
	struct mod_l1  * l1  = &(l2->l1);
#ifdef MODBUS_SL2_TRACE
	printk("mod_sl2_diagn_00()\n");
#endif
    if ((l1->rec.cnt != 8) ||	// das Datenfeld muss genau 2 Bytes lang sein,
        l1->rec.buf[4] ||      	// sein Inhalt muss 0, 0 sein
        l1->rec.buf[5]) 
	{
        l2->exc = EXC_ILLEGAL_DATA_VALUE; // ungültige Struktur der Daten
		return 0;
	}
	return 1;
}

/// FUNC_WR_SINGLE_REGISTER: SPECIAL BAUDRATE
// VIRTUELL !
// Der Spezialfall von "Write Single Register" wird hier
// herausgefiltert.
// Dies ist eine Dummy-Funktion zum "virtuellen" Einstellen der 
// Baudrate und der Parität - wie bei einem tatsächlichen
// Modbusmodul der Firma BTR-Netcom.
// Die Funktion kann auch im bei Listen-Only Zustand als 
// Wartungsfunktion aufgerufen werden -
// Return:
// 1 = erfolgreiche Baudratenumschaltunng
// 0 = nix für diese Funktion
int mod_sl2_special_baudrate(struct mod_l2 * l2)	{
	struct mod_l1  * l1  = &(l2->l1);
	unsigned char idx;	
	unsigned char	CommMode;		// virtueller Mode
	unsigned char	CommBaud;		// virtuelle Baudrate
	// Länge muss 8 sein, Adr-Hi muss 0, Adr-Lo muss 65, Val-Hi muss 0x53 sein (Magic-Num)
    if ((l1->rec.cnt == 8) &&
		(l1->rec.buf[MOD_ADDH_POS] == 0) &&
		(l1->rec.buf[MOD_ADDL_POS] == 65) &&
		(l1->rec.buf[MOD_DATH_POS] == 0x53))	{
#ifdef MODBUS_SL2_TRACE	
		printk("mod_sl2_special_baudrate()\n");
#endif
		CommMode = (l1->rec.buf[5] >> 4) & 0x0F;
		CommBaud = l1->rec.buf[5] & 0x0F;

		switch(CommMode) { 
			case VIRTUAL_PARITY_OFF:
			case VIRTUAL_PARITY_ODD:
			case VIRTUAL_PARITY_EVEN:
				// 
				break;
			default:
				// Bei nicht unterstütztem Mode auf STANDARD: Keine Parität
				CommMode = VIRTUAL_PARITY_OFF;
				break;
		}

		// Bei nicht unterstützter Baudrate auf STANDARD: 9600 stellen!
		if(CommBaud > VIRTUAL_BAUD_115200)
			CommBaud = VIRTUAL_BAUD_009600;

		if (l2->broadcast)		{		// alle parametrierten Geräte	
			for(idx = 0; idx < l2->slvs_cfg; idx++)	{
				l2->slvs[idx].CommMode = CommMode;
				l2->slvs[idx].CommBaud = CommBaud;
			#ifdef MODBUS_SL2_TRACE
				printk("Channel:%d CommMode:0x%02x\n",idx,CommMode);
				printk("Channel:%d CommBaud:0x%02x\n",idx,CommBaud);
			#endif
 			}
		}
		else {							// genau nur dieses Gerät
			l2->slvs[l2->slvs_idx].CommMode = CommMode;
			l2->slvs[l2->slvs_idx].CommBaud = CommBaud;
		#ifdef MODBUS_SL2_TRACE	
			printk("Channel:%d CommMode:0x%02x\n",l2->slvs_idx,CommMode);
			printk("Channel:%d CommBaud:0x%02x\n",l2->slvs_idx,CommBaud);
		#endif
		}
		mod_sl2_diagn_restart_comm(l2);		// ListenOnly-Flag(s) löschen
		mod_sl2_diagn_clear_counter(l2);	// Zähler löschen 
		// Pufferinhalt zurückschicken
		l1->snd.buf[MOD_ADDH_POS] = l1->rec.buf[MOD_ADDH_POS];	
		l1->snd.buf[MOD_ADDL_POS] = l1->rec.buf[MOD_ADDL_POS];
		l1->snd.buf[MOD_DATH_POS] = l1->rec.buf[MOD_DATH_POS];
		l1->snd.buf[MOD_DATL_POS] = l1->rec.buf[MOD_DATL_POS];
		// CRC
		mod_sl1_crc(l1->snd.buf[MOD_ADDH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[MOD_ADDL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[MOD_DATH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		mod_sl1_crc(l1->snd.buf[MOD_DATL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt+=4;
		return 1;
	}
	return 0;
}

/// FUNC_DIAGNOSTIC - SUB-FUNC:00
void mod_sl2_diagn_return_query_data(struct mod_l2 * l2)	{
#ifdef MODBUS_SL2_TRACE
	printk("mod_sl2_diagn_return_query_data()\n");
#endif
}

/// FUNC_DIAGNOSTIC - SUB-FUNC:01
void mod_sl2_diagn_restart_comm(struct mod_l2 * l2)	{
	unsigned char idx;	
#ifdef MODBUS_SL2_TRACE
	printk("mod_sl2_diagn_restart_comm()\n");
#endif
	if(l2->broadcast)		{					// Broadcastadresse 0
		for(idx = 0; idx < l2->slvs_cfg; idx++)	{// alle parametrierten Geräte
			l2->slvs[idx].flg_ListenOnly = 0; 	// Löschen Listen-Only-Mode
		#ifdef MODBUS_SL2_TRACE
			printk("Channel:%d flg_ListenOnly:0\n",idx);
		#endif
		}
	}
	else {											// genau nur dieses Gerät
		l2->slvs[l2->slvs_idx].flg_ListenOnly = 0;	// Löschen Listen-Only-Mode
	#ifdef MODBUS_SL2_TRACE
		printk("Channel:%d flg_ListenOnly:0\n",l2->slvs_idx);
	#endif
	}
};

/// FUNC_DIAGNOSTIC - SUB-FUNC:04
void mod_sl2_diagn_listen_only(struct mod_l2 * l2)	{
	unsigned char idx;	
#ifdef MODBUS_SL2_TRACE
		printk("mod_sl2_diagn_listen_only()\n");
#endif
	if(l2->broadcast)		{					// Broadcastadresse 0
		for(idx = 0; idx < l2->slvs_cfg; idx++)	{// alle parametrierten Geräte
			l2->slvs[idx].flg_ListenOnly = 1; 	// Listen-Only-Mode
		#ifdef MODBUS_SL2_TRACE
			printk("Channel:%d flg_ListenOnly:1\n",idx);
		#endif
		}
	}
	else {											// genau nur dieses Gerät
		l2->slvs[l2->slvs_idx].flg_ListenOnly = 1;	// Listen-Only-Mode
	#ifdef MODBUS_SL2_TRACE	
		printk("Channel:%d flg_ListenOnly:1\n",l2->slvs_idx);
	#endif
	}
	return;
};

/// FUNC_DIAGNOSTIC - SUB-FUNC:0A
void mod_sl2_diagn_clear_counter(struct mod_l2 * l2)	{
	unsigned char idx;	
#ifdef MODBUS_SL2_TRACE
	printk("mod_sl2_diagn_clear_counter()\n");
#endif
	if(l2->broadcast)	{	// Broadcast-Adresse 0
		// für alle parametrierten Geräte, Zähler löschen
		for(idx = 0; idx < l2->slvs_cfg; idx++)	{
		#ifdef MODBUS_SL2_TRACE
			printk("Channel:%d clear all counters\n",idx);
		#endif
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_MESSAGE_COUNT-11] = 0;
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_COMMUNICATION_ERROR_COUNT-11] = 0;
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT-11] = 0;
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT-11] = 0;
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_NO_RESPONSE_COUNT-11] = 0;	
		}
	}
	else { // genau nur dieses Gerät, Zähler löschen!
	#ifdef MODBUS_SL2_TRACE
		printk("Channel:%d clear all counters\n",l2->slvs_idx);
	#endif
		l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_BUS_MESSAGE_COUNT-11] = 0;
		l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_BUS_COMMUNICATION_ERROR_COUNT-11] = 0;
		l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT-11] = 0;
		l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT-11] = 0;
		l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_SLAVE_NO_RESPONSE_COUNT-11] = 0;	
	}
	return;
}

// Funktion:
// Vergleich der Adressen
// Return:
// 1 = wenn Broadcastadresse 0 oder eine konfigurierte Adresse gefunden
//	
// 0 = wenn die Adresse nicht für das Gerät war!
//
int mod_sl2_prim_add_check(struct mod_l2 * l2)	{
	struct mod_l1  * l1  = &(l2->l1);
	unsigned char idx 	= 0;
	l2->broadcast		= 0;	// wird erst gesetzt, wenn die ADDR 0 kam!
	l2->slvs_idx 		= 0;	// wird modifiziert, wenn eine Adresse 1-250 passt
#ifdef MODBUS_SL2_TRACE
	printk("mod_sl2_prim_add_check()\n");
#endif
	// Keine Adresse+CRC  bzw.  CRC nicht 0!
	if((l1->rec.cnt < 3) || (l1->rec.CrcHi || l1->rec.CrcLo)) {
	  for(idx = 0; idx < l2->slvs_cfg; idx++) {
	    // Busfehler zählen!
	    l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_COMMUNICATION_ERROR_COUNT-11]++;	
	    #ifdef MODBUS_SL2_TRACE
	      printk("Channel:%d BUS_COMMUNICATION_ERROR_COUNT:%d\n",\
		  idx,\
		  l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_COMMUNICATION_ERROR_COUNT-11]);
	    #endif
	  }	
	  return 0;
	}

	for(idx = 0; idx < l2->slvs_cfg; idx++) {
	  // Busmeldungen zählen!
	  l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_MESSAGE_COUNT-11]++;
	#ifdef MODBUS_SL2_TRACE
	  printk("Channel:%d BUS_MESSAGE_COUNT:%d\n",\
		idx,\
		l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_MESSAGE_COUNT-11]);
	#endif
	}	

	// Hier: MOD_A_BROADCAST_NOREPLY: 0
	if(l1->rec.fld_a == MOD_A_BROADCAST_NOREPLY) {
		l2->broadcast=1;
		for(idx = 0; idx < l2->slvs_cfg; idx++) {
			// Slave Meldungen zählen!
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT-11]++;
		#ifdef MODBUS_SL2_TRACE
			printk("Channel:%d SLV_MESSAGE_COUNT:%d\n",\
					idx,\
					l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT-11]);
		#endif
			// Broadcasts zählen!
			l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_NO_RESPONSE_COUNT-11]++;
		#ifdef MODBUS_SL2_TRACE
			printk("Channel:%d SLV_NORESP_COUNT:%d\n",\
					idx,\
					l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_NO_RESPONSE_COUNT-11]);
		#endif
		}			
		return 1;	// wir sind gemeint!
	}

	// Hier: MOD_A_1...250
	for(idx = 0; idx < l2->slvs_cfg; idx++) {
	#ifdef MODBUS_SL2_TRACE
		printk("Loop %d FldA:%d pri_add:%d\n", idx, l1->rec.fld_a,l2->slvs[idx].pri_addr); 
	#endif
		// eine Primäradresse stimmt überein
		if( l1->rec.fld_a == l2->slvs[idx].pri_addr)	{	
				l2->slvs_idx = idx;	// Merke den Index zur Adresse!
				// Slave Meldungen zählen
				l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT-11]++;
			#ifdef MODBUS_SL2_TRACE
				printk("Channel:%d SLV_MESSAGE_COUNT:%d\n",\
						idx,\
						l2->slvs[idx].cnt[FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT-11]);
			#endif
				return 1; // wir sind gemeint!
		}
	}
 	return 0;
};

//	[OVERLOAD BY USER]
#define CHECK_FUNC(name, message)	if((*(l2->name))==NULL) {\
	printk(message);	\
	printk("Function: "#name"() not loaded!\n - See the m_app-Object how to do that!\n");\
	return 0;\
}	

/// STUB Function01		Read Coils
int mod_sl2_cmd_rsp_01(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_01,"STUB: FUNC_RD_COILS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_01(l2);
};

/// STUB Function02		Read Discrete Inputs
int mod_sl2_cmd_rsp_02(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_02,"STUB: FUNC_RD_DISCRETE_INPUTS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_02(l2);
};

/// STUB Function03		Read Holding Registers
int mod_sl2_cmd_rsp_03(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_03,"STUB: FUNC_RD_HOLDING_REGISTERS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_03(l2);
};

/// STUB Function04		Read Input Registers
int mod_sl2_cmd_rsp_04(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_04,"STUB: FUNC_RD_INPUT_REGISTERS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_04(l2);
};

/// STUB Function05		Write Single Coil
int mod_sl2_cmd_rsp_05(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_05,"STUB: FUNC_WR_SINGLE_COIL()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_05(l2);
};

/// STUB Function06		Write Single Register
int mod_sl2_cmd_rsp_06(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_06,"STUB: FUNC_WR_SINGLE_REGISTER()\n");
	l2->exc = 0;
#endif
	return l2->cmd_rsp_06(l2);
};

/// STUB Function15		Write Multiple Coils
int mod_sl2_cmd_rsp_15(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_15,"STUB: FUNC_WR_MULTIPLE_COILS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_15(l2);
};

/// STUB Function16		Write Multiple Registers
int mod_sl2_cmd_rsp_16(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_16,"STUB: FUNC_WR_MULTIPLE_REGISTERS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_16(l2);
};

/// STUB Function23		Read & Write Multiple Registers
int mod_sl2_cmd_rsp_23(struct mod_l2 * l2)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_23,"STUB: FUNC_RD_WR_MULTIPLE_REGISTERS()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_23(l2);
};

/// STUB Function43	Mei14 	Read Device ID
int mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code)	{
	l2->exc = EXC_ILLEGAL_FUNCTION;
#ifdef MODBUS_SL2_TRACE
	CHECK_FUNC(cmd_rsp_mei_14_code,"STUB: MEI_READ_DEVICE_IDENTIFICATION()\n");
#endif
	l2->exc = 0;
	return l2->cmd_rsp_mei_14_code(l2, code);
};
