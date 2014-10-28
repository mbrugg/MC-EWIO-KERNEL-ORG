/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$ 	01.09.2011
 	
 	Description: MBUS-Command-Implementation for mct_paa_ai8-Driver

 *******************************************************************************/
#include "mct_ai8_objs.h"		// objects

#include "mct_ai8_mbus_cfg.h"		// MBUS Konfiguration
#include "mct_ai8_mbus_AI.h"		// Geräte:0-7 mit analog Eingang
#include "mct_ai8_mbus_cmd.h"

#include "../../if/mbus_cl2.h"
#include "../../if/mbus_app.h"

#include "../../mct_aio_uni.h"
#include "../../mct_boxes.h"

// Trace-Makro
#ifdef CONFIG_MBUS_CMD_TRACE
	#define MBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MBUS_CMD_TRACE(args...);
#endif

#define CREATE_DRV_PTR 		struct tai8_drv *drv = (struct tai8_drv *)(l2->pdata)
#define CREATE_L2_PTR		struct m_l2  * 	l2  = &(la->l2)
#define CREATE_L1_PTR 		struct m_l1  *  l1  = &(l2->l1)
#define CREATE_IDEV_PTR		struct tai8_dev_inp   * idev = NULL

#define GET_DRV_FIRMWARE_PIC(val)	{	\
	spin_lock_bh(&drv->Op_Physic->lock);	\
	val = drv->Op_Physic->P_PICLayer.value;	\
	spin_unlock_bh(&drv->Op_Physic->lock);	}

// Hier: Geräte 0-7 (INPUT's)
#define GET_DEV_INPUT32(channel,val)	{	\
	idev = drv->Op_Inputs[(channel)];	\
	spin_lock_bh(&idev->Op_Value->lock);	\
	val = idev->Op_Value->P_Value.value;	\
	spin_unlock_bh(&idev->Op_Value->lock);}

#define SET_DEV_INPUT32(channel,val)	{	\
	idev = drv->Op_Inputs[(channel)];	\
	spin_lock_bh(&idev->Op_Value->lock);	\
	idev->Op_Value->P_Value.value = val;	\
	spin_unlock_bh(&idev->Op_Value->lock);}

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

// ------------------------------------------------------------------
// FIRMWARE-VERSION (PIC)  
// ------------------------------------------------------------------
#define DSCR_FIRMWARE_VERSION_LEN	0x03
#define DSCR_FIRMWARE_VERSION_BCC	\
	(	DIF_DATA_VARIABLE_LENGTH +	\
		VIF_LINEAR_EXTENTION_FD + \
		VIFE_FD_FIRMWARE_VERSION )

// 0x0d, 0xfd, 0x0e,[]
const unsigned char dscr_firmware_version[] =	\
	{	DIF_DATA_VARIABLE_LENGTH,	\
		VIF_LINEAR_EXTENTION_FD,	\
		VIFE_FD_FIRMWARE_VERSION};

/// ------------------------------------------------------------------
/// APPLIKATION_RESET (OVERLOADED) Multi-Modul
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_application_reset(struct m_l2 * l2, unsigned char mask)
{
	CREATE_DRV_PTR;
	CREATE_L1_PTR;
	CREATE_IDEV_PTR;
	unsigned char 	chan 		= 0;
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
					// GERÄT:0-7
					SET_DEV_INPUT32(chan,0);
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
	CREATE_IDEV_PTR;	// NULL, muss noch initialisiert werden!
	unsigned char 		idx 	= 0;
	char				bcc 	= 0; 
	unsigned char 		chan 	= l2->slvs_idx; // aktiver Kanal
	uint32_t 			value32 	= 0;
	unsigned char		value8		= 0;
	unsigned char		kennlinie	= 0;
	unsigned char 		buf[50];
#ifdef CONFIG_MBUS_CMD_TRACE
	unsigned int 		i = 0;
#endif
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",chan);
	bcc = m_cl2_header_build(l2, chan, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!
	
	idev = drv->Op_Inputs[chan];
	value8 = box_sprintf_sel(&(idev->Op_Type->P_Type),buf);
	if(strstr(buf,AI_PROP_MODE_PATTERN_0)) {
		// "Ohm"
		memcpy(&l1->snd.buf[idx],dscr_AI_ohm,DSCR_AI_OHM_LEN);
		bcc+=	(unsigned char) (DSCR_AI_OHM_BCC);
		idx = idx + DSCR_AI_OHM_LEN;
	}
	else if (strstr(buf,AI_PROP_MODE_PATTERN_2)){
		// "Volt"
		memcpy(&l1->snd.buf[idx],dscr_AI_volt,DSCR_AI_VOLT_LEN);
		bcc+=	(unsigned char) (DSCR_AI_VOLT_BCC);
		idx = idx + DSCR_AI_VOLT_LEN;
	}
	else {
		// unbekannt - z.B inactive
//		printk("Inactive!\n");
		memcpy(&l1->snd.buf[idx],dscr_AI_undef,DSCR_AI_UNDEF_LEN);
		bcc+=	(unsigned char) (DSCR_AI_UNDEF_BCC);
		idx = idx + DSCR_AI_UNDEF_LEN;
	}
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// DATA_32 (INPUT) - Register (Type 32Bit,Storage0)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	GET_DEV_INPUT32(chan,value32);
	memcpy(&l1->snd.buf[idx],&value32,sizeof(uint32_t));
	bcc+= l1->snd.buf[idx];
	idx++;
	bcc+= l1->snd.buf[idx];
	idx++;
	bcc+= l1->snd.buf[idx];
	idx++;
	bcc+= l1->snd.buf[idx];
	idx++;

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// DIF_DATA_VARIABLE_LENGTH (SENSOR) String
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	memcpy(&l1->snd.buf[idx],dscr_AI_sensor,DSCR_AI_SENSOR_LEN);
	bcc+=	(unsigned char) (DSCR_AI_SENSOR_BCC);
	idx = idx + DSCR_AI_SENSOR_LEN;
	// index in der BOX ermitteln, welche Kennlinie
	kennlinie = box_get_sel(&(idev->Op_Type->P_Type));
	value8 = strlen(ai_label_tab[kennlinie]);
	memcpy(buf,ai_label_tab[kennlinie],value8);
	l1->snd.buf[idx] = value8;	// Länge speichern
	bcc+= l1->snd.buf[idx];
	idx++;			
	while(value8--) {
		l1->snd.buf[idx] = buf[value8];	// rückwärts lesen!
		bcc+= l1->snd.buf[idx];
		idx++;
	}

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
