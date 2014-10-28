/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		25.08.2011
 	
 	Description: MBUS-Command-Implementation for mct_paa_di4-Driver
	
 *******************************************************************************/
#include "mct_di4_objs.h"			// objects

#include "mct_di4_mbus_cfg.h"		// Konfiguration
#include "mct_di4_mbus_DI.h"		// Gerät:1 mit digitalen Eingängen

#include "../../if/mbus_cl2.h"
#include "../../if/mbus_app.h"

// Trace-Makro
#ifdef CONFIG_MBUS_CMD_TRACE
	#define MBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MBUS_CMD_TRACE(args...);
#endif

#define CREATE_L2_PTR		struct m_l2  * 	l2  = &(la->l2)
#define CREATE_L1_PTR 		struct m_l1  *  l1  = &(l2->l1)

#define STORAGE_START_POS 	0

// ------------------------------------------------------------------
// SOFTWARE-DESCRIPTOR 
// ------------------------------------------------------------------
#define DSCR_SOFTWARE_VERSION_LEN	10
#define DSCR_SOFTWARE_VERSION_BCC	\
	(	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH) +	\
		VIF_LINEAR_EXTENTION_FD + \
		VIFE_FD_SOFTWARE_VERSION + \
		6 + '0'+'0'+'.'+'1'+' '+'V')

// 0x0d, 0xfd, 0x0f,[]
const unsigned char dscr_software_version[] =	\
	{	(DIF_STORAGE0 | DIF_DATA_VARIABLE_LENGTH),	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_SOFTWARE_VERSION,	\
		6,'0','0','.','1',' ','V'};

/// ------------------------------------------------------------------
/// APPLIKATION_RESET (OVERLOADED) Multi-Modul
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_application_reset(struct m_l2 * l2, unsigned char mask)
{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;	// ist initialisiert! 
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_application_reset() mask:[0x%02x]\n",mask);
	
	if (l1->rec.buf[l1->rec.stack] == APPLICATION_RESET_USER_DATA) {
		// GERÄT:1 
		if(	_bitset(mask,MBUS_SLV_IN)) {
			MBUS_CMD_TRACE("APPLICATION_RESET_USER_DATA: %d\n",MBUS_SLV_IN);
			l2->slvs[MBUS_SLV_IN].access_cnt = 1;
			l2->sel_without[MBUS_SLV_IN] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);	
			OBJS_IDEV_WR_INPUT(idev,0);		// input löschen ?
		}
	}
	else {
		MBUS_CMD_TRACE("RESET-Subcode:0x%02x Unsupported!\n",l1->rec.buf[l1->rec.stack]);
	}	
	return 1;
}

/// ------------------------------------------------------------------
/// GLOBAL_READOUT (OVERLOADED) Multi-Modul
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_global_readout(struct m_l2 * l2, unsigned char mask) {
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_global_readout() mask:[0x%02x]\n",mask);
	// GERÄT:1 
	if(_bitset(mask,MBUS_SLV_IN))	{
			l2->sel_without[MBUS_SLV_IN] |= SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION;
	}
	return 1;
}


/// ------------------------------------------------------------------
/// RSP_UD_DATA2 (OVERLOADED) Multi-Modul
/// ------------------------------------------------------------------
// SOFTWARE VERSION - Register (Type variabel 6Byte, Storage0)
unsigned char _cmd_rsp_ud_software(	struct m_l2 * l2,\
							struct m_l1 * l1,\
							unsigned char chan,\
							unsigned char idx,\
							char *bcc)
{
	MBUS_CMD_TRACE("OVERLOAD: _cmd_rsp_ud_software() channel:[%d]\n",chan);	
	if(l2->sel_without[chan] & SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION)
	{
		memcpy(&l1->snd.buf[idx],&dscr_software_version, DSCR_SOFTWARE_VERSION_LEN);
		bcc 	+= (unsigned char) (DSCR_SOFTWARE_VERSION_BCC);
		idx = idx + DSCR_SOFTWARE_VERSION_LEN;
		// einmaliges auslesen beendet: 
		l2->sel_without[chan] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);
	}
	return idx;		// idx aktualisiert!
}

int mbus_cmd_rsp_ud_data2(struct m_app * la) {
	CREATE_L2_PTR;
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;	// ist initialisiert! 
	unsigned char 		idx 	= 0;
	char				bcc 	= 0; 
	unsigned char 		chan 	= l2->slvs_idx; // aktiver Kanal
	unsigned char 		value8	= 0;
#ifdef CONFIG_MBUS_CMD_TRACE
	unsigned int 		i = 0;
#endif
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",chan);
	bcc = m_cl2_header_build(l2, chan, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!
	
	// GERÄT:1
	if(chan == MBUS_SLV_IN) {
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		// BINARY_1BYTE (INPUT) - Register (Type 8Bit,Storage0)
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		memcpy(&l1->snd.buf[idx],dscr_DI_standard,DSCR_DI_STANDARD_LEN);
		bcc+=	(unsigned char) (DSCR_DI_BCC);
		idx = idx + DSCR_DI_STANDARD_LEN;

		OBJS_IDEV_RD_INPUT(idev,value8);

		l1->snd.buf[idx] = value8;
		bcc+= l1->snd.buf[idx];
		idx++;
	}
	
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// SOFTWARE
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	idx = _cmd_rsp_ud_software(l2,l1,chan,idx,&bcc);
	l1->snd.cnt = idx;
	l1->snd.buf[M_XFRA_L_FLD1_IDX]					= l1->snd.cnt-4;
	l1->snd.buf[M_XFRA_L_FLD2_IDX]					= l1->snd.cnt-4;
	l1->snd.buf[l1->snd.buf[M_XFRA_L_FLD1_IDX]+4]	= bcc;
	l1->snd.cnt++;
	l1->snd.buf[l1->snd.buf[M_XFRA_L_FLD1_IDX]+5]	= FT_1_2_STP;
	l1->snd.cnt++;
	#ifdef CONFIG_MBUS_CMD_TRACE
		printk("RSP_UD_DATA2 ");	
		for (i = 0; i< l1->snd.cnt; i++)
			printk("%02x ",l1->snd.buf[i]); 
		printk("\n");
	#endif	
	return 1;
};
