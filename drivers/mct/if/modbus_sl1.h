/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		20.09.2011
 	
 	Description:  MODBUS SLAVE LAYER1

 *******************************************************************************/
#ifndef __MODBUS_SL1_H__
	#define __MODBUS_SL1_H__

#include "modbus_def.h"		// basics
#include <linux/module.h>

enum MOD_REC_STATE {  		// Empfangsstati MBus-Schnittstelle
	MOD_REC_IDLE   = 0,	// nichts zu tun!	
	MOD_REC_FUNC,		// empfange Funktion
	MOD_REC_ADDH,		// empfange Adresse (Hi-Part)
	MOD_REC_ADDL,		// empfange Adresse (Lo-Part)
	MOD_REC_SUBH,		// empfange Subfuntion (Hi-Part)
	MOD_REC_SUBL,		// empfange Subfuntion (Lo-Part
	MOD_REC_DATA,		// empfange Daten ...
	MOD_REC_CRCH,		// empfange CRC-High
	MOD_REC_CRCL,		// empfange CRC-Low
	MOD_REC_CRCX,		// empfange CRC-Offset
};

struct tl1_rec {
	unsigned char 	state;			// status			
// Telegrammaufbau
	unsigned char 	fld_a;			// Adress-Feld
	unsigned char	fld_f;			// Function-Feld
	unsigned char	SubHi;
	unsigned char	SubLo;
	unsigned char	AddHi;
	unsigned char	AddLo;
	char 		CrcHi;     		// High-Byte für CRC
	char 		CrcLo;     		// Low-Byte für CRC
// Puffer-Zeug
	u16		cnt;			// aktuelle Zeichenanzahl in buf[]
	unsigned char 	crc_abs;		// 1 = Absolute CRC-Position bekannt, 0 noch nicht!
	unsigned char	crc_cnt;		// Zähler bis zur CRC	
	unsigned char	overflow;		// Überlauf-Merker!
	char 		buf[MOD_ADU_RS485_MAX];
};

struct tl1_snd {
	char 			CrcHi;     	// High-Byte für CRC
	char 			CrcLo;     	// Low-Byte für CRC
	u16			cnt;		// aktuelle Zeichenanzahl in buf[]
	char 			buf[MOD_ADU_RS485_MAX];
};

// LAYER1 Struktur
struct	mod_l1 {  
	struct	tl1_rec rec;
	struct 	tl1_snd snd;
};


extern void 	mod_sl1_crc(char sign, char * crcHi, char * crcLo);
extern size_t 	mod_sl1_record(char *tlg, size_t count, struct tl1_rec * rec);


#endif // __MODBUS_L1_H__
