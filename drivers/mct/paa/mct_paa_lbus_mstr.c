/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

*********************************************************************************/
#include <linux/module.h>
#include "../mct_debug.h"			// debug-support

#include "mct_paa_lbus_mstr.h"

// BUS-UP-Nachricht
unsigned char msg_lbus_up[LBUS_DEF_TLG_LEN] = {\
	LBUS_SIGN_START,\
	LBUS_ADD_FF,\
	LBUS_BUS_UP,\
	LBUS_ERR_NONE,\
	0,\
	0xE1,\
	LBUS_SIGN_STOP};

// BUS-Down-Nachricht
unsigned char msg_lbus_down[LBUS_DEF_TLG_LEN] = {\
	LBUS_SIGN_START,\
	LBUS_ADD_FF,\
	LBUS_BUS_DOWN,\
	LBUS_ERR_NONE,\
	0,\
	0xF1,\
	LBUS_SIGN_STOP};

// Telegramme werden initialisiert
// Links auf die Telegramme werden gesetzt
int paa_lbus_mstr_init(struct lower_bus * pbus, int slots)	{
	unsigned char tmp_req[LBUS_DEF_TLG_LEN];
	unsigned char slot 	= 0;
	unsigned char req 	= 0;
	unsigned char crc 	= 0;
	
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s() SLOTS: %d\n",__FILE__, __FUNCTION__,slots);
	
	memset(pbus,0,sizeof(struct lower_bus));

	// Copy predefined Bus-Messages
 	memcpy(&pbus->msg_up,msg_lbus_up,LBUS_DEF_TLG_LEN);
 	memcpy(&pbus->msg_down,msg_lbus_down,LBUS_DEF_TLG_LEN);

	// Build predefined Messages!
	for(slot=0; slot < slots; slot++) {
		for(req=0; req < LBUS_DEF_TLGS; req++) {	
			// 2er Kompliment bauen
			crc = (unsigned char)(~(slot + req + 0) +1);
			tmp_req[LBUS_IDX_STA] = LBUS_SIGN_START;
			tmp_req[LBUS_IDX_ADD] = slot;
			tmp_req[LBUS_IDX_CMD] = req;
			tmp_req[LBUS_IDX_ERR] = LBUS_ERR_NONE;
			tmp_req[LBUS_IDX_LEN] = 0;
			tmp_req[LBUS_IDX_CRC] = crc;
			tmp_req[LBUS_IDX_STP] = LBUS_SIGN_STOP;
			switch( req) {
				case LBUS_RES:	//0
					memcpy(&pbus->snd[slot].msg_reset,tmp_req,LBUS_DEF_TLG_LEN);
					pbus->snd[slot].msgs[LBUS_RES]  = pbus->snd[slot].msg_reset;
					break;
				case LBUS_WHO:	//1
					memcpy(&pbus->snd[slot].msg_who,tmp_req,LBUS_DEF_TLG_LEN);
					pbus->snd[slot].msgs[LBUS_WHO]  = pbus->snd[slot].msg_who;
					break;
				case LBUS_PING:	//2
					memcpy(&pbus->snd[slot].msg_ping,tmp_req,LBUS_DEF_TLG_LEN);
					pbus->snd[slot].msgs[LBUS_PING]  = pbus->snd[slot].msg_ping;
					break;
				case LBUS_CFG:	//3
					memcpy(&pbus->snd[slot].msg_cfg,tmp_req,LBUS_DEF_TLG_LEN);
					pbus->snd[slot].msgs[LBUS_CFG]  = pbus->snd[slot].msg_cfg;
					break;
				case LBUS_DATA:	//4 leere Datenanfrage, wenn z.B. nur Inputs
					memcpy(&pbus->snd[slot].msg_data,tmp_req,LBUS_DEF_TLG_LEN);
					pbus->snd[slot].msgs[LBUS_DATA]  = pbus->snd[slot].msg_data;
					break;
				default:
					break;
			}
/*
			printk("Slot[%d].Tlg[%d]-%2.0x-%2.0x-%2.0x-%2.0x-%2.0x-%2.0x-%2.0x\n",slot,req,\
					pbus->snd[slot].msgs[req][0],pbus->snd[slot].msgs[req][1],\
					pbus->snd[slot].msgs[req][2],pbus->snd[slot].msgs[req][3],
					pbus->snd[slot].msgs[req][4],pbus->snd[slot].msgs[req][5],
					pbus->snd[slot].msgs[req][6]);
*/
		pbus->snd[slot].cmd = LBUS_REQ_WHO;
		}	// End slots
	}	// End req
	pbus->mode = LBUS_MODE_IDLE;
	db_lbus(LBUS_MODE_IDLE_STR);
	return 0;
}

int paa_lbus_mstr_delete(struct lower_bus * pbus)	{
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s()\n",__FILE__, __FUNCTION__);
	pbus->mode = LBUS_MODE_DOWN;
	return 0;
}
