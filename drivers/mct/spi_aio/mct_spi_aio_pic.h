/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:

*********************************************************************************/
#ifndef MCT_SPI_AIO_PIC_
	#define MCT_SPI_AIO_PIC_
	#include <linux/spi/spi.h>			// spi_transfer
/// PIC-HARDWARE	Infos
/// D/A-Outputs - SPANNUNG / SPANNUNG
/* ------------------------------------------------------------------------	
	Der Digital/Analog-Wandler kann bis zu 14 bit auflösen. Laut Spec D/A 
	sind bis ca. 25mV Spannung bei Leerlauf möglich! Bereits einfache Mess-
	versuche ergaben, Toleranzen von bis zu 7mV. Wir nutzen deshalb nur 
	10bit und verwenden 10mA als LSB. (Eventuell können noch 5mV LSB einen
	Sinn ergeben, dazu sollte aber die gesamte Messreihe ermittelt werden!)
	
 
	Spannungen	: 10mV bis 10.23V  	bit5....bit14 (10bit-Auflösung in 10mV)
	Ströme		: 10yA bis 20.47mA	bit4....bit14 (11bit-Auflösung in 10yA)
	_____________________		 	
	USER<->ARM-Format:
	
	Bit15		Vorzeichen (immer 0)
	Bit14	 5120	mV			10240 yA	
	Bit13	 2560	mV			 5120 yA
	Bit12	 1280	mV			 2560 yA
	Bit11	  640	mV			 1280 yA
	Bit10	  320	mV			  640 yA
	Bit9	  160	mV			  320 yA
	Bit8	   80	mV			  160 yA
	Bit7	   40	mV			   80 yA
	Bit6	   20	mV			   40 yA
	Bit5	   10	mV			   20 yA
	Bit4	    5	mV			   10 yA
	Bit3	  2.5	mV			    5 yA
	Bit2	 1.25	mV			  2.5 yA	
	Bit1    0.625	mV			 1.25 yA
	Bit0	-----				 ----
	__________________________	
	ARM->PIC Transport-Format
	kx = kanalcode, w15=0, w14=bit9, w4-w0=0

		Bit7	Bit6	Bit5	Bit4	Bit3	Bit2	Bit1	Bit0	
	Byte0	1	1	0	0	k1	k0	w15	w14
	Byte1	0	w13	w12	w11	w10	w9	w8	w7
	Byte2	0	w6	w5	w4	w3	w2	w1	w0
*/

/// PIC-HARDWARE	Infos
/// A/D-Inputs
/* ------------------------------------------------------------------------	
	Der A/D-Wandler muss mit einem Messbereich
------------------------------------------------------------------------*/
#define DEVICE_AIO_IN_MAX	4	// vom PIC maximal unterstützte IN-Geräte
#define DEVICE_AIO_IN_12_MAX	2	
	
#define DEVICE_AIO_OUT_MAX	4	// vom PIC maximal unterstützte OUT-Geräte
#define DEVICE_AIO_OUT_12_MAX	2

#define DEVICE_IN_0		0	// IN-Gerätenummern
#define DEVICE_IN_1		1
#define DEVICE_IN_2		2
#define DEVICE_IN_3		3

#define DEVICE_OUT_0		0	// OUT-Gerätenummern
#define DEVICE_OUT_1		1	
#define DEVICE_OUT_2		2	
#define DEVICE_OUT_3		3

#define PIC_SND_IDLE 		0
#define PIC_SND_VERSION 	1
#define PIC_SND_ABGLEICH_0	2
#define PIC_SND_ABGLEICH_1	3
#define PIC_SND_ABGLEICH	4
#define PIC_SND_CONFIG 		5
#define PIC_SND_SET_MESSWERT 	6

#define PIC_REC_IDLE 		0	//
#define PIC_REC_VERSION		1	// Version-Telegramm
#define PIC_REC_ABGLEICH	2
#define PIC_REC_GET_MESSWERT 	3	// Messwert-Telegramm
#define PIC_REC_LIFE		4	// Life-Telegramm

#define PIC_TIMEOUT_ON_VERSION  1<<0
#define PIC_TIMEOUT_ON_ABGLEICH 1<<1
#define PIC_TIMEOUT_ON_MEASURE  1<<2
#define PIC_TIMEOUT_ON_CONFIG   1<<3

struct t_pic_snd {
	u8	state;
	u8	write;
	u8	pending;	
	u8	cmd[16];		// Commando-Buffer für Befehle an den PIC
	u16	aou[DEVICE_AIO_OUT_MAX];// Arbeitsvarable: analoge Ausgangswerte für den PIC
};

struct t_pic_rec {
	u8	state;
	u32	ist;
	u32	soll; 
	u8	signs; 
	u8	kanal;			// Arbeitsvariable.. aktueller Kanal
	u8	mode;			// Arbeitsvariable.. aktuelle Messeinstellung
	u32	ain;			// Arbeitsvariable.. aktueller Eingangswert (float)
	u32	aia;			// Arbeitsvariable.. Abgleichwert
	u8	aia_kanal;		// Arbeitsvariable.. Abgleichwert-Kanal
};

/// PIC-CONTROL
struct t_pic{
	struct t_pic_snd	snd;
	struct t_pic_rec	rec;
};

#endif

