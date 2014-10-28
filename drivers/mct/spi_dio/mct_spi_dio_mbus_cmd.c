/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
 	
 	Description:  MBUS Command-Implementation for spi_dio-Driver

 *******************************************************************************/
#include "mct_spi_dio_objs.h"		// objects

#include "mct_spi_dio_mbus_cfg.h"	// Konfiguration
#include "mct_spi_dio_mbus_DI.h"	// Gerät:1 mit digitalen Eingängen
#include "mct_spi_dio_mbus_DO.h"	// Gerät:2 mit digitalen Ausgängen + Handmaske

#include "../if/mbus_cl2.h"
#include "../if/mbus_app.h"

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
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;	// ist initialisiert! 
	OBJS_CREATE_ODEV_PTR;	// ist initialisiert! 
	CREATE_L1_PTR;
	u8 value8 = 0;
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_application_reset() mask:[0x%02x]\n",mask);
	
	if (l1->rec.buf[l1->rec.stack] == APPLICATION_RESET_USER_DATA) {
		// GERÄT:1 
		if(	_bitset(mask,MBUS_SLV_IN)) {
			MBUS_CMD_TRACE("APPLICATION_RESET_USER_DATA: %d\n",MBUS_SLV_IN);
			l2->slvs[MBUS_SLV_IN].access_cnt = 1;
			l2->sel_without[MBUS_SLV_IN] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);	
			OBJS_IDEV_WR_INPUT(idev,0);		// input löschen ?
		}
		// GERÄT:2
		if(	_bitset(mask,MBUS_SLV_OUT)) {
			MBUS_CMD_TRACE("APPLICATION_RESET_USER_DATA: %d\n",MBUS_SLV_OUT);
			l2->slvs[MBUS_SLV_OUT].access_cnt = 1;
			l2->sel_without[MBUS_SLV_OUT] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);	
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,value8);	// output löschen
			OBJS_ODEV_WR_INIT_4Bit(odev,value8);	// init löschen
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
	// GERÄT:2 
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
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR;	// ist initialisiert! 
	CREATE_L1_PTR;
	unsigned char 	idx = l1->rec.stack;
	unsigned char	flg_write;
	u8				value8 = 0;
	u8				output8 = 0;

	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_var_data() mask:[0x%02x]\n",mask);	

	// GERÄT:0
	// Keine Aufgaben ...
	// GERÄT:1
	if(_bitset(mask,MBUS_SLV_OUT))	{
		/// DATA_8 (OUTPUT) (Type 8Bit,Storage0)
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
			memcpy(&output8,&l1->rec.buf[idx],sizeof(u8));
			idx=idx+1;
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,output8);
			MBUS_CMD_TRACE("OUTPUT: (write/replace) New: %u\n",output8);	
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  OR
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_or,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			memcpy(&value8,&l1->rec.buf[idx],sizeof(u8));
			OBJS_ODEV_RD_OUTPUT(odev,output8);	
			output8 = value8 | output8;
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,output8);
			MBUS_CMD_TRACE("OUTPUT: (OR) New: %u\n",output8);	
			idx=idx+1;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  AND
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_and,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			memcpy(&value8,&l1->rec.buf[idx],sizeof(u8));
			OBJS_ODEV_RD_OUTPUT(odev,output8);
			output8 = value8 & output8;
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,output8);
			idx=idx+1;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  XOR (toggle bits)
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_xor,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			memcpy(&value8,&l1->rec.buf[idx],sizeof(u8));			
			OBJS_ODEV_RD_OUTPUT(odev,output8);
			output8 = value8 ^ output8;
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,output8);
			idx=idx+1;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  AND NOT (clear bits)
		if( memcmp(&l1->rec.buf[idx],	\
					dscr_DO_andnot,	\
					DSCR_DO_ACTION_LEN ) == 0)
		{	
			idx = idx + DSCR_DO_ACTION_LEN;
			memcpy(&value8,&l1->rec.buf[idx],sizeof(u8));			
			OBJS_ODEV_RD_OUTPUT(odev,output8);	
			output8 = output8 & (~value8);
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,output8);
			idx=idx+1;
			l1->rec.stack = idx;
			return 1;
		}
		// ACTION:  CLEAR
		if( memcmp(&l1->rec.buf[idx],	\
				dscr_DO_clear,	\
				DSCR_DO_ACTION_LEN ) == 0)	{	
			idx = idx + DSCR_DO_ACTION_LEN;
			OBJS_ODEV_WR_OUTPUT_4Bit(odev,output8);
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
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;	// ist initialisiert! 
	OBJS_CREATE_ODEV_PTR;	// ist initialisiert! 
	unsigned char 		idx 	= 0;
	char				bcc 	= 0; 
	unsigned char 		chan 	= l2->slvs_idx; // aktiver Kanal
	unsigned char 		value8;
	u8					output8=0;
#ifdef CONFIG_MBUS_CMD_TRACE
	unsigned int 		i = 0;
#endif
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",chan);
	bcc = m_cl2_header_build(l2, chan, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!
	
	// GERÄT:0
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
	
	// GERÄT:1
	if(chan == MBUS_SLV_OUT) {		
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		// BINARY_1BYTE (OUTPUT) - Register (Type 8Bit,Storage0)
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		memcpy(&l1->snd.buf[idx],dscr_DO_standard,DSCR_DO_STANDARD_LEN);
		bcc+=	(unsigned char) (DSCR_DO_BCC);
		idx = idx + DSCR_DO_STANDARD_LEN;		
		OBJS_ODEV_RD_OUTPUT(odev,output8);
		memcpy(&l1->snd.buf[idx],&output8,sizeof(u8));
		bcc+= l1->snd.buf[idx];
		idx++;

		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		// BINARY_1BYTE (INIT) - Register (Type 8Bit,Storage1)
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		memcpy(&l1->snd.buf[idx],dscr_INIT_standard,DSCR_INIT_STANDARD_LEN);
		bcc+=	(unsigned char) (DSCR_INIT_BCC);
		idx = idx + DSCR_INIT_STANDARD_LEN;		
		OBJS_ODEV_RD_INIT(odev,value8);
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
