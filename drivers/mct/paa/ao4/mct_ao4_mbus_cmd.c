/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	02.09.2011
 	
 	Description: MBUS-Command-Implementation for mct_paa_ao4-Driver
	
 *******************************************************************************/
#include "mct_ao4_objs.h"		// objects

#include "mct_ao4_mbus_cfg.h"		// MBUS Konfiguration
#include "mct_ao4_mbus_AO.h"		// Geräte:0-3 mit analog Ausgang + Handmaske
#include "mct_ao4_mbus_cmd.h"

#include "../../if/mbus_cl2.h"
#include "../../if/mbus_app.h"

// Trace-Makro
#ifdef CONFIG_MBUS_CMD_TRACE
	#define MBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MBUS_CMD_TRACE(args...);
#endif

#define CREATE_DRV_PTR 		struct tao4_drv *drv = (struct tao4_drv *)(l2->pdata)
#define CREATE_L2_PTR		struct m_l2  * 	l2  = &(la->l2)
#define CREATE_L1_PTR 		struct m_l1  *  l1  = &(l2->l1)
#define CREATE_ODEV_PTR		struct tao4_dev_outp  * odev = NULL

// Hier: Geräte 0-3 (OUTPUT's)
#define GET_DEV_OUTPUT16(channel,val)	{	\
	odev = drv->Op_Outputs[channel];	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	val = odev->Op_Value->P_Value.value;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

// Mit Limitüberwachung 10bit
#define SET_DEV_OUTPUT16(channel,val)	{	\
	if(val < 0)	\
		val = 0;	\
	else if (val > SIZE_MAX_10bit)	\
		val = SIZE_MAX_10bit;	\
	odev = drv->Op_Outputs[channel];	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	odev->Op_Value->P_Value.value = val;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

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
	CREATE_DRV_PTR;
	CREATE_L1_PTR;
	CREATE_ODEV_PTR;
	unsigned char 	chan 		= 0;
	u16				value16		= 0;
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_application_reset() mask:[0x%02x]\n",mask);

	// Aktive Channel in der Maske suchen!
	for (chan=0;chan < l2->slvs_cfg;chan++)	{
		if(_bitset(mask,chan))	{
			switch(l1->rec.buf[l1->rec.stack]) {
				case APPLICATION_RESET_USER_DATA:
					MBUS_CMD_TRACE("APPLICATION_RESET_USER_DATA: %d\n",chan);
					// GERÄT: ALLE
					l2->slvs[chan].access_cnt = 1;
					l2->sel_without[chan] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);
					l2->sel_without[chan] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_FIRMWAREVERSION);
					SET_DEV_OUTPUT16(chan,value16);
					break;
				default:
					MBUS_CMD_TRACE("RESET-Subcode:0x%02x Unsupported!\n",l1->rec.buf[l1->rec.stack]);	
					break;
			}	// End Subcode
			break;
		} 
	} // End for chan
	return 1;
}

/// ------------------------------------------------------------------
/// GLOBAL_READOUT (OVERLOADED) Multi-Modul
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_global_readout(struct m_l2 * l2, unsigned char mask) {
	unsigned char 	chan;
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_global_readout() mask:[0x%02x]\n",mask);

	// Aktive Channel in der Maske suchen!
	for (chan=0;chan < l2->slvs_cfg;chan++)	{
		// Für jedes Gerät ... 
		if(_bitset(mask,chan))	{
			l2->sel_without[chan] |= SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION;
			l2->sel_without[chan] |= SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_FIRMWAREVERSION;
		}
	}
	return 1;
}

/// ------------------------------------------------------------------
/// SND_UD_USR_DATA (OVERLOADED) Multi-Modul 
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_usr_data(struct m_l2 * l2, unsigned char mask)
{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;							// NULL, noch initialisieren! 
	CREATE_L1_PTR;
	unsigned char 		chan 		= 0;		// aktueller Channel aus _mask	
	unsigned char 		chan_ 		= 0xff;		// Hilfswert
	unsigned char 		idx 		= l1->rec.stack;
	uint16_t			value16		= 0;
	unsigned char		flg_write;
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_usr_data() mask:[0x%02x]\n",mask);	

	// GERÄT:0-3
	// Den ersten Channel suchen!
	for (chan=0;chan< l2->slvs_cfg; chan++)	
		if(_bitset(mask,chan))	{
			chan_ = chan;
			break;
		}
	// ABBRUCH, da kein Channel in mask gekennzeichnet ist!
	if( chan_ == 0xff) {
		MBUS_CMD_TRACE("ERROR!\n");
		return 0;
	}

	///DATA_16 OUTPUT-Register (Type 16Bit,Storage0)
	// ACTION:  (Kurzbefehl write/replace)
	flg_write = 0;
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_standard,	\
				DSCR_AO_STANDARD_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_STANDARD_LEN;
		flg_write = 1;
	}
	// ACTION:  (Langbefehl write/replace) 
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_write,	\
				DSCR_AO_ACTION_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_ACTION_LEN;
		flg_write = 1;
	}
	// ACTION: WRITE/REPLACE
	if(	flg_write) {
		memcpy(&value16,&l1->rec.buf[idx],sizeof(uint16_t));
		idx=idx+2;
		SET_DEV_OUTPUT16(chan,value16);

		MBUS_CMD_TRACE("AO: OUTPUT %d (write/replace) New: %u\n",chan,value16);	
		// Noch mehr Geräte aktiviert?
		for (chan = chan+1;chan < l2->slvs_cfg; chan++)	{	
			if(_bitset(mask,chan))	{	
				SET_DEV_OUTPUT16(chan,value16); // Werte: (==)
				MBUS_CMD_TRACE("AO: OUTPUT %d (write/replace) New: %u\n",chan,value16);	
			}
		}
		l1->rec.stack = idx;
		return 1;
	}
	// ACTION:  CLEAR
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_clear,	\
				DSCR_AO_ACTION_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_ACTION_LEN;
		SET_DEV_OUTPUT16(chan,value16);
		MBUS_CMD_TRACE("AO: OUTPUT %d (clear) New: %u\n",chan, value16);	
		// Noch mehr Geräte aktiviert?
		for (chan = chan+1;chan < l2->slvs_cfg; chan++)	{	
			if(_bitset(mask,chan)) {	
				SET_DEV_OUTPUT16(chan,value16); // Werte  (== 0)
				MBUS_CMD_TRACE("AO: OUTPUT %d (clear) New: %u\n",chan, value16);
			}
		}
		l1->rec.stack = idx;
		return 1;
	}
	return 0;
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
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;	// NULL, muss noch initialisiert werden! 
	unsigned char 		idx 	= 0;
	char				bcc 	= 0; 
	unsigned char 		chan 	= l2->slvs_idx; // aktiver Kanal
	uint16_t			value16		= 0;
#ifdef CONFIG_MBUS_CMD_TRACE
	unsigned int 		i = 0;
#endif
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",chan);
	bcc = m_cl2_header_build(l2, chan, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// DATA_16 (OUTPUT) - Register (Type 16Bit,Storage0)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	memcpy(&l1->snd.buf[idx],dscr_AO_standard,DSCR_AO_STANDARD_LEN);
	bcc+=	(unsigned char) (DSCR_AO_BCC);
	idx = idx + DSCR_AO_STANDARD_LEN;		
	GET_DEV_OUTPUT16(chan,value16);
	memcpy(&l1->snd.buf[idx],&value16,sizeof(uint16_t));
	bcc+= l1->snd.buf[idx];
	idx++;
	bcc+= l1->snd.buf[idx];
	idx++;

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// SOFTWARE (Treiber)
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
