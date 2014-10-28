/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MODBUS SLAVE LAYER2

 *******************************************************************************/
#ifndef __MODBUS_SL2_H__
	#define __MODBUS_SL2_H__

#include "modbus_sl1.h"

#define MODBUS_SLAVES_MAX		4

// Bit AN/AUS
#define _bit_test(d,b) ((d)&(1<<b))

// größter zulässiger baud wert = 7
#define VIRTUAL_BAUD_001200		0 
#define VIRTUAL_BAUD_002400		1 
#define VIRTUAL_BAUD_004800		2 
#define VIRTUAL_BAUD_009600		3 
#define VIRTUAL_BAUD_019200		4 
#define VIRTUAL_BAUD_038400		5 
#define VIRTUAL_BAUD_057600		6 
#define VIRTUAL_BAUD_115200		7 

// Bit0: Parity odd,     Parity even
// Bit1: Parity disable, Parity enable
#define VIRTUAL_PARITY_OFF		0
#define VIRTUAL_PARITY_ON		2
#define VIRTUAL_PARITY_ODD		0 | VIRTUAL_PARITY_ON 
#define VIRTUAL_PARITY_EVEN		1 | VIRTUAL_PARITY_ON

struct t_slv	{
	unsigned char 	pri_addr;		// aktuelle ID = Modbus Adresse
	unsigned char	flg_ListenOnly;
	unsigned char	CommMode;		// virtueller Mode
	unsigned char	CommBaud;		// virtuelle Baudrate
	s16				cnt[5];			// BUS_ERROR,BUS_MESSAGE,SLV_MESSAGE,SLV_EXCEPT,SLV_NORESP
};

struct mod_l2 {									// MBUS-Infos		
	unsigned char 	inst;						// Instanznummer des Treibers,	
	unsigned char 	slvs_idx;					// Slave-Index (aktuell)
	unsigned char 	slvs_cfg;					// configurierte Slaves 
	unsigned char	broadcast;					// Slaves wurden per Broadcast-Adresse gerufen
	char		  	exc;						// Exception-Code (aktuell)
	struct 			t_slv  slvs[MODBUS_SLAVES_MAX];	// MBUS-Slaves
	struct 			mod_l1 l1;
	/// STUB's
	int 			(* cmd_rsp_01)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_02)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_03)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_04)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_05)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_06)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_15)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_16)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_23)			(struct mod_l2 *l2);
	int 			(* cmd_rsp_mei_14_code)	(struct mod_l2 *l2, unsigned char code);
	void * pdata;	// Zeiger auf Datenstruktur des zentralen O_MCT
};

extern int			mod_sl2_prim_add_check 			(struct mod_l2 * l2);

extern int 			mod_sl2_special_baudrate		(struct mod_l2 * l2);

extern unsigned char mod_sl2_diagn_00				(struct mod_l2 * l2);
extern void 		mod_sl2_diagn_return_query_data	(struct mod_l2 * l2);
extern void			mod_sl2_diagn_restart_comm 		(struct mod_l2 * l2);
extern void			mod_sl2_diagn_listen_only 		(struct mod_l2 * l2);
extern void			mod_sl2_diagn_clear_counter		(struct mod_l2 * l2);

/// BASIC STUB's
extern int 			mod_sl2_cmd_rsp_01		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_02		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_03		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_04		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_05		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_06		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_15		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_16		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_23		(struct mod_l2 * l2);
extern int 			mod_sl2_cmd_rsp_mei_14_code	(struct mod_l2 * l2, unsigned char code);


#endif // __MODBUS_SL2_H__
