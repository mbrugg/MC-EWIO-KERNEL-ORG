/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS CIENT LAYER "APPLICATION"

*********************************************************************************/
#include <linux/slab.h>		// kfree, kzalloc

#include "mbus_app.h"		// client layer application

#define HIGH_NIB			0xF0
#define LOW_NIB				0x0F
#define ALL_BITS			0xFF

const unsigned char _channel_To_mask[MBUS_SLAVES_MAX+1]=	\
	{0, 1, 3, 7,15,31,63,127,255};

const unsigned char	_mask_To_bitscount[256] = \
	{	0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,\
	 	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,\
	 	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,\
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,\
	 	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,\
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,\
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,\
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,\
	 	1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,\
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,\
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,\
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,\
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,\
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,\
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,\
		4,5,6,6,5,6,6,7,5,6,6,7,6,7,7,8,\
};

///------------------------------------------
/// C_FELD-Selectoren
///------------------------------------------
/// 1.Typ C-FELD: REQ_UD2 ---> RSP_UD_DATA2
// OVERLOADABLE
int m_app_rsp_ud_data2 (struct m_app * la) {
#ifdef MBUS_APP_TRACE
	printk("GENERIC: m_app_rsp_ud_data2()\n");
	printk("HowTo:\n");
	printk("struct m_l2  * 	l2  = &(la->l2);\n");
	printk("struct m_l1  * 	l1  = &(l2->l1);\n");
	printk("for aktive channel use: l2->slvs_idx\n");
#endif
	///---------------------------------------
	//TODO ...Dummy-Call
	///---------------------------------------
	return 0;
}

/// 2.Typ C-FELD: REQ_UD1 ---> RSP_UD_DATA1
// OVERLOADABLE
int m_app_rsp_ud_data1 (struct m_app * la) {
#ifdef MBUS_APP_TRACE
	printk("GENERIC: m_app_rsp_ud_data1()\n");
	printk("HowTo:\n");
	printk("struct m_l2  * 	l2  = &(la->l2);\n");
	printk("struct m_l1  * 	l1  = &(l2->l1);\n");
	printk("for active channel use: l2->slvs_idx\n");
#endif
	///---------------------------------------
	//TODO ...Dummy-Call
	///---------------------------------------
	return 0;
}

/// 3.Typ C-FELD: SND_UD ---> CI-FELD-Selector 
// NON OVERLOADABLE
// return 1 =  MBus Modul hatte etwas zu erledigen
//		  0 =  MBus Modul ist nicht gemeint	
int m_app_snd_ud(struct m_app * la)
{
	struct m_l2  * 	l2  = &(la->l2);
	struct m_l1  * 	l1  = &(l2->l1);
	unsigned char	_select = 0;	// Select-Maske (lokal)
	char           	_ret = 0;		// 
	unsigned char	_idx = 0;		// Channel-Index (lokal)
	unsigned char  	n;				// 
	unsigned int	addr;			// EEPROM Adresse
	char		   	val;			// EEPROM Wert

	if(l1->rec.fld_ci == CI_REC_SELECTION_OF_SLAVES) {
		goto SEL_SEC_ADDR;
	}

	switch(l1->rec.fld_a)
	{	// 253 (Netzwerk)
		// Für jeden selektierten Channel Quittung!
		// Für jeden selektierten Channel ausführen!
		case A_NETWORK_LAYER:				
			if(la->select == 0)						// Netzwerk (253) nicht selektiert!
				return 0;
			_ret = _mask_To_bitscount[la->select];	// Wieviele Channel? 
			_select = la->select;					// keine,eine, mehrere Quittungen
			break;
		// 255 (Rundruf)
		// Für alle angelegten Channel
		case A_BROADCAST_NOREPLY:
			_select = _channel_To_mask[l2->slvs_cfg];	// Alle Channel bearbeiten ...
			_ret = 0;									// z.B. 3 Kanäle liefert (0x07)
			break;										// keine  Quittung
		// 254 (Rundruf)
		case A_BROADCAST_REPLY:							// 
				_select = _channel_To_mask[l2->slvs_cfg]; // Alle Channel bearbeiten ...
				_ret = 1;								// reduziere auf eine Quittung
		// Addr	für aktiven Channel
		default:
			// KanalIndex 0,1,2,3 in  KanalMaske 0000 xxxx umwandeln
			_select = (0x00 | (1 << l2->slvs_idx));
			_ret = 1;								// eine Quittung
			break;
	}
	
	switch(l1->rec.fld_ci)
	{	// RESET Applikation
		case CI_REC_APPLICATION_RESET:
		#ifdef MBUS_APP_TRACE
			printk("CI_REC_APPLICATION_RESET\n");
		#endif	
			if( (*(l2->cmd_snd_ud_application_reset)) == NULL) {
			#ifdef MBUS_APP_TRACE
				printk("cmd_snd_ud_appl_reset() == NULL\n");
			#endif
				break;	// RAUS!	
			}	
			l2->cmd_snd_ud_application_reset(l2,_select);
			break;

		// DATA_SEND (Daten vom Master zum Slave)
		case CI_REC_DATA_SEND:
		#ifdef MBUS_APP_TRACE
			printk("CI_REC_DATA_SEND\n");
		#endif
			while(m_cl2_snd_ud_data(l2,_select));
			break;

		// BAUDRATE (Umschalten)
		case CI_REC_SET_BAUDRATE_00300 :
		case CI_REC_SET_BAUDRATE_00600 :
		case CI_REC_SET_BAUDRATE_01200 :
		case CI_REC_SET_BAUDRATE_02400 :
		case CI_REC_SET_BAUDRATE_04800 :
		case CI_REC_SET_BAUDRATE_09600 :
		case CI_REC_SET_BAUDRATE_19200 :
		case CI_REC_SET_BAUDRATE_38400 :
		#ifdef MBUS_APP_TRACE
			printk("CI_REC_SET_BAUDRATE_XXXXX\n");
		#endif
			l2->baud_wish = l1->rec.fld_ci;	// Wunschbaudrate
			l2->baud_tlg = l2->tlg_rdy_100cnt;
			l2->baud_state = 0;
			l2->baud_idx = l1->rec.fld_ci;
		#ifdef MBUS_APP_TRACE
			printk("BAUDRATE-IDX %02x gesetzt ohne Zeit-Kontrolle!\n",l2->baud_idx);
		#endif
			//TODO Eventuell muss noch Telegramm und Zeitüberwachung
			//hinzugefügt werden
			break;

		// Sekundäradressierung
		case CI_REC_SELECTION_OF_SLAVES	:
SEL_SEC_ADDR:
		#ifdef MBUS_APP_TRACE
			printk("CI_REC_SELECTION_OF_SLAVES\n");			
			// Netzwerkaddresse
			// Vergleich der Sekundaeraddresse
			// Return 1  Addresse ist identisch bzw. fällt in den Bereich
			// Return 0	 Addresse nicht zutreffend 	
			printk("Addr: %02x %02x %02x %02x\n", (unsigned char) l1->rec.buf[0],(unsigned char) l1->rec.buf[1],(unsigned char) l1->rec.buf[2],(unsigned char) l1->rec.buf[3] );
		#endif
			la->select = 0;			// Deselect All
			_ret = 0;				// Summierer für gefundene Adressen!	
			for(_idx = 0; _idx< l2->slvs_cfg; _idx++) 
			{
				for(n=0; n<4; n++)		// Länge Sekundaeraddresse
				{	// high-nibble
					if((l1->rec.buf[n] & HIGH_NIB) != HIGH_NIB)
						if((l1->rec.buf[n] & HIGH_NIB) != (l2->slvs[_idx].sec_addr[n] & HIGH_NIB))	{
							goto NO_SEC_ADDR;	// Keine passende Sekundäradresse 
						}
					// low-nibble
					if((l1->rec.buf[n] & LOW_NIB) != LOW_NIB)
						if((l1->rec.buf[n]  & LOW_NIB) !=  (l2->slvs[_idx].sec_addr[n] & LOW_NIB))	{
							goto NO_SEC_ADDR;	// Keine passende Sekundäradresse 
						}
				}
				// MAN-Vgl.
				for(n=4; n<6; n++)		// Länge MAN-Kennung
				{
					if(l1->rec.buf[n] != ALL_BITS)
						if(l1->rec.buf[n] != l2->slvs[_idx].sec_addr[n])	{
							goto NO_SEC_ADDR;	// Keine passende Sekundäradresse 
						}
				}
				if(l1->rec.buf[6] != ALL_BITS)	
					if(l1->rec.buf[6] !=  l2->slvs[_idx].sec_addr[6])	{
						goto NO_SEC_ADDR;	// Keine passende Sekundäradresse
					} 
				if(l1->rec.buf[7] != ALL_BITS)
					if(l1->rec.buf[7] != l2->slvs[_idx].sec_addr[7])	{
						goto NO_SEC_ADDR;	// Keine passende Sekundäradresse
					} 
				_setbit(la->select, _idx);	// gültige Adresse gefunden
				_ret++;						// Summe der gefundenen Adressen
				//+++++++++++++++++++++++++++++++++++++++++++
				// Hier wird der chan_idx immer auf die 1.
				// gefundene, gültige Adresse gesetzt 
				if(_ret == 1)
					l2->slvs_idx = _idx;
NO_SEC_ADDR:;		
		}	// End For
		break;	// END

		///******************
		/// EEPROM-BYTE-WRITE 
		///*******************
		case CI_REC_MANUFACTURER_EEPROM_BYTE_WRITE:
		#ifdef MBUS_APP_TRACE
			printk("ToDo: CI_REC_MANUFACTURER_EEPROM_BYTE_WRITE needs Memory!\n");
		#endif	
			if(la->eeprom_protect == 1)	// 0x01 == protect
				return 0;
			// (1) Adresse und Wert ermitteln
			addr = (l1->rec.buf[0] << 8) + l1->rec.buf[1];
			val  = l1->rec.buf[2];
			///----------------------------
			// TODO Speicher noch notwendig!
			///-----------------------------
		#ifdef MBUS_APP_TRACE
			printk("Addr: 0x%04x Value: 0x%02x\n",addr,val);	
		#endif
			// (2) Wert schreiben
			//eep_write(_i, (char*) &m_rec_buf[EEPROM_BYTE_IDX] ,1 );
			// (3) Verifizierung vorbereiten
			//eep_read(_i,&m_app.eeprom_verify, 1);
			_ret = 1;
			break;
		///******************
		/// EEPROM-READ
		///******************
		case CI_REC_EEPROM_READ:
		#ifdef MBUS_APP_TRACE
			printk("ToDo: CI_REC_EEPROM_READ needs Memory!\n");
		#endif
			if(la->eeprom_protect == 1)	// 0x01 == protect 
				return 0;
			// (1) Adresse ermitteln
			addr = (l1->rec.buf[0] << 8) + l1->rec.buf[1];
			val  = l1->rec.buf[2];	// Achtung val ist die gewünschte Lese-Länge
			///----------------------------
			// TODO Speicher notwendig!
			///-----------------------------
		#ifdef MBUS_APP_TRACE
			printk("Addr: 0x%04x Size: 0x%02x\n",addr,val);
		#endif
			// (2) Werte lesen - laut len
			_ret = 1;
			break;
		///******************
		/// EEPROM-BYTE-VERIFY (letzte geschriebene Byte)	
		///******************
		case CI_REC_MANUFACTURER_EEPROM_BYTE_VERIFY:
		#ifdef MBUS_APP_TRACE
			printk("ToDo: CI_REC_MANUFACTURER_EEPROM_BYTE_VERIFY needs Memory!\n");
		#endif	
			if(la->eeprom_protect == 1)		// 0x01 == protect
				return 0;
		#ifdef MBUS_APP_TRACE
			printk("Val-Snd: 0x%02x <-> Val-EEPROM: 0x%02x\n",l1->rec.buf[0],la->eeprom_verify);	
		#endif
			if(l1->rec.buf[0] == la->eeprom_verify)
				_ret = 1;
			break;
	}
	// §08.07.2011
	if(l1->rec.fld_a != A_BROADCAST_NOREPLY) {	
		// Netzwerk 253 (selected), 254, Addr
		m_cl2_conf_ack(l2, _ret);
		return 1;
	}
	return 0;
};

///-----------------
///	M_APP-Empfänger
///-----------------
void m_app_task_rec(struct m_app * la)	{
	struct m_l2  * l2 = &(la->l2);
	struct m_l1  * l1 = &(l2->l1);
	unsigned char _fcb;

	if(!m_cl2_primaer_check(l2))  {
		// Diese Adresse war nicht für uns bestimmt!
		return;
	}

	switch(l1->rec.fld_c)	
	{
	//-------------------------------------------------------------------------
	// Layer2: SND_NKE - Short Frame Frame, Initialization of Slave	
	//-------------------------------------------------------------------------
		case C_SND_NKE:
		#ifdef MBUS_APP_TRACE
			printk("(INP) SND_NKE [ADD=%d]",l1->rec.fld_a);
		#endif
			switch(l1->rec.fld_a)	{ 
				case  A_NETWORK_LAYER:		// 253 - Netzwerk
					if(la->select != 0)	{	// Netzwerk selektiert!
					#ifdef MBUS_APP_TRACE
						printk(" SELECT= is true\n");
					#endif
						_clrbit(la->last_ud2_fcb_bits_253,l2->slvs_idx);
						m_cl2_conf_ack(l2,1);
					}
					else	{				// Netzwerk nicht selektiert!
					#ifdef MBUS_APP_TRACE
						printk(" SELECT= is false\n");
					#endif
					}
					la->select = 0;
					break;	// Ende A-Feld
  				
				case A_BROADCAST_REPLY:     // 254 
				#ifdef MBUS_APP_TRACE
					printk("FCB[all]=0\n");
				#endif
			        la->last_ud2_fcb_bits = 0;
				#ifdef INCLUDE_SUPPORT_SWITCH_MBUS_REQ_UD1
					la->last_ud1_fcb_bits = 0;
				#endif
					m_cl2_conf_ack(l2,1);
					break;	// Ende A-Feld
				
				case A_BROADCAST_NOREPLY:	// 255
				#ifdef MBUS_APP_TRACE
					printk("FCB[all]=0\n");	
				#endif
					la->last_ud2_fcb_bits = 0;
				#ifdef INCLUDE_SUPPORT_SWITCH_MBUS_REQ_UD1
					la->last_ud1_fcb_bits = 0;
				#endif
					break;	// Ende A-Feld
 
				default:     				// prim. Addresse
				#ifdef MBUS_APP_TRACE
					printk("FCB[%d]=0\n",l2->slvs_idx);
				#endif
					_clrbit(la->last_ud2_fcb_bits,l2->slvs_idx);
				#ifdef INCLUDE_SUPPORT_SWITCH_MBUS_REQ_UD1
					_clrbit(la->last_ud1_fcb_bits,l2->slvs_idx);
				#endif
					m_cl2_conf_ack(l2,1);
					break;	// Ende A-Feld
			}
			break;	// Ende C-Feld (NKE)

		//-------------------------------------------------------------------------
		// Layer2: SND_UD - Long/Control Frame, Send User Data to Slave
		//-------------------------------------------------------------------------
		case C_SND_UD_VF:					// FrameCount Valid, meens check FCB	
		#ifdef MBUS_APP_TRACE
			printk("(INP) SND_UD  [FCV=1,FCB=1]\n");			//(C= 0x73)
		#endif
			goto SND_UD_POINT;
		case C_SND_UD_V :					// FrameCount Valid, meens check FCB
		#ifdef MBUS_APP_TRACE	
			printk("(INP) SND_UD  [FCV=1,FCB=0]\n");			//(C= 0x53)
		#endif
			goto SND_UD_POINT;							 	
		case C_SND_UD_F	:					// FrameCount not Valid, meens ignore FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) SND_UD  [FCV=0,FCB=1(ignore)]\n");	//(C= 0x63)	
		#endif
			goto SND_UD_POINT;													
		case C_SND_UD	:  					// FrameCount not Valid, meens ignore FCB
		#ifdef MBUS_APP_TRACE	
			printk("(INP) SND_UD  [FCV=0,FCB=0(ignore)]\n");	//(C= 0x43)				
		#endif		
		SND_UD_POINT:
			/// 3.Typ C-FELD
			m_app_snd_ud(la);	
			break;	// Ende C-Feld	(SND_UD)
	
		//-------------------------------------------------------------------------
		// Layer2: REQ_UD1 - Short Frame Request for Class1 Data (ALARM-Protokoll!)
		//-------------------------------------------------------------------------
		case C_REQ_UD1_VF:					// FrameCount Valid, meens check FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD1 [FCV=1,FCB=1]\n");			//(C= 0x7A)
		#endif
			goto REQ_UD1_POINT;
		case C_REQ_UD1_V:					// FrameCount Valid, meens check FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD1 [FCV=1,FCB=0]\n");			//(C= 0x5A)			
		#endif
			goto REQ_UD1_POINT;
		case C_REQ_UD1_F:					// FrameCount not Valid, meens ignore FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD1 [FCV=0,FCB=1(ignore)]\n");	//(C= 0x6A)
		#endif
			goto REQ_UD1_POINT;
		case C_REQ_UD1:						// FrameCount not Valid, meens ignore FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD1 [FCV=0,FCB=0(ignore)]\n");	//(C= 0x4A)					
		#endif
			//***************
			// ALLE REQ_UD1's
			//***************	
			REQ_UD1_POINT:
				if((l1->rec.fld_a == A_UNCONFIGURED) || (l1->rec.fld_a == A_NETWORK_LAYER) ||	\
					(l1->rec.fld_a == A_BROADCAST_REPLY) || (l1->rec.fld_a == A_BROADCAST_NOREPLY) ||
					(l1->rec.fld_a == A_FUTURE_USE1) || (l1->rec.fld_a == A_FUTURE_USE2))  		
					break;	// Ende A_Feld		
			#ifdef INCLUDE_SUPPORT_SWITCH_MBUS_REQ_UD1	
				// Nur Primäraddresse!
				if((l1->rec.fld_c == C_REQ_UD1_V) || (l1->rec.fld_c == C_REQ_UD1_VF))
				{
					if(_bitset(la->last_ud1_fcb_bits,l2->slvs_idx)) 
						_fcb = FCB_BIT;
					else
						_fcb = 0; 
					// bits xxx0xxxx == xxx0xxxx oder xxx1xxxx == xxx1xxxx 
					if( _fcb == (l1->rec.fld_c & FCB_BIT))	
					{   // Das alte Telegramm noch mal bitte!, aber nicht 0xe5
						if( l1->snd.buf[0] != FT_1_2_CONTROL1)
						{	// bits bleibt so wie es ist!
						#ifdef MBUS_APP_TRACE
							printk("(Slave send your telegramm again! (1-a)\n");
						#endif
							l1->snd.cnt = l1->snd.backup; // play it again Sam 	
						// OLD: m_snd_again(); 
							break;
						}	
					}
					// bits xxx0xxxx != xxx1xxxx oder xxx1xxxx != xxx0xxxx
					// dann übernehmen wir den Zustand des m_rec.fld.c!!!
					if(l1->rec.fld_c & FCB_BIT)
						_setbit(la->last_ud1_fcb_bits,l2->slvs_idx);
					else
						_clrbit(la->last_ud1_fcb_bits,l2->slvs_idx);
				}
				/// 2.Typ C-FELD		
				if( (*(la->cmd_rsp_ud_data1)) == NULL) {
				#ifdef MBUS_APP_TRACE
					printk("cmd_rsp_ud_data1 == NULL\n");
				#endif
					break;
				}
				// Ausführung erfolgreich ?
				if(la->cmd_rsp_ud_data1(la) == 1)
					break;
			#endif	
			m_cl2_conf_ack(l2,1);
			break;	// Ende C-Feld 	(REQ_UD1)
			
		//-------------------------------------------------------------------------
		// Layer2: REQ_UD2 - Short Frame Request for Class2 Data
		//-------------------------------------------------------------------------
		case C_REQ_UD2_VF:					// FrameCount Valid, meens check FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD2 [FCV=1,FCB=1]\n");	//(C= 0x7B)
		#endif
			goto REQ_UD2_POINT1;			
		case C_REQ_UD2_V :					// FrameCount Valid, meens check FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD2 [FCV=1,FCB=0]\n");	//(C= 0x5B)
		#endif
			// ALLE REQ_UD2's -wenn FRAME COUNT VALID
			REQ_UD2_POINT1:
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD2 [ADD=%d]\n",l1->rec.fld_a);
		#endif
			// [253 - NETZWERK]
			if(l1->rec.fld_a == A_NETWORK_LAYER)	
			{		
				if(la->select == 0)
					break;				// nicht selektiert, dann raus!	
				// 253 (Netzwerk)
				if(_bitset(la->last_ud2_fcb_bits_253,l2->slvs_idx)) 
					_fcb = FCB_BIT;
				else
					_fcb = 0; 
				// bits xxx0xxxx == xxx0xxxx oder xxx1xxxx == xxx1xxxx 
				if( _fcb == (l1->rec.fld_c & FCB_BIT))	
				{     	// das alte Telegramm noch mal bitte!,aber nicht 0xe5
					if( l1->snd.buf[0] != FT_1_2_CONTROL1)
					{	// bits bleibt so wie es ist!
					#ifdef MBUS_APP_TRACE
						printk("Slave send your telegramm again! (2-a)\n");
					#endif
						l1->snd.cnt = l1->snd.backup; // play it again Sam
						// OLD m_snd_again();  // play it again sam
						break;
					}
				}
				// bits xxx0xxxx != xxx1xxxx oder xxx1xxxx != xxx0xxxx
				// dann Übernehmen wir den Zustand des m_rec.fld.c!!!
				if(l1->rec.fld_c & FCB_BIT)
					_setbit(la->last_ud2_fcb_bits_253,l2->slvs_idx);
				else
					_clrbit(la->last_ud2_fcb_bits_253,l2->slvs_idx);
			}	// Ende A-Feld: 253
			else
			// [254, 255, primaeraddresse]
			{
				// !!! Versuch mit Addr. 254 Gerät ansprechen, aber es gibt
				// mehrere, dann dürfen auf keinen Fall Daten gesendet werden!
				// §08.07.2011 
				if((l1->rec.fld_a == A_BROADCAST_REPLY) && (l2->slvs_cfg > 1)) {
				#ifdef MBUS_APP_TRACE
					printk("(A_BROADCAST_REPLY on MULTI-CHANNEL\n");
				#endif	
					m_cl2_conf_ack(l2,-1);	// COLLISION
					break;	
				}

				if(_bitset(la->last_ud2_fcb_bits,l2->slvs_idx)) 
					_fcb = FCB_BIT;
				else
					_fcb = 0; 
				// bits xxx0xxxx == xxx0xxxx oder xxx1xxxx == xxx1xxxx 
				if( _fcb == (l1->rec.fld_c & FCB_BIT))	
				{	// das alte Telegramm noch mal bitte!
					// aber nicht, wenn  Adresse 255!
					// und bei 0xe5 
					if((l1->rec.fld_a != A_BROADCAST_NOREPLY) && (l1->snd.buf[0] != FT_1_2_CONTROL1))
					{	// bits bleibt so wie es ist!
					#ifdef MBUS_APP_TRACE 
						printk("Slave send your telegramm again! (2-b)\n");
					#endif
						l1->snd.cnt = l1->snd.backup; // play it again Sam
						// OLD m_snd_again();  // play it again sam
						break;
					}
				}
				// bits xxx0xxxx != xxx1xxxx oder xxx1xxxx != xxx0xxxx
				// dann übernehmen wir den Zustand des m_rec.fld.c!!!
				if(l1->rec.fld_c & FCB_BIT)
					_setbit(la->last_ud2_fcb_bits,l2->slvs_idx);
				else
					_clrbit(la->last_ud2_fcb_bits,l2->slvs_idx);
			}
			// -------------------------------------
			// REPLAY DIRECTION 
			// Layer 2:  RSP_UD C-Field = 0x08 (fix)
			// --------------------------------------
			/// 1.Typ C-FELD		
			if( (*(la->cmd_rsp_ud_data2)) == NULL) {
			#ifdef MBUS_APP_TRACE
				printk("cmd_rsp_ud_data2 == NULL\n");
			#endif
			} 
			else {	
				la->cmd_rsp_ud_data2(la);
				// Auslesecounter um 1 erhöhen ...
				l2->slvs[l2->slvs_idx].access_cnt++;	
				// Nach dem 1. Auslesen, löschen des Spannungsausfall-bit's im STATUS
				l2->slvs[l2->slvs_idx].state &= (~VARIABLE_DATA_STATUS_POWER_LOW);
				}
			break;
			
		case C_REQ_UD2_F:	// FrameCount not Valid, meens ignore FCB
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD2 [FCV=0,FCB=1(ignore)]\n");	//(C= 0x6B)
		#endif
			goto REQ_UD2_POINT2;
		
		case C_REQ_UD2:		
		#ifdef MBUS_APP_TRACE
			printk("(INP) REQ_UD2 [FCV=0,FCB=0(ignore)]\n");	//(C= 0x5B)
		#endif
			// !!! Einsprung für alle REQ_UD2 OHNE frame count valid !!!	
			REQ_UD2_POINT2:
			if((l1->rec.fld_a == A_NETWORK_LAYER) && (la->select == 0))
				break;
			
			// !!! Versuch mit Addr. 254 Gerät ansprechen, aber es gibt
			// mehrere, dann dürfen auf keinen Fall Daten gesendet werden!
			// §08.07.2011 
			if((l1->rec.fld_a == A_BROADCAST_REPLY) && (l2->slvs_cfg > 1)) {
			#ifdef MBUS_APP_TRACE
				printk("(A_BROADCAST_REPLY on MULTI-CHANNEL\n");
			#endif
				m_cl2_conf_ack(l2,-1);	// COLLISION
				break;
			}

			//  *** no response *** 
			//	- REQ_UD2 has not received correctly
			//  - REQ_UD2 contained address does not match modul-address
			/// 1.Typ C-FELD
			if( (*(la->cmd_rsp_ud_data2)) == NULL) {
			#ifdef MBUS_APP_TRACE
				printk("cmd_rsp_ud_data2 == NULL\n");
			#endif
			} 
			else {
				la->cmd_rsp_ud_data2(la);
				// Auslesecounter zurückstellen!
				if(l2->slvs[l2->slvs_idx].access_cnt == 0xff) {
					l2->slvs[l2->slvs_idx].access_cnt = 0;
				}
				// Auslesecounter um 1 erhöhen ...
				l2->slvs[l2->slvs_idx].access_cnt++;	
				// Nach dem 1. Auslesen, löschen des Spannungsausfall-bit's im STATUS
				l2->slvs[l2->slvs_idx].state &= (~VARIABLE_DATA_STATUS_POWER_LOW);
			}
			break;	

		//-------------------------------------------------------------------------
		// Layer2: Unbekannter Control Code empfangen!
		// 		   Das darf eigentlich nicht auftreten: ... 0xe5?	
		//-------------------------------------------------------------------------
		default:
		#ifdef MBUS_APP_TRACE
			printk("(INP) UNKNOWN_TLG\n");
		#endif
			break;
	}
};
