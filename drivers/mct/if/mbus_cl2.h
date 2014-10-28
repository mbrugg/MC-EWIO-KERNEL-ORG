/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS CIENT LAYER2

	// Einige Subfunktionen können durch die Anwendung überladen werden,
	// um sie dem Gerätetyp anzupassen ...
	//	[OVERLOAD BY USER]
	int (* cmd_snd_ud_application_reset)(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_global_readout)	(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_mfr_data)			(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_var_data)			(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_usr_data)			(struct m_l2 *l2, unsigned char mask);

 *******************************************************************************/
#ifndef __MBUS_CL2_H__
	#define __MBUS_CL2_H__

#include "mbus_cl1.h"

#define MBUS_SLAVES_MAX		8

#define INCLUDE_SUPPORT_SWITCH_MBUS_REQ_UD1

struct t_slv	{
	unsigned char pri_addr;			// aktuelle primäre M-Bus adresse
	unsigned char sec_addr[8];		// Sek-Addr. incl. manufacturer, version, medium
	unsigned char state;
	unsigned char access_cnt;
	unsigned char signature[2];	
};

struct m_l2 {						// MBUS-Infos		
	unsigned char inst;				// Instanznummer des Treibers,	
	unsigned char slvs_idx;			// aktueller Slave-Index 
	unsigned char slvs_cfg;			// configurierte Slaves 
	struct t_slv  slvs[MBUS_SLAVES_MAX];//MBUS-Slaves		

									// Baudratenbereich:
	unsigned char baud_idx;			// aktueller  Baudraten-Index
	unsigned char baud_wish;		// Wunschbaudrate
	unsigned char baud_tlg;			// Telegrammmerker (enthält tlg_rdy_100cnt)
	unsigned char baud_state;		// Status der Baudratenumschaltung z.Z. frei
	unsigned char tlg_rdy_100cnt;	// Zählt alle empfangenen phys. i.O. Telegramme
 	struct m_l1		l1;
	int (* cmd_snd_ud_application_reset)(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_global_readout)	(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_mfr_data)			(struct m_l2 *l2, unsigned char mask);
	int (* cmd_snd_ud_usr_data)			(struct m_l2 *l2, unsigned char mask);
	unsigned char sel_with[MBUS_SLAVES_MAX];	// bis 8 DATA_RECORDS selektiv addiert/elemeniert
	unsigned char sel_without[MBUS_SLAVES_MAX];// bis 8 DATA_RECORDS zusätzlich bei global readout
	void * pdata;					// Zeiger auf Datenstruktur des zentralen O_MCT
									// für den Zugriff
};


extern int	 m_cl2_primaer_check	(struct m_l2 * l2);	// multi-channel
extern void	 m_cl2_conf_ack	\
			(struct m_l2 * l2, signed char count);		// -1= No NKE, count=anzahl NKE
extern char	m_cl2_header_build	\
			(struct m_l2 * l2, unsigned char channel, unsigned char type );
extern int 	m_cl2_snd_ud_data	\
			(struct m_l2 * l2, unsigned char mask);		// FIX, see STUB of subfkts 1-4

/// OVERLOADABLE ...
//-------------------------------------------------------------------------------------
extern int  _cmd_snd_ud_global_readout	\
			(struct m_l2 * l2, unsigned char mask);		// STUB subfkt 1
extern int  _cmd_snd_ud_var_data	\
			(struct m_l2 * l2, unsigned char mask);		// STUB subfkt 2
extern int  _cmd_snd_ud_mfr_data	\
			(struct m_l2 * l2, unsigned char mask);		// STUB subfkt 3
extern int  _cmd_snd_ud_usr_data	\
			(struct m_l2 * l2, unsigned char mask);		// STUB subfkt 4
//-------------------------------------------------------------------------------------
extern int  _cmd_snd_ud_application_reset	\
			(struct m_l2 * l2, unsigned char mask);		// STUB

/* Bit setzen, loeschen, testen auf gesetzt, geloescht */
#define _setbit(d,b) (d|=1<<b)
#define _clrbit(d,b) (d&=~(1<<b))
#define _bitset(d,b) (((d)&(1<<b))!=0)
#define _bitclr(d,b) ((d&1<<b)==0)

/* teste ob ein MSBit in d hoeher b gesetz, geloescht ist */
#define _msbset(d,b) ((d>>b+1)!=0)
#define _msbclr(d,b) (~(d>>b+1)!=0)

/* loescht / setzt MSBits ab */
#define _clrmsb(d,b) (d&=(1<<b)-1)


#endif // __MBUS_CL2_H__
