/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		20.09.2011
 	
 	Description:  MODBUS SLAVE LAYER1

 *******************************************************************************/
#include "modbus_sl1.h"		// slave layer1

/*****************************************************************************
Tabellen für CRC-Berechnung, Low Byte und High Byte
Gegenüber dem Programmbeispiel in der Modbus-Beschreibung wurden die Namen
der Tabellen so geändert, dass High/Low-Byte sinnvoller unterschieden sind.
Sie entsprechen hier CrcHi und CrcLo.
*****************************************************************************/
char const CrcTabLo[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  //  0
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  // 10
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  // 20
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  // 30
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  // 40
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,  // 50
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  // 60
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  // 70
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  // 80
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  // 90
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  //100
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //110
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  //120
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  //130
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  //140
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  //150
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  //160
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //170
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  //180
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //190
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  //200
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  //210
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  //220
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  //230
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  //240
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40                           //250
};

char const CrcTabHi[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,  //  0
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,  // 10
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,  // 20
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,  // 30
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,  // 40
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,  // 50
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,  // 60
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,  // 70
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,  // 80
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,  // 90
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,  //100
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,  //110
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,  //120
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,  //130
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,  //140
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,  //150
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,  //160
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,  //170
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,  //180
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,  //190
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,  //200
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,  //210
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,  //220
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,  //230
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,  //240
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40                           //250
};

void mod_sl1_crc(char sign, char * CrcHi, char * CrcLo) {
    unsigned char Index;              	// Index in die CRC-Tabellen
    Index = (*CrcLo) ^ sign;
    (*CrcLo) = (*CrcHi) ^ CrcTabLo[Index];	// CRC berechnen
    (*CrcHi) = CrcTabHi[Index];
//	printk("CrCHi: %x CrCLo %x\n",(*CrcHi),(*CrcLo)); 
}


// MODBUS-Empfangsfunktion
// Return:
// 0 = nix zu tun
// 1... = letzter Status bei Bearbeitung
size_t mod_sl1_record(char *tlg, size_t count, struct tl1_rec * rec ) {
	int	ret  = 0;
	char 	sign = 0;
//	rec->state = MOD_REC_IDLE;

	while(count)	
	{
		count--;
		sign = *tlg;						// aktuelles Zeichen
		tlg++;						
		switch(rec->state) 					// Status-Maschine	
		{
			case MOD_REC_IDLE:				// mind. ein Zeichen vorhanden ...
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_FLDA: 0x%02x\n", sign);
			#endif
				rec->cnt = 0;
				rec->CrcHi = 0xff;
				rec->CrcLo = 0xff;
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->fld_a = sign;			// Adresse merken ...
				rec->state = MOD_REC_FUNC;
				break;	

			case MOD_REC_FUNC:				// Function-Code
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_FUNC: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->fld_f = sign;			// Function merken ...
				rec->crc_abs = 1;			// voreingestellt Absolute CRC-Position
				rec->crc_cnt = 0;
				switch(rec->fld_f)	{		// Welche Länge erwarten wir?
				    case FUNC_RD_COILS:			//1 
				    case FUNC_RD_DISCRETE_INPUTS:	//2
				    case FUNC_RD_HOLDING_REGISTERS:	//3
				    case FUNC_RD_INPUT_REGISTERS:	//4
				    case FUNC_WR_SINGLE_COIL:		//5
				    case FUNC_WR_SINGLE_REGISTER:	//6
				    case FUNC_RD_EXCEPTION_STATUS:	//7
					  rec->crc_cnt = 4;		// 4 Zeichen bis zum CRC
					  rec->state = MOD_REC_ADDH;	// Adresse (High) erwartet ...
					  break;
					  
				    case FUNC_DIAGNOSTIC:		//8
					  rec->crc_cnt = 4;		// 4 Zeichen bis zum CRC
					  rec->state = MOD_REC_SUBH;	// SubFunction (High) erwartet ...
					  break;
					  
				    case FUNC_GET_COM_EVENT_COUNTER:	//11
				    case FUNC_GET_COM_EVENT_LOG:	//12
					  rec->state = MOD_REC_CRCH;	// sofort CRC
					  break;
					  
				    case FUNC_WR_MULTIPLE_COILS:	//15
				    case FUNC_WR_MULTIPLE_REGISTERS:	//16				      
					  rec->crc_abs = 0;		// RELATIVE crc-Position
					  rec->crc_cnt = 4;		// 4 Zeichen bis zum CRC-OFFSET
					  rec->state = MOD_REC_ADDH;	// Adresse (High) erwartet ...
					  break;
					  
				    case FUNC_REPORT_SLAVE_ID:		//17
					  rec->state = MOD_REC_CRCH;	// sofort CRC
					  break;
					  
				    case FUNC_RD_FILE_RECORD:		//20
				    case FUNC_WR_FILE_RECORD:		//21
					  rec->crc_abs = 0;		// RELATIVE crc-Position
					  rec->crc_cnt = 0;		// 0 Zeichen bis zum CRC-OFFSET
					  rec->state = MOD_REC_CRCX;
					  break;
					  
				    case FUNC_MASK_WR_REGISTER:		//22
					  rec->crc_cnt = 6;		// 6 Zeichen bis zum CRC
					  rec->state = MOD_REC_ADDH;	// Adresse (High) erwartet ...
					  break;
					  
				    case FUNC_RD_WR_MULTIPLE_REGISTERS: //23
					  rec->crc_abs = 0;		// RELATIVE crc-Position
					  rec->crc_cnt = 8;		// 8 Zeichen bis zum CRC-OFFSET
					  rec->state = MOD_REC_ADDH;	// Adresse (High) erwartet ...
					  break;
					  
				    case FUNC_RD_FIFO_QUEUE:		//24
					  rec->crc_cnt = 2;		// 2 Zeichen bis zum CRC
					  rec->state = MOD_REC_ADDH;	// Adresse (High) erwartet ...
					  break;
					  
				    case FUNC_ENCAPSULATED_INTERFACE_TRANSPORT : //43
					  rec->crc_cnt = 3;		// 3 Zeichen bis zum CRC-OFFSET
					  rec->state = MOD_REC_SUBH; 	// SubFunction
					  break;  
				    // 9 und 10 frei!
				    //13 und 14 frei!
				    //18 und 19 frei!
				    //25 ...42 frei!
				    default:				// Keine Funktion laut Modbus Spec 1.1b
					  memset(rec,0,sizeof(struct tl1_rec));	/// MOD_REC_IDLE
					  goto FINAL;				/// ERROR: Unknown-Function - Kein Antworttelegramm!	
					  break;
				} 				
				break;

			/// Address
			case MOD_REC_ADDH:				// Adresse (High) 
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_ADDH: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->AddHi = sign;			// merken
				rec->crc_cnt--;				// crc_cnt muss immer mindestens 2 sein,
				rec->state = MOD_REC_ADDL;		// sonst darf die Marke REC_ADDH nicht aufgerufen
				break;					// werden!

			case MOD_REC_ADDL:				// Adresse (Low)
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_ADDL: 0x%02x\n", sign);
			#endif	
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->AddLo = sign;			// merken
				rec->crc_cnt--;				// Crc-Position dichter
				if( rec->crc_cnt == 0)			
				  rec->state = MOD_REC_CRCH;		// crc_cnt hatte genau 2 dann folgen nun
				else  					// CRC-Bytes
				  rec->state = MOD_REC_DATA;
				break;

			/// Subfunction
			case MOD_REC_SUBH:				// SubFunction (High)
			#ifdef MODBUS_SL1_TRACE	
				printk("MOD_REC_SUBH: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->SubHi = sign;			// merken
				rec->crc_cnt--;				// Crc-Position dichter
				if( rec->crc_cnt == 0)			// crc_cnt muss immer mindestens 2 sein,
				  rec->state = MOD_REC_CRCH;
				else  
				  rec->state = MOD_REC_SUBL;
				break;

			case MOD_REC_SUBL:				// SubFunction (High)
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_SUBL: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->SubLo = sign;			// merken
				rec->crc_cnt--;				// Crc-Position dichter
				if( rec->crc_cnt == 0)			// crc_cnt hatte genau 2 dann folgen nun	
				    rec->state = MOD_REC_CRCH;		// CRC-Bytes
				else  
				  rec->state = MOD_REC_DATA;
				break;

			case MOD_REC_DATA:				// Daten kommen!
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_DATA: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
									// Längenüberwachung!
				if(rec->cnt  > MOD_ADU_RS485_MAX-2) {	// >254 Zeichen, dann hat der 2 Byte CRC kein Platz
				    memset(rec,0,sizeof(struct tl1_rec));/// MOD_REC_IDLE
				    rec->cnt = MOD_ADU_RS485_MAX;		
				    rec->overflow = 1;			/// ERROR: Puffer-Überlauf
				    ret = 1;				/// Antwort notwendig!
				    goto FINAL;				/// Auswertung erfolgt im Layer App
				}										

				rec->cnt++;
				if(rec->crc_cnt == 0)	{			// Dieser Fehler kann nur auftreten, wenn im State
			#ifdef MODBUS_SL1_TRACE					// MOD_REC_FUC der rec->crc_cnt falsch vorbelegt	
				  printk("MOD_REC_DATA:ERROR crc_cnt == 0!\n");	// wird ... im aktuellen "Betrieb" gibt es diesen 
			#endif							// Fehler nicht!
				  memset(rec,0,sizeof(struct tl1_rec));/// MOD_REC_IDLE
				  goto FINAL;				/// ERROR: crc_cnt - Kein Antworttelegramm!		
				}
				rec->crc_cnt--;				// Crc-Position dichter
				if(rec->crc_cnt == 0) {
				    if(rec->crc_abs == 1)
				      rec->state = MOD_REC_CRCH;	// CRC-Bytes erwartet
				    else
				      rec->state = MOD_REC_CRCX;	// CRC-Offset erwartet..
				}
				break;
				
			case MOD_REC_CRCX:				/// CRC-Offset
			#ifdef MODBUS_SL1_TRACE
				printk("MOD_REC_CRCX: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->crc_abs = 1;			//Absolute CRC Position bekannt!
				rec->crc_cnt = sign;			//Zeichenanzahl bis CRC Empfang ..
				rec->state = MOD_REC_DATA;
				break;
				
									///CRC-Empfang
			case MOD_REC_CRCH:				// crc (High)
			#ifdef MODBUS_SL1_TRACE	
				printk("MOD_REC_CRCH: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->state = MOD_REC_CRCL;
				break;

			case MOD_REC_CRCL:				// crc (Low)
			#ifdef MODBUS_SL1_TRACE	
				printk("MOD_REC_CRCL: 0x%02x\n", sign);
			#endif
				rec->buf[rec->cnt] = sign;
				mod_sl1_crc(sign,&rec->CrcHi,&rec->CrcLo);
				rec->cnt++;
				rec->state = MOD_REC_IDLE;		/// MOD_REC_IDLE
				ret = 1;				/// Antwort notwendig!
				goto FINAL;				/// TELEGRAMM i.O.	
				break;

			default:					// kann eigentlich nichtvorkommen!
			#ifdef MODBUS_SL1_TRACE
				printk("Error: MOD_REC_UNKNOWN\n");
			#endif
				memset(rec,0,sizeof(struct tl1_rec));	/// MOD_REC_IDLE
				goto FINAL;				/// ERROR: Unknown-State, keine Antwort!
				break;
		}	// ENDE: EMPANGSSTATUSMACHINE	

	}
FINAL:
//	return(rec->state);
	return (ret);
};

