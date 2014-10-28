/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	30.08.2011
 	
 	Description: MBUS-Command-Implementation for mct_paa_do4-Driver

 *******************************************************************************/
#include "mct_do4_objs.h"		// objects

#include "mct_do4_mbus_cfg.h"		// Konfiguration
#include "mct_do4_mbus_DO.h"		// Gerät:1 mit digitalen Ausgängen + Handmaske

#include "../../if/mbus_cl2.h"
#include "../../if/mbus_app.h"

// Trace-Makro
#ifdef CONFIG_MBUS_CMD_TRACE
	#define MBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MBUS_CMD_TRACE(args...);
#endif

#define CREATE_DRV_PTR 		struct tdo4_drv *drv = (struct tdo4_drv *)(l2->pdata)
#define CREATE_ODEV_PTR		struct tdo4_dev_outp  * odev = drv->Op_Outputs
#define CREATE_L2_PTR		struct m_l2  * 	l2  = &(la->l2)
#define CREATE_L1_PTR 		struct m_l1  *  l1  = &(l2->l1)

// Hinweis: OUTPUT-Feedback-Variable lesen
#define GET_DEV_OUTPUT(val)	{	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	val = odev->Op_Value->P_ValueIn.value;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

#define SET_DEV_OUTPUT(val)	{	\
	val = val & MBUS_SLV_OUT_BITS;	\
	spin_lock_bh(&odev->Op_Value->lock);	\
	odev->Op_Value->P_ValueOut.value= val;	\
	spin_unlock_bh(&odev->Op_Value->lock);}

#define GET_DEV_MASK(val)	{	\
	spin_lock_bh(&odev->Op_Mask->lock);	\
	val = odev->Op_Mask->P_Value.value;	\
	spin_unlock_bh(&odev->Op_Mask->lock);}

#define SET_DEV_MASK(val)	{	\
	spin_lock_bh(&odev->Op_Mask->lock);	\
	odev->Op_Mask->P_Value.value=val;	\
	spin_unlock_bh(&odev->Op_Mask->lock);}

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
	CREATE_ODEV_PTR;	// ist initialisiert! 
	CREATE_L1_PTR;
	unsigned char	value8 = 0;
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_application_reset() mask:[0x%02x]\n",mask);

	if (l1->rec.buf[l1->rec.stack] == APPLICATION_RESET_USER_DATA) {
		if(	_bitset(mask,MBUS_SLV_OUT)) {
			MBUS_CMD_TRACE("APPLICATION_RESET_USER_DATA: %d\n",MBUS_SLV_OUT);
			l2->slvs[MBUS_SLV_OUT].access_cnt = 1;
			l2->sel_without[MBUS_SLV_OUT] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);	
			SET_DEV_OUTPUT(value8);		// output löschen
			SET_DEV_MASK(value8);		// maske löschen
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
	
	// GERÄT:0
	if(_bitset(mask,MBUS_SLV_OUT))	{
			l2->sel_without[MBUS_SLV_OUT] |= SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION;
	}
	return 1;
}

/// ------------------------------------------------------------------
/// SND_UD_USR_DATA (OVERLOADED) Multi-Modul 
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_usr_data(struct m_l2 * l2, unsigned char mask)
{
	CREATE_DRV_PTR;
	CREATE_ODEV_PTR;	// ist initialisiert! 
	CREATE_L1_PTR;
	unsigned char 		idx = l1->rec.stack;
	unsigned char		flg_write;
	unsigned char		value8 = 0;
	unsigned char		output8 = 0;
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_usr_data() mask:[0x%02x]\n",mask);	

	// GERÄT:0
	if(_bitset(mask,MBUS_SLV_OUT))	{
		/// DATA_8 	(OUTPUT) (Type 8Bit,Storage0)
		// ACTION:  (Kurzbefehl write/replace)
		flg_write = 0;
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_standard,	\
					DSCR_DO_STANDARD_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_STANDARD_LEN;
			flg_write = 1;
		}
		// ACTION:  (Langbefehl write/replace) 
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_write,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			flg_write = 1;			
		}
		// ACTION: WRITE/REPLACE
		if(	flg_write) {
			output8 = l1->rec.buf[idx];
			SET_DEV_OUTPUT(output8);
			MBUS_CMD_TRACE("DO: OUTPUT (write/replace) New: 0x%02x\n",output8);
			idx++;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  OR
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_or,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			value8 = l1->rec.buf[idx];	//0xaa
			GET_DEV_OUTPUT(output8);	
			output8 = value8 | output8;
			SET_DEV_OUTPUT(output8);
			MBUS_CMD_TRACE("DO: OUTPUT (OR) New: 0x%02x\n",output8);
			idx++;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  AND
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_and,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			value8 = l1->rec.buf[idx];
			GET_DEV_OUTPUT(output8);
			output8 = value8 & output8;
			SET_DEV_OUTPUT(output8);
			MBUS_CMD_TRACE("DO: OUTPUT (AND) New: 0x%02x\n",output8);
			idx++;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  XOR (toggle bits)
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_xor,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			value8 = l1->rec.buf[idx];
			GET_DEV_OUTPUT(output8);
			output8 = value8 ^ output8;
			SET_DEV_OUTPUT(output8);
			MBUS_CMD_TRACE("DO: OUTPUT (XOR) New: 0x%02x\n",output8);
			idx++;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  AND NOT (clear bits)
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_andnot,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			value8 = l1->rec.buf[idx];
			GET_DEV_OUTPUT(output8);	
			output8 = output8 & (~value8);
			SET_DEV_OUTPUT(output8);
			MBUS_CMD_TRACE("DO: OUTPUT (AND NOT) New: 0x%02x\n",output8);
			idx++;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  CLEAR
		if( memcmp(&l1->rec.buf[idx],	\
				dscr_DO_clear,	\
				DSCR_DO_ACTION_LEN ) == 0)	{	
			idx = idx + DSCR_DO_ACTION_LEN;
			SET_DEV_OUTPUT(output8);
			MBUS_CMD_TRACE("DO: OUTPUT (clear) New: 0x%02x\n",output8);
			l1->rec.stack = idx;
			return 1;
		}
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
	CREATE_ODEV_PTR;	// ist initialisiert! 
	unsigned char 		idx 	= 0;
	char				bcc 	= 0; 
	unsigned char 		chan 	= l2->slvs_idx; // aktiver Kanal
	unsigned char 		value8	= 0;
#ifdef CONFIG_MBUS_CMD_TRACE
	unsigned int 		i;
#endif
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",chan);
	bcc = m_cl2_header_build(l2, chan, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!	
	
	// GERÄT:0
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// BINARY_1BYTE (OUTPUT) - Register (Type 8Bit,Storage0)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	memcpy(&l1->snd.buf[idx],dscr_DO_standard,DSCR_DO_STANDARD_LEN);
	bcc+=	(unsigned char) (DSCR_DO_BCC);
	idx = idx + DSCR_DO_STANDARD_LEN;		
	GET_DEV_OUTPUT(value8);
	l1->snd.buf[idx] = value8;
	bcc+= l1->snd.buf[idx];
	idx++;
	
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// BINARY_1BYTE (HANDMASK) - Register (Type 8Bit,Storage1)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	memcpy(&l1->snd.buf[idx],dscr_MASK_standard,DSCR_MASK_STANDARD_LEN);
	bcc+=	(unsigned char) (DSCR_MASK_BCC);
	idx = idx + DSCR_MASK_STANDARD_LEN;		
	GET_DEV_MASK(value8);
	l1->snd.buf[idx] = value8;
	bcc+= l1->snd.buf[idx];
	idx++;
		
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
