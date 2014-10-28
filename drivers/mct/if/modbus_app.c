/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$		21.09.2011
 	
 	Description:  MBUS SLAVE LAYER "APPLICATION"

*********************************************************************************/
#include <linux/slab.h>		// kfree, kzalloc
#include "modbus_app.h"		// slave layer application

///-----------------
///	MOD_APP-Empfänger
///-----------------
void mod_app_task_rec(struct mod_app * la)	{
	struct mod_l2  * l2  = &(la->l2);

	if(!mod_sl2_prim_add_check(l2))
		return;
	// Kommando bearbeiten
	mod_app_response(la);
};

///------------------------------------------
/// MOD_APP --> RESPONSE indication
///------------------------------------------
// Regeln:
// (1)	Für alle nicht gerätespezifischen und fehlerfreien Modbus-Telegramme, 
//     	wird der CRC16 sofort berechnet (on the fly). 
// (3) 	Für alle gerätespezifischen und fehlerfreien Modbus-Telegramme werden die
//	Telgramme durch die jeweilige Funktion (SLOT) gebaut, der CRC16 aber erst
//	nach dem RETURN der Funktion.
// (2) 	Für alle fehlerhaften Modbus-Telegramme, egal ob gerätespezifisch oder nicht
//     	gerätespezifisch, wird das komplette Telegramm und der CRC16 im Bereich
//	"EXCEPTION-RESPONSE" nachgeordnet berechnet.
// Fehlerfreie Bearbeitung, dann
// l2->l1.cnd.cnt	= Anzahl zu sendender Zeichen
// l2->exc		= 0 keine Ausnahme.
int mod_app_response(struct mod_app * la)	{
	struct mod_l2  * l2 = &(la->l2);
	struct mod_l1  * l1 = &(l2->l1);
	unsigned char idx;
	unsigned char sub_func = 0;
	s16				data;
#ifdef MODBUS_APP_TRACE
	unsigned int	i;
#endif

#ifdef MODBUS_APP_TRACE
	printk("mod_app_response()\n");
#endif
	l2->exc = 0;	   			// fehlerfrei ...
	l1->snd.cnt = 0;			// Keine Zeichen zum Senden ...default
	l1->snd.CrcHi = 0xff;			// CRC initialisieren
	l1->snd.CrcLo = 0xff;

	l1->snd.buf[MOD_FLDA_POS] = l1->rec.buf[MOD_FLDA_POS];	// Prim-Adresse kopieren ..
	mod_sl1_crc(l1->snd.buf[MOD_FLDA_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC Adresse
	l1->snd.cnt++;				// 1. Zeichen
	
	if(l1->rec.overflow)	{ 		/// LAYER1 Fehler: Datenüberlauf!	
        l2->exc = EXC_ILLEGAL_DATA_VALUE;   	// ungültige Struktur der Daten
	#ifdef MODBUS_APP_TRACE	
		printk("ERROR: MOD_ADU_RS485_MAX overflow!\n"); 
	#endif
 		l1->rec.overflow = 0;		/// Lösche Überlaufkennung
		goto EXCEPTION;
	}

    if (l1->rec.cnt < 4)	{		// Funktionscode da?
        l1->snd.buf[MOD_FUNC_POS] = 0x80;      	// ungültige (0) statt fehlende Funktion
        l1->snd.cnt++;				// 2. Zeichen
		l2->exc = EXC_ILLEGAL_FUNCTION;
	#ifdef MODBUS_APP_TRACE	
		printk("ERROR: missing\n");
	#endif
		goto EXCEPTION;
    }
	else {
	 	l1->snd.buf[MOD_FUNC_POS] = l1->rec.buf[MOD_FUNC_POS];  // Func-Code
		mod_sl1_crc(l1->snd.buf[MOD_FUNC_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		l1->snd.cnt++;			// 2. Zeichen	
	}
						// Funktion: Diagnostics
	switch (l1->rec.buf[MOD_FUNC_POS])
	{
		///******************
		/// FUNC- sonstige
		///******************
 		case FUNC_DIAGNOSTIC:
		#ifdef MODBUS_APP_TRACE	
			printk("FUNC_DIAGNOSTIC()\n");
		#endif
			// Subfunktionscode nicht da oder High-Byte nicht == 0 
			if ((l1->rec.cnt < 6) || (l1->rec.buf[MOD_SUBH_POS] != 0))	 {
			#ifdef MODBUS_APP_TRACE	
				printk("ERROR: subcode\n");
			#endif
				l2->exc = EXC_ILLEGAL_FUNCTION;
				break;	// EXCEPTION
			}
			l1->snd.buf[MOD_SUBH_POS] = l1->rec.buf[MOD_SUBH_POS];	// Sub-CodeH kopieren ..
			l1->snd.buf[MOD_SUBL_POS] = l1->rec.buf[MOD_SUBL_POS];	// Sub-CodeL kopieren ..
			mod_sl1_crc(l1->snd.buf[MOD_SUBH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC SubHi
			mod_sl1_crc(l1->snd.buf[MOD_SUBL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC SubLo
			l1->snd.cnt+=2;							// 3,4. Zeichen			
			sub_func = l1->rec.buf[MOD_SUBL_POS];	// Subfunktion ermitteln ...
			switch (sub_func)
			{	/// SUB-FUNC:00  (with Echo Request Data)
				case FUNC_SUB_RETURN_QUERY_DATA:
					mod_sl2_diagn_return_query_data(l2);
					// Pufferinhalt nach snd.buf[] kopieren und zurückschicken
					for(idx = l1->snd.cnt; idx + MOD_CRC_LEN  < l1->rec.cnt; idx++) {
					#ifdef MODBUS_APP_TRACE	
						printk("Copy: Buffer! %d\n",l1->rec.buf[idx]);
					#endif
						l1->snd.buf[idx] = l1->rec.buf[idx];
						mod_sl1_crc(l1->snd.buf[idx],&l1->snd.CrcHi,&l1->snd.CrcLo);	// CRC Data
						l1->snd.cnt++;
					}
					break;

				/// SUB-FUNC:01 (with Echo Request Data)
				case FUNC_SUB_RESTART_COMMUNICATIONS_OPTIONS:
					if (l1->rec.cnt != 8)  {  // das Datenfeld muss 2 Bytes lang sein
						l2->exc = EXC_ILLEGAL_DATA_VALUE;
					}
					else	{
						mod_sl2_diagn_restart_comm(l2);
						mod_sl2_diagn_clear_counter(l2);	// Zähler löschen
						// in den Sendepuffer ...
						l1->snd.buf[MOD_DATH_POS] = l1->rec.buf[MOD_DATH_POS];
						l1->snd.buf[MOD_DATL_POS] = l1->rec.buf[MOD_DATL_POS];	
						mod_sl1_crc(l1->snd.buf[MOD_DATH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
						mod_sl1_crc(l1->snd.buf[MOD_DATL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
						l1->snd.cnt+=2;	// 5,6. Zeichen
					}
					break;

				/// SUB-FUNC:04 (with No Responce Returned)
				case FUNC_SUB_FORCE_LISTEN_ONLY_MODE:
					if (mod_sl2_diagn_00(l2))	{				// Datenfeld muss 0,0 sein
						mod_sl2_diagn_listen_only(l2);
					}
					break;

				/// SUB-FUNC:0A  (with Echo Request Data)
				case FUNC_SUB_CLEAR_COUNTERS_AND_DIAGNOSTIC_REGISTER:
					if (mod_sl2_diagn_00(l2))	{
						mod_sl2_diagn_clear_counter(l2);
						// in den Sendepuffer ...
						l1->snd.buf[MOD_DATH_POS] = l1->rec.buf[MOD_DATH_POS];
						l1->snd.buf[MOD_DATL_POS] = l1->rec.buf[MOD_DATL_POS];	
						mod_sl1_crc(l1->snd.buf[MOD_DATH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
						mod_sl1_crc(l1->snd.buf[MOD_DATL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
						l1->snd.cnt+=2;	// 5,6. Zeichen			
					}
					break;	

				/// SUB-FUNC:0B - 0F
				case FUNC_SUB_RETURN_BUS_MESSAGE_COUNT:
				case FUNC_SUB_RETURN_BUS_COMMUNICATION_ERROR_COUNT:
				case FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT:
				case FUNC_SUB_RETURN_SLAVE_MESSAGE_COUNT:
				case FUNC_SUB_RETURN_SLAVE_NO_RESPONSE_COUNT:
					data = l2->slvs[l2->slvs_idx].cnt[sub_func-11];
					if (mod_sl2_diagn_00(l2))	{
						l1->snd.buf[MOD_DATH_POS] = (char) ((data >> 8 ) & 0xff);
						l1->snd.buf[MOD_DATL_POS] = (char) (data & 0xff);
						mod_sl1_crc(l1->snd.buf[MOD_DATH_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
						mod_sl1_crc(l1->snd.buf[MOD_DATL_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
						l1->snd.cnt += 2;
					}
					break;

				default :
				#ifdef MODBUS_APP_TRACE	
					printk("ERROR: FLD_SUBL unsupportd\n");
				#endif
					l2->exc = EXC_ILLEGAL_FUNCTION;
					break;	
			}		// End switch "Sub-Functs"
			break; 	// End case "Func Diagnostics"
		
		/// STUB Function01		Read Coils	
		case FUNC_RD_COILS:
			// Keine Brodcast und kein ListenOnly 
			if((!l2->broadcast) && (l2->slvs[l2->slvs_idx].flg_ListenOnly==0))		
				mod_sl2_cmd_rsp_01(l2);
			break;
		
		/// STUB Function02		Read Discrete Inputs
		case FUNC_RD_DISCRETE_INPUTS:
			// Keine Brodcast und kein ListenOnly 
			if((!l2->broadcast) && (l2->slvs[l2->slvs_idx].flg_ListenOnly==0))		
				mod_sl2_cmd_rsp_02(l2);
			break;
		
		/// STUB Function03		Read Holding Registers
		case FUNC_RD_HOLDING_REGISTERS:
			// Keine Brodcast und kein ListenOnly 
			if((!l2->broadcast) && (l2->slvs[l2->slvs_idx].flg_ListenOnly==0))		
				mod_sl2_cmd_rsp_03(l2);
			break;

		/// STUB Function04		Read Input Registers
		case FUNC_RD_INPUT_REGISTERS:
			// Keine Brodcast und kein ListenOnly 
			if((!l2->broadcast) && (l2->slvs[l2->slvs_idx].flg_ListenOnly==0))		
				mod_sl2_cmd_rsp_04(l2);
			break;

		/// STUB Function05		Write Single Coil
		case FUNC_WR_SINGLE_COIL:
			// Kein ListenOnly
			if(!l2->slvs[l2->slvs_idx].flg_ListenOnly)
				mod_sl2_cmd_rsp_05(l2);
			break;
		
		/// STUB Function06		Write Single Register
		case FUNC_WR_SINGLE_REGISTER:
			if(mod_sl2_special_baudrate(l2))	/// Special Baudrate?
				break;
			// Kein ListenOnly
			if(!l2->slvs[l2->slvs_idx].flg_ListenOnly)
				mod_sl2_cmd_rsp_06(l2);
			break;

		/// STUB Function15		Write Single Coil
		case FUNC_WR_MULTIPLE_COILS:
			// Kein ListenOnly
			if(!l2->slvs[l2->slvs_idx].flg_ListenOnly)
			mod_sl2_cmd_rsp_15(l2);
			break;

		/// STUB Function16		Write Multiple Registers
		case FUNC_WR_MULTIPLE_REGISTERS:
			// Kein ListenOnly
			if(!l2->slvs[l2->slvs_idx].flg_ListenOnly)
				mod_sl2_cmd_rsp_16(l2);
			break;

		/// STUB Function23		Read & Write Multiple Registers
		case FUNC_RD_WR_MULTIPLE_REGISTERS:
			// Kein ListenOnly
			if(!l2->slvs[l2->slvs_idx].flg_ListenOnly)
				mod_sl2_cmd_rsp_16(l2);
			break;
	
		/// STUB Function43	Mei14 	Read Device ID Code 1-4
		case FUNC_ENCAPSULATED_INTERFACE_TRANSPORT:
			// in den Sendepuffer ...
			switch(l1->rec.buf[MOD_MEIT_POS]) {
				case MEI_TYPE_READ_DEVICE_ID:
					// Abbruch, wenn Broadcast oder ListenOnly
					if(l2->broadcast || l2->slvs[l2->slvs_idx].flg_ListenOnly)
						break;
					l1->snd.buf[MOD_MEIT_POS] = l1->rec.buf[MOD_MEIT_POS];
					mod_sl1_crc(l1->snd.buf[MOD_MEIT_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
					l1->snd.cnt++;
					switch(l1->rec.buf[MOD_MEIC_POS]) {
						case MEI_READ_DEVICE_ID_CODE_1:
							l1->snd.buf[MOD_MEIC_POS] = l1->rec.buf[MOD_MEIC_POS];
							mod_sl1_crc(l1->snd.buf[MOD_MEIC_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
							l1->snd.cnt++;
							/// STUB - CALL
							mod_sl2_cmd_rsp_mei_14_code(l2,l1->snd.buf[MOD_MEIC_POS]);
							break;
						case MEI_READ_DEVICE_ID_CODE_2:
						case MEI_READ_DEVICE_ID_CODE_3:
						case MEI_READ_DEVICE_ID_CODE_4:
						default:
							l2->exc = EXC_ILLEGAL_DATA_VALUE;
						#ifdef MODBUS_APP_TRACE	
							printk("ERROR: MEI-Code unsupported\n");
						#endif
							break;
					}
					break;

				case MEI_TYPE_CANopen_GENERAL_REFERENCE:	// Achtung - Fällt durch!
				default:
					l2->exc = EXC_ILLEGAL_FUNCTION;
				#ifdef MODBUS_APP_TRACE	
					printk("ERROR: MEI-TYPE unsupported\n");
				#endif
					break;
				}	// End Switch MEI	
			break;	// End Switch FUNC 43
		///******************
		/// FUNC-DEFAULT's
		///******************
		default:
			l2->exc = EXC_ILLEGAL_FUNCTION;
		#ifdef MODBUS_APP_TRACE	
			printk("ERROR: FLD_F unsupported\n");
		#endif
/*			if (!MbListenOnly) {
        		// Gerätespezifische Funktionen erledigen, normalen Antwort-Frame
        		// vorbereiten, oder Fehlercode in MbExcept ablegen
        		KommandoAntwort();
    		}	*/
			break;
	}	// End Switch FUNC

EXCEPTION:
	///*********************
	/// EXCEPTION-RESPONSE
	///*********************
    if (l2->exc) {						// wenn ein Fehler vorkam
		/// TRACE Errors
		switch(l2->exc) {
			case EXC_ILLEGAL_DATA_ADDRESS:
			#ifdef MODBUS_APP_TRACE	
				printk("EXC_ILLEGAL_DATA_ADDRESS\n");
			#endif
				break;
			case EXC_ILLEGAL_DATA_VALUE:
			#ifdef MODBUS_APP_TRACE
				printk("EXC_ILLEGAL_DATA_VALUE\n");
			#endif
				break;
			case EXC_ILLEGAL_FUNCTION:
			#ifdef MODBUS_APP_TRACE
				printk("EXC_ILLEGAL_FUNCTION\n");
			#endif
				break;
			default:
			#ifdef MODBUS_APP_TRACE
				printk("EXCEPTION unsupported Type: %d\n",l2->exc);
			#endif
				break;  
		}
		if( l2->broadcast)	{			// wenn MOD_A_BROADCAST_NOREPLY
			// Für alle parametrierten Geräte die Exception zählen
			for(idx = 0; idx < l2->slvs_cfg; idx++)	{
				l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT-11]++;
			#ifdef MODBUS_APP_TRACE
				printk(	"Channel:%d BUS_EXCEPTION_ERROR_COUNT:%d\n",\
				idx,\
				l2->slvs[idx].cnt[FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT-11]);
			#endif
			}
		}
		else {	// genau nur dieses Gerät die Exception zählen
			l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT-11]++;
		#ifdef MODBUS_APP_TRACE
			printk(	"Channel:%d BUS_EXCEPTION_ERROR_COUNT:%d\n",\
			l2->slvs_idx,\
			l2->slvs[l2->slvs_idx].cnt[FUNC_SUB_RETURN_BUS_EXCEPTION_ERROR_COUNT-11]);
		#endif
		}
		l1->snd.CrcHi = 0xff;
		l1->snd.CrcLo = 0xff;
		// Adresse bereits im Sendepuffer und aktuell
		mod_sl1_crc(l1->snd.buf[MOD_FLDA_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		// Exception-Response
		l1->snd.buf[MOD_FUNC_POS] = (l1->rec.buf[MOD_FUNC_POS] | 0x80); 
		mod_sl1_crc(l1->snd.buf[MOD_FUNC_POS],&l1->snd.CrcHi,&l1->snd.CrcLo);
		// Encapsulated Interface Transport ?
		if(l1->rec.buf[MOD_FUNC_POS] == FUNC_ENCAPSULATED_INTERFACE_TRANSPORT) {
			 // copy MEI-Code
			l1->snd.buf[2] = l1->rec.buf[2];
			mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
			// Modbus-Exception-Code
			l1->snd.buf[3] = l2->exc;
			mod_sl1_crc(l1->snd.buf[3],&l1->snd.CrcHi,&l1->snd.CrcLo);
			l1->snd.cnt = 4;				// Zeichnanzahl "Sender" Länge 4
        }
        else {								// normale Funktion
            l1->snd.buf[2] = l2->exc;		// Modbus-Exception-Code
			mod_sl1_crc(l1->snd.buf[2],&l1->snd.CrcHi,&l1->snd.CrcLo);
			l1->snd.cnt = 3;				// Zeichnanzahl "Sender" Länge 3
        }
    }
	///*********************
	/// BROADCAST-Test
	///*********************
	if(l2->broadcast) {
		l1->snd.cnt = 0;
	#ifdef MODBUS_APP_TRACE
		printk("SILENT - Broadcast-Message detected!\n");
	#endif
		return 0;
	}
	///*********************
	/// ListenOnly-Test
	///*********************
	if(l2->slvs[l2->slvs_idx].flg_ListenOnly) {
		l1->snd.cnt = 0;
	#ifdef MODBUS_APP_TRACE
		printk("SILENT - ListenOnly Mode detected! channel[%d]\n",l2->slvs_idx);
	#endif
		return 0;
	}

	///*********************
	/// CRC-EXTENDER
	///*********************
	l1->snd.buf[l1->snd.cnt] = l1->snd.CrcLo;
	l1->snd.cnt++;
	l1->snd.buf[l1->snd.cnt] = l1->snd.CrcHi;
	l1->snd.cnt++;					// Zeichenanzahl "SENDEN" komplett

/*
    if (MbListenOnly)               // im Listen-Only-Mode nichts senden,
        PufLen = 0;                 // deshalb Puffer leeren
*/

#ifdef MODBUS_APP_TRACE
	for (i = 0; i< l1->snd.cnt; i++)
		printk("0x%02x ",l1->snd.buf[i]); 
	printk("\n");
#endif
	return 1;
}