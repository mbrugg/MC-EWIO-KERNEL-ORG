/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description: MBUS-Command-Implementation for mct_spi_aio-Driver
	
 *******************************************************************************/
#include "mct_spi_aio_objs.h"			// objects

#include "mct_spi_aio_mbus_cfg.h"		// MBUS Konfiguration
#include "mct_spi_aio_mbus_AI.h"		// Geräte:1-4 mit analog Eingang
#include "mct_spi_aio_mbus_AO.h"		// Geräte:5-7 mit analog Ausgang + Handmaske
#include "mct_spi_aio_mbus_cmd.h"

#include "../if/mbus_cl2.h"
#include "../if/mbus_app.h"

#include "../mct_boxes.h"

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
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR_NULL;
	OBJS_CREATE_ODEV_PTR_NULL;
	unsigned char 	chan = 0;

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
					switch(chan) { 
						case MBUS_SLV_IN0: // GERÄT:0-3
						case MBUS_SLV_IN1:
						case MBUS_SLV_IN2:
						case MBUS_SLV_IN3: 
							OBJS_CREATE_IDEV_PTR(chan-CHANNEL_IN_OFFSET);
							OBJS_IDEV_WR_INPUT(idev,0);
							break;
						case MBUS_SLV_OUT0: // GERÄT:4-6
						case MBUS_SLV_OUT1:
						case MBUS_SLV_OUT2:
						case MBUS_SLV_OUT3:
							OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
							OBJS_ODEV_WR_OUTPUT_xBit(odev,0);
							break;
						default:
							break;
					}
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
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_ODEV_PTR_NULL;
	unsigned char 		chan 		= 0;		// aktueller Channel aus _mask	
	unsigned char 		chan_ 		= 0xff;		// Hilfswert
	unsigned char 		idx 		= l1->rec.stack;
	uint16_t			value16		= 0;
	unsigned char		volt_write;
	unsigned char		ampere_write;
	
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_usr_data() mask:[0x%02x]\n",mask);	
	// GERÄT:0-3
	// Keine Aufgaben ...
	// GERÄT:4-7
	// Den ersten Channel suchen!
	for (chan=CHANNEL_OUT_OFFSET;chan< l2->slvs_cfg; chan++)	
		if(_bitset(mask,chan))	{
			chan_ = chan;
			break;
		}
	// ABBRUCH, da kein Channel in mask gekennzeichnet ist!
	if( chan_ == 0xff) {
		MBUS_CMD_TRACE("ERROR!\n");
		return 0;
	}
	OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
	
    ///SPANNUNG
	// ACTION:  (Kurzbefehl write/replace)
	volt_write = 0;
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_volt,	\
				DSCR_AO_VOLT_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_VOLT_LEN;
		volt_write = 1;
	}
	// ACTION:  (Langbefehl write/replace) 
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_volt_write,	\
				DSCR_AO_ACTION_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_ACTION_LEN;
		volt_write = 1;
	}
	// ACTION: WRITE/REPLACE
	if(	volt_write) {
		memcpy(&value16,&l1->rec.buf[idx],sizeof(uint16_t));
		idx=idx+2;
		OBJS_ODEV_WR_OUTPUT_xBit(odev,value16); /// Intern auf 10 bit !
		MBUS_CMD_TRACE("AO: OUTPUT %d (write/replace) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);	
		if(chan == MBUS_SLV_OUT0) {
			chan = chan+1;
			if(_bitset(mask,chan))	{		// nachfolgendes Gerät auch SPANNUNGS-Ausgang?
				OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
				OBJS_ODEV_WR_OUTPUT_xBit(odev,value16);  // Werte: (==)
				MBUS_CMD_TRACE("AO: OUTPUT %d (write/replace) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);
			}
		}
		l1->rec.stack = idx;
		return 1;
	}
	// ACTION:  CLEAR
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_volt_clear,	\
				DSCR_AO_ACTION_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_ACTION_LEN;
		OBJS_ODEV_WR_OUTPUT_xBit(odev,value16);
		MBUS_CMD_TRACE("AO: OUTPUT %d (clear) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);	
		if(chan == MBUS_SLV_OUT0) {
			chan = chan+1;
			if(_bitset(mask,chan))	{		// nachfolgendes Gerät auch SPANNUNGS-Ausgang?
				OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
				OBJS_ODEV_WR_OUTPUT_xBit(odev,value16); // Werte  (== 0)
				MBUS_CMD_TRACE("AO: OUTPUT %d (clear) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);
			}
		}
		l1->rec.stack = idx;
		return 1;
	}

    ///STROM
	// ACTION:  (Kurzbefehl write/replace)
	ampere_write = 0;
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_ampere,	\
				DSCR_AO_AMPERE_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_AMPERE_LEN;
		ampere_write = 1;
	}
	// ACTION:  (Langbefehl write/replace) 
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_ampere_write,	\
				DSCR_AO_ACTION_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_ACTION_LEN;
		ampere_write = 1;
	}
	// ACTION: WRITE/REPLACE
	if(	ampere_write) {
		memcpy(&value16,&l1->rec.buf[idx],sizeof(uint16_t));
		idx=idx+2;
		OBJS_ODEV_WR_OUTPUT_xBit(odev,value16); /// Intern auf 11 bit !
		MBUS_CMD_TRACE("AO: OUTPUT %d (write/replace) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);	
		if(chan == MBUS_SLV_OUT2) {
			chan = chan+1;		
			if(_bitset(mask,chan))	{ 		// nachfolgendes Gerät auch STROM-Ausgang?
				OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
				OBJS_ODEV_WR_OUTPUT_xBit(odev,value16);  // Werte: (==)
				MBUS_CMD_TRACE("AO: OUTPUT %d (write/replace) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);
			}
		}
		l1->rec.stack = idx;
		return 1;
	}
	// ACTION:  CLEAR
	if( memcmp(&l1->rec.buf[idx],	\
				dscr_AO_ampere_clear,	\
				DSCR_AO_ACTION_LEN ) == 0)
	{	
		idx = idx + DSCR_AO_ACTION_LEN;
		OBJS_ODEV_WR_OUTPUT_xBit(odev,value16);
		MBUS_CMD_TRACE("AO: OUTPUT %d (clear) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);	
		if(chan == MBUS_SLV_OUT2) {
			chan = chan+1;		
			if(_bitset(mask,chan))	{ 		// nachfolgendes Gerät auch STROM-Ausgang?
				OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
				OBJS_ODEV_WR_OUTPUT_xBit(odev,value16); // Werte  (== 0)
				MBUS_CMD_TRACE("AO: OUTPUT %d (clear) New: %u\n",chan-CHANNEL_OUT_OFFSET,value16);
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

// FRIMWARE VERSION PIC - Register (Type variabel 6Byte, Storage0)
unsigned char _cmd_rsp_ud_firmware(	struct m_l2 * l2,\
							struct m_l1 * l1,\
							unsigned char chan,\
							unsigned char idx,\
							char *bcc)
{
	OBJS_CREATE_DRV_PTR(l2->pdata);
	unsigned char firmware;
	unsigned char bcd_low;
	unsigned char bcd_high;
	MBUS_CMD_TRACE("OVERLOAD: _cmd_rsp_ud_firmware() channel:[%d]\n",chan);	
	if(l2->sel_without[chan] & SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_FIRMWAREVERSION)
	{
		memcpy(&l1->snd.buf[idx],&dscr_firmware_version, DSCR_FIRMWARE_VERSION_LEN);
		bcc 	+= (unsigned char) (DSCR_FIRMWARE_VERSION_BCC);
		idx = idx + DSCR_FIRMWARE_VERSION_LEN;
		OBJS_DRV_RD_PIC_VERSION(drv,firmware);
		bcd_low = (firmware & 0x0F) + 0x30;
		bcd_high = firmware & 0xF0;
		bcd_high = (bcd_high >> 4) + 0x30;
		// -------------- FORMAT --------
		MBUS_CMD_TRACE("Hig: %c\n",	bcd_high);
		MBUS_CMD_TRACE("Low: %c\n",	bcd_low);
		// 0x06,'0', bcd_low, '.', bcd_high, ' ', 'V'
		l1->snd.buf[idx] = 6;	
		bcc+= l1->snd.buf[idx];
		idx++;
		l1->snd.buf[idx] = '0';	
		bcc+= l1->snd.buf[idx];
		idx++;
		l1->snd.buf[idx] = bcd_low;
		bcc+= l1->snd.buf[idx];
		idx++;
		l1->snd.buf[idx] = '.';
		bcc+= l1->snd.buf[idx];
		idx++;
		l1->snd.buf[idx] = bcd_high;
		bcc+= l1->snd.buf[idx];
		idx++;
		l1->snd.buf[idx] = ' ';
		bcc+= l1->snd.buf[idx];
		idx++;
		l1->snd.buf[idx] = 'V';
		bcc+= l1->snd.buf[idx];
		idx++;
		// einmaliges auslesen beendet: 
		l2->sel_without[chan] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_FIRMWAREVERSION);
	}
	return idx;		// idx aktualisiert!
}

int mbus_cmd_rsp_ud_data2(struct m_app * la) {
	CREATE_L2_PTR;
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR_NULL;
	OBJS_CREATE_ODEV_PTR_NULL;
	unsigned char 		idx 	= 0;
	char				bcc 	= 0; 
	unsigned char 		chan 	= l2->slvs_idx; // aktiver Kanal
	uint32_t 			value32 	= 0;
	uint16_t			value16		= 0;
	unsigned char		value8		= 0;
	unsigned char		kennlinie	= 0;
	unsigned char 		buf[50];
#ifdef CONFIG_MBUS_CMD_TRACE
	unsigned int 		i;
#endif

	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",chan);
	bcc = m_cl2_header_build(l2, chan, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!
	
	switch(chan) {
		// GERÄT:0,1  (Spannung, Widerstand)
		case MBUS_SLV_IN0:
		case MBUS_SLV_IN1:
			OBJS_CREATE_IDEV_PTR(chan-CHANNEL_IN_OFFSET);
			value8 = box_sprintf_sel(&(idev->Op_Type->P_Type),buf);
			if(strstr(buf,AI_PROP_MODE_PATTERN_0)) {
				// "Ohm"
				memcpy(&l1->snd.buf[idx],dscr_AI_ohm,DSCR_AI_OHM_LEN);
				bcc+=	(unsigned char) (DSCR_AI_OHM_BCC);
				idx = idx + DSCR_AI_OHM_LEN;
			}
			else if (strstr(buf,AI_PROP_MODE_PATTERN_1)) {
				// "Temp"
				memcpy(&l1->snd.buf[idx],dscr_AI_temp,DSCR_AI_TEMP_LEN);
				bcc+=	(unsigned char) (DSCR_AI_TEMP_BCC);
				idx = idx + DSCR_AI_TEMP_LEN;
			}
			else if (strstr(buf,AI_PROP_MODE_PATTERN_2)){
 				// "Volt"
				memcpy(&l1->snd.buf[idx],dscr_AI_volt,DSCR_AI_VOLT_LEN);
				bcc+=	(unsigned char) (DSCR_AI_VOLT_BCC);
				idx = idx + DSCR_AI_VOLT_LEN;
			}
			else {
				// unbekannt - z.B inactive
				MBUS_CMD_TRACE("Inactive!\n");
				memcpy(&l1->snd.buf[idx],dscr_AI_undef,DSCR_AI_UNDEF_LEN);
				bcc+=	(unsigned char) (DSCR_AI_UNDEF_BCC);
				idx = idx + DSCR_AI_UNDEF_LEN;
			}
			goto NEXT_IN;
		
		// GERÄT:2,3  (Strom)
		case MBUS_SLV_IN2:
		case MBUS_SLV_IN3:
			OBJS_CREATE_IDEV_PTR(chan-CHANNEL_IN_OFFSET);
			value8 = box_sprintf_sel(&(idev->Op_Type->P_Type),buf);
			if(strstr(buf,AI_PROP_MODE_PATTERN_3)) {
				// "Ampere"
				memcpy(&l1->snd.buf[idx],dscr_AI_ampere,DSCR_AI_AMPERE_LEN);
				bcc+=	(unsigned char) (DSCR_AI_AMPERE_BCC);
				idx = idx + DSCR_AI_AMPERE_LEN;
			}
			else {
				// unbekannt - z.B inactive
				MBUS_CMD_TRACE("Inactive!\n");
				memcpy(&l1->snd.buf[idx],dscr_AI_undef,DSCR_AI_UNDEF_LEN);
				bcc+=	(unsigned char) (DSCR_AI_UNDEF_BCC);
				idx = idx + DSCR_AI_UNDEF_LEN;
			}
NEXT_IN:
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// DATA_32 (INPUT) - Register (Type 32Bit,Storage0)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			OBJS_IDEV_RD_INPUT(idev,value32);
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

			if(chan == MBUS_SLV_IN0 || chan == MBUS_SLV_IN1) {
				value8 = strlen(ai_12_label_tab[kennlinie]);
				memcpy(buf,ai_12_label_tab[kennlinie],value8);
			}
			else {
				value8 = strlen(ai_34_label_tab[kennlinie]);
				memcpy(buf,ai_34_label_tab[kennlinie],value8);
			}

			l1->snd.buf[idx] = value8;	// Länge speichern
			bcc+= l1->snd.buf[idx];
			idx++;			
			while(value8--) {
				l1->snd.buf[idx] = buf[value8];	// rückwärts lesen!
				bcc+= l1->snd.buf[idx];
				idx++;
			}
			break;

			// GERÄT:4,5 (Spannungs-Ausgänge)
		case MBUS_SLV_OUT0:
		case MBUS_SLV_OUT1:
			OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// DATA_16 (OUTPUT) - Register (Type 16Bit,Storage0)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			memcpy(&l1->snd.buf[idx],dscr_AO_volt,DSCR_AO_VOLT_LEN);
			bcc+=	(unsigned char) (DSCR_AO_VOLT_BCC);
			idx = idx + DSCR_AO_VOLT_LEN;		
			OBJS_ODEV_RD_OUTPUT(odev,value16);
			memcpy(&l1->snd.buf[idx],&value16,sizeof(uint16_t));
			bcc+= l1->snd.buf[idx];
			idx++;
			bcc+= l1->snd.buf[idx];
			idx++;
			break;

			// GERÄT:6/7 (Strom-Ausgänge)
		case MBUS_SLV_OUT2:
		case MBUS_SLV_OUT3:
			OBJS_CREATE_ODEV_PTR(chan-CHANNEL_OUT_OFFSET);
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// DATA_16 (OUTPUT) - Register (Type 16Bit,Storage0)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			memcpy(&l1->snd.buf[idx],dscr_AO_ampere,DSCR_AO_AMPERE_LEN);
			bcc+=	(unsigned char) (DSCR_AO_AMPERE_BCC);
			idx = idx + DSCR_AO_AMPERE_LEN;		
			OBJS_ODEV_RD_OUTPUT(odev,value16);
			memcpy(&l1->snd.buf[idx],&value16,sizeof(uint16_t));
			bcc+= l1->snd.buf[idx];
			idx++;
			bcc+= l1->snd.buf[idx];
			idx++;
			break;

		default:
			MBUS_CMD_TRACE("SLAVE-TYPE ???\n");
			break;
	}
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// SOFTWARE (Treiber)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	idx = _cmd_rsp_ud_software(l2,l1,chan,idx,&bcc);
	
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// FIRMWARE (PIC)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	idx = _cmd_rsp_ud_firmware(l2,l1,chan,idx,&bcc);
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
