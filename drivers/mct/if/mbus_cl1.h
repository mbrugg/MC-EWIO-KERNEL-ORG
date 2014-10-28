/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:	MBUS CIENT LAYER1

 *******************************************************************************/
#ifndef __MBUS_CL1_H__
	#define __MBUS_CL1_H__
#include <linux/module.h>

#define MBUS_TLG_MAX_LEN		255

// define Empfangs Attribut Codes
#define M_ATTR_REC_NONE			0
#define M_ATTR_REC_FRA_S		1 <<1		// Single-Frame
#define M_ATTR_REC_FRA_C		1 <<2		// Control-Frame
#define M_ATTR_REC_FRA_L		1 <<3		// Long-Frame	
	// Control/LongFrame
#define M_XFRA_STA1_IDX			0
#define M_XFRA_L_FLD1_IDX		1
#define M_XFRA_L_FLD2_IDX		2
#define M_XFRA_STA2_IDX			3
#define M_XFRA_C_FLD_IDX		4			
#define M_XFRA_A_FLD_IDX		5			
#define M_XFRA_CI_FLD_IDX		6

#define M_XFRA_ID_IDX			7			//fixed Header-Field
#define M_XFRA_ACCESS_IDX		15
#define M_XFRA_STATUS_IDX		16	
#define M_XFRA_SIG1_IDX			17			// 1. Signaturfeld

#define M_HDR2_CNT				7			// Anzahl HeaderBytes f. C/L-Frame
#define M_SIG_CNT				2

enum M_REC_STATE {  // Empfangsstati MBus-Schnittstelle
		M_REC_IDLE   = 0,
		M_REC_L_FLD1,
		M_REC_L_FLD2,
		M_REC_START_CL,
		M_REC_C_FLD,
		M_REC_A_FLD,
		M_REC_CI_FLD,
		M_REC_DATA,
		M_REC_BCC,
		M_REC_STOP
};

struct tl1_rec {
	unsigned char 	state;			// status			
	unsigned char 	attr;			// spezielle Attribute	   
	unsigned char 	fld_c;
	unsigned char 	fld_a;
	unsigned char	fld_l;			 
	unsigned char 	fld_ci;
	char			bcc;			// lfd. blockcheck
	unsigned int	len;			// lfd. Länge mit C,A,CI 
	unsigned int	cnt;			// lfd. Zeichenanzahl in buf (ohne Steuerzeug,...)
	char 			buf[MBUS_TLG_MAX_LEN];
	unsigned int	stack;			// Arbeitszähler 
};

struct tl1_snd {
	unsigned int	cnt;			// lfd. Zeichenanzahl in buf (mit Steuerzeug,...)
	unsigned int	backup;			// Zeichenanzahl gesichert
	char 			buf[MBUS_TLG_MAX_LEN];
};

// LAYER1 Struktur
struct	m_l1 {  
	struct	tl1_rec rec;
	struct 	tl1_snd snd;
};

extern int 		m_cl1_format(char * buf, int idx);
extern size_t 	m_cl1_record(char *tlg, size_t count, struct tl1_rec * rec );


#endif // __MBUS_CL1_H__
