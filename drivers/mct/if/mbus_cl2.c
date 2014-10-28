/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS CIENT LAYER2

	30.06.2011	HACK snd.cnt = 0 wenn ein gültiges Telegramm vorliegt und
			die Adresse 

 *******************************************************************************/
#include "mbus_def.h"		// basics
#include "mbus_cl2.h"		// client layer2

#define PRI_ADDR_LEN		1
#define SEC_ADDR_LEN		8
#define SEC_ID_LEN			4

void m_cl2_conf_ack(struct m_l2 * l2, signed char count) {
		if(count == 0)
			return;
		if((count == -1) || (count > 1)) {
		#ifdef MBUS_CL2_TRACE
			printk("->COLI! %d\n",count);
		#endif
			memset(l2->l1.snd.buf,FT_1_2_CONTROL2,1);	
			l2->l1.snd.cnt = 1;
		}
		else {
		#ifdef MBUS_CL2_TRACE
			printk("->CONF!\n");
		#endif
			memset(l2->l1.snd.buf,FT_1_2_CONTROL1,count);
			l2->l1.snd.cnt = count;			// Sender aktivieren
		}
		l2->tlg_rdy_100cnt++;			// Immer wenn wir was gesendet haben, war
		if( l2->tlg_rdy_100cnt == 100)	// das Telegramm auch für uns bestimmt!
			l2->tlg_rdy_100cnt = 0;		// Wir zählen die gültigen Telgs bis 100,
										// dann von 0 beginnend!
}

// Vgl. Addressen x,253,254,255 
int m_cl2_primaer_check(struct m_l2 * l2)	{
	struct m_l1  * l1  = &(l2->l1);
	unsigned char idx;
	for(idx = 0; idx < l2->slvs_cfg; idx++) {
		// eine Primäradresse stimmt überein
		if( l1->rec.fld_a == l2->slvs[idx].pri_addr)	{	
				l2->slvs_idx = idx;
				l2->l1.snd.cnt = 0;		// Sender
				return 1;
		}
	}
	// 253, 254, 255
	if( (l1->rec.fld_a == A_BROADCAST_REPLY) || (l1->rec.fld_a == A_NETWORK_LAYER) || (l1->rec.fld_a == A_BROADCAST_NOREPLY)) {
		l2->l1.snd.cnt = 0;
		return 1;
	}
 	return 0;
};

char m_cl2_header_build(struct m_l2 * l2, unsigned char channel, unsigned char type )	{
	char bcc = 0;
	struct m_l1  * l1  = &(l2->l1);

	// 1. Fixed len of frame Header = 7Byte	
	l1->snd.buf[M_XFRA_STA1_IDX]		= FT_1_2_VAR;
	l1->snd.buf[M_XFRA_STA2_IDX]		= FT_1_2_VAR;
	// start Länge	
	bcc+= l1->snd.buf[M_XFRA_C_FLD_IDX]	= type;
	// wenn noch keine Primäraddresse vergeben wurde ,dann 
	// mit der allgemeinen Netzwerkaddresse antworten
	if(l2->slvs[channel].pri_addr == A_UNCONFIGURED)
		bcc+= l1->snd.buf[M_XFRA_A_FLD_IDX]	= A_NETWORK_LAYER;	
	else
		bcc+= l1->snd.buf[M_XFRA_A_FLD_IDX]	= l2->slvs[channel].pri_addr;
		
	bcc+= l1->snd.buf[M_XFRA_CI_FLD_IDX]	= CI_SND_VARIABLE_DATA;
	// 2. Fixed data header = 12 byte					
	memcpy(&l1->snd.buf[M_XFRA_ID_IDX],&l2->slvs[channel].sec_addr,8);
	bcc+= l2->slvs[channel].sec_addr[0];
	bcc+= l2->slvs[channel].sec_addr[1];
	bcc+= l2->slvs[channel].sec_addr[2];
	bcc+= l2->slvs[channel].sec_addr[3];
	bcc+= l2->slvs[channel].sec_addr[4];
	bcc+= l2->slvs[channel].sec_addr[5];
	bcc+= l2->slvs[channel].sec_addr[6];
	bcc+= l2->slvs[channel].sec_addr[7];
	
	bcc+=l1->snd.buf[M_XFRA_ACCESS_IDX]	= l2->slvs[channel].access_cnt;
	bcc+=l1->snd.buf[M_XFRA_STATUS_IDX]	= l2->slvs[channel].state;

	memcpy(&l1->snd.buf[M_XFRA_SIG1_IDX], l2->slvs[channel].signature, M_SIG_CNT);	
	bcc+= l2->slvs[channel].signature[0];
	bcc+= l2->slvs[channel].signature[1];
	
	l1->snd.cnt = 19;					// Anzahl der Zeichen im Puffer
	return bcc;
};


///-----------------------------------------------
/// CI_REC_DATA_SEND	- NO OVERLOAD! 
///-----------------------------------------------
// Einige Subfunktionen können durch die Anwendung überladen werden,
// um sie dem Gerätetyp anzupassen ...

//	[OVERLOAD BY USER]
//	_cmd_snd_ud_global_readout		(struct m_l2 * l2);
//	_cmd_snd_ud_mfr_readout			(struct m_l2 * l2);
//  _cmd_snd_ud_usr_data			(struct m_l2 * l2);

//	[NO OVERLOAD]
//	_snd_ud_write_primary			(struct m_l2 * l2);
//	_snd_ud_write_secondary_id		(struct m_l2 * l2);
//	_snd_ud_write_secondary_address	(struct m_l2 * l2);

#define CHECK_FUNC(name)	if((*(l2->name))==NULL) {\
	printk("Function: "#name"() not loaded!\n - See the m_app-Object how to do that!\n");\
	return 0;\
}	

int _snd_ud_write_primary			(struct m_l2 * l2);
int _snd_ud_write_secondary_id			(struct m_l2 * l2);
int _snd_ud_write_secondary_address		(struct m_l2 * l2);



int m_cl2_snd_ud_data(struct m_l2 * l2, unsigned char mask)	{
	struct m_l1  * l1  = &(l2->l1);
#ifdef MBUS_CL2_TRACE
	printk("m_cl2_snd_ud_data()\n");
#endif
	if(l1->rec.stack >= l1->rec.cnt) {
#ifdef MBUS_CL2_TRACE
		printk("STACK-OVER!\n");
#endif
		return 0;
	}

	switch(l1->rec.buf[l1->rec.stack]) 
	{
		case DIF_DATA_8__BIT_INT: 
		#ifdef MBUS_CL2_TRACE
			printk("DIF_DATA_8__BIT_INT\n");
		#endif
			l1->rec.stack++;	
			switch(l1->rec.buf[l1->rec.stack])	
			{	
				case VIF_PRIMARY_ADDRESS_RECORD:
				#ifdef MBUS_CL2_TRACE
					printk("VIF_PRIMARY_ADDRESS_RECORD\n");
				#endif
					l1->rec.stack++;
			 		// stack zeigt hinter das VIF-FELD
					return (_snd_ud_write_primary(l2));
					break;
				default:
					l1->rec.stack--;
			 		// stack zeigt auf das DIF-FELD
					goto DIF_DEFAULT;
					break;	
			}
		break;

		case DIF_DATA_64_BIT_INT:
		#ifdef MBUS_CL2_TRACE
			printk("DIF_DATA_64_BIT_INT\n");
		#endif
			l1->rec.stack++;	
			switch(l1->rec.buf[l1->rec.stack])
			{	// SCHREIBE Sekundäradresse LongID = 8Byte)
				case VIF_PRIMARY_IDENTIFICATION_RECORD:
					l1->rec.stack++;
			 		// stack zeigt hinter das VIF-FELD
					return (_snd_ud_write_secondary_address(l2));
					break;
				default:
					l1->rec.stack--;	
			 		// stack zeigt auf das DIF-FELD
					goto DIF_DEFAULT;
					break;	
			}
		break;

		case DIF_DATA_8__DIG_BCD:	
		#ifdef MBUS_CL2_TRACE
			printk("DIF_DATA_8__DIG_BCD\n");
		#endif
			l1->rec.stack++;	
			switch(l1->rec.buf[l1->rec.stack])
			{	// SCHREIBE Sekundäradresse (ID = 4Byte als 8 digit BCD)
				case VIF_PRIMARY_IDENTIFICATION_RECORD:
					l1->rec.stack++;
			 		// stack zeigt hinter das VIF-FELD					
					return (_snd_ud_write_secondary_id(l2));
					break;
				default: 
					l1->rec.stack--;
			 		// stack zeigt auf das DIF-FELD
					goto DIF_DEFAULT;
					break;	
			}
		break;
		// SELECTION WITHOUT SPECIFIED DATAFIELD
		// (selection of all data for readout request)
		case DIF_GLOBAL_READOUT_REQUEST:
			#ifdef MBUS_CL2_TRACE
				printk("DIF_GLOBAL_READOUT_REQUEST\n");
			#endif
				CHECK_FUNC(cmd_snd_ud_global_readout);
				l1->rec.stack++;
				if(l1->rec.buf[l1->rec.stack] == VIF_ANY)
					l1->rec.stack++;
				return (l2->cmd_snd_ud_global_readout(l2,mask));
			break;

		case DIF_MANUFACTURER_SPECIFIC_DATA_MORE_RECORDS:
		case DIF_MANUFACTURER_SPECIFIC_DATA_RECORDS:
			#ifdef MBUS_CL2_TRACE
			 	printk("DIF_MANUFACTURER_SPECIFIC_DATA_RECORDS or MORE_RECORDS \n");
			#endif
			 	CHECK_FUNC(cmd_snd_ud_mfr_data);
			 	// stack zeigt auf das DIF-FELD
				return (l2->cmd_snd_ud_mfr_data(l2,mask));
			break;

DIF_DEFAULT:
		// Wir haben diese DIF-Befehle (eventuell mit spezieller VIF-Kennung)
		// nicht als internen Befehl implementiert!
		// Wir senden die Sequenze einfach an die Funktion cmd_snd_ud_usr_data()
		// weiter, wo eine Auswertung erfolgen kann!
		// Hier: z.B. 	DIF_DATA_VARIABLE_LENGTH:
		//				DIF_DATA_VARIABLE_LENGTH | EXTENSION_BIT:
		//				DIF_DATA_16_BIT_INT 
		default:
			#ifdef MBUS_CL2_TRACE
				printk("DIF-[0x%02x] as default\n",l1->rec.buf[l1->rec.stack]);
			#endif
			 	CHECK_FUNC(cmd_snd_ud_usr_data);
				// stack zeigt auf das DIF-FELD
			 	return (l2->cmd_snd_ud_usr_data(l2,mask));
		break;
	}
	return 0;
};

///-----------------------------------------------
/// CI_REC_APPLICATION_RESET	[OVERLOAD BY USER] 
///-----------------------------------------------
// Löst ein Reset aus - entprechend muss nach dem CI-Feld ein CI-Extend-Feld
// kommen, dass im Empfangspuffer unmittelbar am Index 0, steht der - "Reset-Subcode"
// Fehler = 0
// Erfolg = 1
// Die Funktion modifiziert den Stack!
int _cmd_snd_ud_application_reset(struct m_l2 * l2, unsigned char mask)	{
	struct m_l1  * l1  = &(l2->l1);
#ifdef MBUS_CL2_TRACE
	printk("GENERIC: _cmd_snd_ud_application_reset() Mask:[0x%2.0x]\n",mask);
	printk("RESET-Subcode:0x%02x\n",l1->rec.buf[l1->rec.stack]);	
#endif	
	switch(l1->rec.buf[l1->rec.stack]) {
		case APPLICATION_RESET_ALL:
	#ifdef MBUS_CL2_TRACE
		printk("APPLICATION_RESET_ALL\n");
	#endif
			///------------------------------------
			//TODO ...ausführen der Reset-Funktion ...
			///------------------------------------
			break;
		case APPLICATION_RESET_USER_DATA:
		case APPLICATION_RESET_SIMPLE_BILLING:
		case APPLICATION_RESET_ENHANCED_BILLING:
		case APPLICATION_RESET_MULTI_TARIF_BILLING:
		case APPLICATION_RESET_INSTANEOUS_VALUES:
		case APPLICATION_RESET_LOAD_MANAGEMENT_VALUES:	
		case APPLICATION_RESET_INSTALLATION_AND_STARTUP:
		case APPLICATION_RESET_TESTING:
		case APPLICATION_RESET_CALIBRATION:
		case APPLICATION_RESET_MANUFACTURING:
		case APPLICATION_RESET_DEVELOPMENT:
		case APPLICATION_RESET_SELFTEST:
		default:
			l1->rec.stack++;	// Ein Zeichen weiter ...
			return 0;			// UNSUPPORTED application_reset()
			break;
	}
	l1->rec.stack++;			// Ein Zeichen weiter ...
	return 1;	// SUPPORTED application_reset()
};

/// LOAD/OVERLOAD
int _cmd_snd_ud_global_readout(struct m_l2 * l2, unsigned char mask)	{
#ifdef MBUS_CL2_TRACE
	printk("GENERIC: _cmd_snd_ud_global_readout()\n");
#endif
	///---------------------------------------
	//TODO ...für alle MBUS-Module gemeinsam: z.B. Firmwareversion
	///---------------------------------------
	return 0;	// UNSUPPORTED
}

/// LOAD/OVERLOAD
int _cmd_snd_ud_mfr_data(struct m_l2 * l2, unsigned char mask) {
	struct m_l1  * l1  = &(l2->l1);
#ifdef MBUS_CL2_TRACE
	printk("GENERIC: _cmd_snd_ud_mfr_data()\n");
#endif
	///---------------------------------------
	//TODO ...ausführen 
	///---------------------------------------
	switch(l1->rec.buf[l1->rec.stack]) {
		case DIF_MANUFACTURER_SPECIFIC_DATA_RECORDS:
			#ifdef MBUS_CL2_TRACE
				printk("DIF: SPECIFIC_DATA_RECORDS\n");
			#endif
			break;
		case DIF_MANUFACTURER_SPECIFIC_DATA_MORE_RECORDS:
			#ifdef MBUS_CL2_TRACE
				printk("DIF: SPECIFIC_DATA_MORE_RECORDS\n");
			#endif
			break;
	}
	return 0;	// UNSUPPORTED 
};

/// LOAD/OVERLOAD
int _cmd_snd_ud_usr_data(struct m_l2 * l2,unsigned char mask) {
#ifdef MBUS_CL2_TRACE
	printk("GENERIC: _cmd_snd_ud_usr_data()\n");
#endif
	///---------------------------------------
	//TODO ...Dummy-Call
	///---------------------------------------
	return 0;	// UNSUPPORTED
};

/// FIX/NO OVERLOAD
// Diese Funktion schreibt die Primäradresse
// Im Fehlerfall liefert die Funktion = 0
// Im Erfolgsfall liefert die Funktion = 1 und
// modifiziert den Stack!
int _snd_ud_write_primary(struct m_l2 * l2)	{
	struct m_l1  * l1  = &(l2->l1);
#ifdef MBUS_CL2_TRACE
	printk("_snd_ud_write_primary()\n");
#endif
	if(l2->slvs_idx >= l2->slvs_cfg)	{
		//ungültiger Kanal!
#ifdef MBUS_CL2_TRACE
		printk("Error: Channel-%d unconfigured!\n",l2->slvs_idx);
#endif
		return 0;
	}
#ifdef MBUS_CL2_TRACE
	printk("Device-Idx: %d\n",l2->slvs_idx);
	printk("Old: PRI-ADR. 0x%02x\n",l2->slvs[l2->slvs_idx].pri_addr);
	printk("New: PRI-ADR. 0x%02x\n",l1->rec.buf[l1->rec.stack]);
#endif
	l2->slvs[l2->slvs_idx].pri_addr =  l1->rec.buf[l1->rec.stack];
	l1->rec.stack++;	// Ein Zeichen weiter ..
	return 1;
};

/// FIX/NO OVERLOAD
// switchable 4 Byte ID schreiben
int _snd_ud_write_secondary_id(struct m_l2 * l2) {
	struct m_l1  * l1  = &(l2->l1);
#ifdef MBUS_CL2_TRACE
	printk("_snd_ud_write_secondary_id()\n");
#endif
	if(l2->slvs_idx >= l2->slvs_cfg)	{
		//ungültiger Kanal!
#ifdef MBUS_CL2_TRACE
		printk("Error: Channel-%d unconfigured!\n",l2->slvs_idx);
#endif
		return 0;
	}
	memcpy(&l2->slvs[l2->slvs_idx].sec_addr,&l1->rec.buf[l1->rec.stack],4);
#ifdef MBUS_CL2_TRACE
	printk("NEW: SEC-ID. 0x%02x 0x%02x 0x%02x 0x%02x \n",\
	l2->slvs[l2->slvs_idx].sec_addr[0],\
	l2->slvs[l2->slvs_idx].sec_addr[1],\
	l2->slvs[l2->slvs_idx].sec_addr[2],\
	l2->slvs[l2->slvs_idx].sec_addr[3]);
#endif
	l1->rec.stack = l1->rec.stack+4;	// 4 Zeichen weiter ..	
	return 1;
};

/// FIX/NO OVERLOAD
// switchable 8 Byte LongID schreiben
int _snd_ud_write_secondary_address(struct m_l2 * l2) {
	struct m_l1  * l1  = &(l2->l1);
#ifdef MBUS_CL2_TRACE
	printk("_snd_ud_write_secondary_address()\n");
#endif
	memcpy(&l2->slvs[l2->slvs_idx].sec_addr,&l1->rec.buf[l1->rec.stack],8);
#ifdef MBUS_CL2_TRACE
	printk("NEW: SEC-ADR. 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",\
	l2->slvs[l2->slvs_idx].sec_addr[0],\
	l2->slvs[l2->slvs_idx].sec_addr[1],\
	l2->slvs[l2->slvs_idx].sec_addr[2],\
	l2->slvs[l2->slvs_idx].sec_addr[3],\
	l2->slvs[l2->slvs_idx].sec_addr[4],\
	l2->slvs[l2->slvs_idx].sec_addr[5],\
	l2->slvs[l2->slvs_idx].sec_addr[6],\
	l2->slvs[l2->slvs_idx].sec_addr[7]);
	l1->rec.stack = l1->rec.stack+8;	// 8 Zeichen weiter ..	
#endif
	return 1;
};
