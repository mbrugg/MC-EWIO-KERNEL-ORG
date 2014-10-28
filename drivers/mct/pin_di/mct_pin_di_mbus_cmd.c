/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	25.08.2011
 	
 	Description:  MBUS Command-Implementation for mct_pin_di-Driver
	
	Schalter-Einstellung: 
				CONFIG_MBUS_CMD_TRACE ---> siehe: mct_pin_di_mbus_cfg.h

 *******************************************************************************/
#include "mct_pin_di_objs.h"		// objects

#include "mct_pin_di_mbus_cfg.h"	// MBUS Konfiguration
#include "mct_pin_di_mbus_S0.h"		// MBUS als Counter S0
#include "mct_pin_di_mbus_DI.h"		// MBUS als digitaler Eingang mit Entprellung

#include "../if/mbus_cl2.h"
#include "../if/mbus_app.h"

// Trace-Makro
#ifdef CONFIG_MBUS_CMD_TRACE
	#define MBUS_CMD_TRACE(args...) printk(args);
#else 
	#define MBUS_CMD_TRACE(args...);
#endif

#define CREATE_L2_PTR		struct m_l2  * 	l2  	= &(la->l2)
#define CREATE_L1_PTR 		struct m_l1  *  l1  	= &(l2->l1)

#define STORAGE_START_POS 	0

// only Counter
static u32 	stamp_32;
//static unsigned char  	calimode_8 = 0;

// ------------------------------------------------------------------
// SOFTWARE-DESCRIPTOR  - TREIBER
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
/// APPLIKATION_RESET (OVERLOADED) Single-Modul
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_application_reset(struct m_l2 * l2, unsigned char mask) {
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR;
	unsigned int mode;		// Input,S0-Counter
	bool		 init;		// Initialisierungsflag
	u64 		 lokal64;

	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_application_reset() mask:[0x%02x]\n",mask);
	MBUS_CMD_TRACE("RESET-Subcode:0x%02x\n",l1->rec.buf[l1->rec.stack]);	
	
	/// MODUL-MODE ermitteln
	OBJS_IDEV_RD_MODE(idev,mode);

	if(_bitset(mask,0))	{
		// Subcode Handler
		switch(l1->rec.buf[l1->rec.stack])	{
			case APPLICATION_RESET_USER_DATA:
				l2->slvs[0].access_cnt = 1;
				l2->sel_without[0] &= (~SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION);
//				printk("ToDo - mode unterscheiden notwendig??? - pruefen\n");
				MBUS_CMD_TRACE("APPLICATION_RESET_USER_DATA:\n");
				OBJS_IDEV_WR_VALUE(idev,0)	// input-Wert löschen
				OBJS_IDEV_WR_FREEZE(idev,0)	// freeze-Wert löschen
				stamp_32 = 0;				// Zeistempel löschen
				OBJS_IDEV_WR_INIT(idev,0);	// Kalibration beendet
				break;

			case APPLICATION_RESET_INSTALLATION_AND_STARTUP:
				switch(mode) 	{
					case MODE_S0:				/// fällt durch!
						MBUS_CMD_TRACE("APPLICATION_RESET_INSTALLATION_AND_STARTUP:\n");
						OBJS_IDEV_WR_VALUE(idev,0)	// input-Wert löschen
						OBJS_IDEV_WR_FREEZE(idev,0)	// freeze-Wert löschen
						stamp_32 = 0;				// Zeistempel löschen
						OBJS_IDEV_WR_INIT(idev,1);	// Kalibration aktiviert!
						break;
				}
				break;

			case APPLICATION_RESET_CALIBRATION:
				switch(mode)	{
					case MODE_S0:
						MBUS_CMD_TRACE("APPLICATION_RESET_CALIBRATION:\n");
						OBJS_IDEV_RD_INIT(idev,init);	// Initialisierungsflag lesen
						 if(init)	{
							OBJS_IDEV_RD_FREEZE(idev,lokal64);
							OBJS_IDEV_WR_VALUE(idev,lokal64); 
							OBJS_IDEV_WR_INIT(idev,0); 	// Initialisierung beendet
						}
						break;
				}
				break;

			default: 
				MBUS_CMD_TRACE("RESET-Subcode:0x%02x Unsupported!\n",l1->rec.buf[l1->rec.stack]);
				break;
		}
	}
	return 1;
}

/// ------------------------------------------------------------------
/// GLOBAL_READOUT (OVERLOADED) Single-Modul
/// ------------------------------------------------------------------
int mbus_cmd_snd_ud_global_readout(struct m_l2 * l2, unsigned char mask) {
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_snd_ud_global_readout() mask:[0x%02x]\n",mask);	
	if(_bitset(mask,0))	{
		l2->sel_without[0] |= SEL_WITHOUT_SPECIFIED_DATAFIELD_APP_SOFTWAREVERSION;
	}
	return 1;
}

/// ------------------------------------------------------------------
/// SND_UD_USR_DATA (OVERLOADED) Single-Modul 
/// ------------------------------------------------------------------
#define TO_64(v64,v32) {	\
		memcpy(&(v32),&l1->rec.buf[idx+4],sizeof(uint32_t));	\
		(v64) = (v32);	\
		(v64) = (v64) << 32;	\
		memcpy(&(v32),&l1->rec.buf[idx],sizeof(uint32_t));	\
		(v64)	= (v64) | (v32);	\
		idx=idx+8;}

int mbus_cmd_snd_ud_usr_data(struct m_l2 * l2, unsigned char mask)	{
	CREATE_L1_PTR;
	OBJS_CREATE_DRV_PTR(l2->pdata);
	OBJS_CREATE_IDEV_PTR; 
	unsigned char 		idx 		= l1->rec.stack;
	unsigned long long 	lokal64 	= 0;	
	unsigned long long 	lokal64_0 	= 0;
	uint32_t  			lokal32 	= 0;
	unsigned int		mode 		= 0;		// Input,Counter
	bool				flg_write;
	bool		 		init;	// Initialisierungsflag

//	printk("OVERLOAD: mbus_cmd_rsp_ud_data2() mask:[0x%02x]\n",mask);
/*	printk("OVERLOAD: mbus_cmd_snd_ud_usr_data()0x%02x 0x%02x 0x%02x 0x%02x mask:[0x%02x]\n",\
			l1->rec.buf[idx],\
			l1->rec.buf[idx+1],\
			l1->rec.buf[idx+2],\
			l1->rec.buf[idx+3],mask);
*/
	// ABBRUCH, da der Channel in mask nicht gekennzeichnet ist!
	if(_bitclr(mask,0))	{
		MBUS_CMD_TRACE("ERROR!\n");
		return 0;
	}
	/// MODUL-MODE feststellen, dev belegen ...
	OBJS_IDEV_RD_MODE(idev,mode);
	switch(mode) 
	{
		default:
			MBUS_CMD_TRACE("TODO: MODE ???\n");
			break;

		case MODE_DIRECT:
			break;

		case MODE_S0:
			///CUMULATION_COUNTER-Register (Type 64Bit,Storage0)
			// ACTION:  (Kurzbefehl write/replace)
			flg_write = 0;
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_imp_standard,	\
						DSCR_S0_IMP_STANDARD_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_IMP_STANDARD_LEN;
				flg_write = 1;
			}
			// ACTION:  (Langbefehl write/replace) 
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_imp_write,	\
						DSCR_S0_IMP_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_IMP_ACTION_LEN;
				flg_write = 1;			
			}
			// ACTION: WRITE/REPLACE
			if(	flg_write) {
				TO_64(lokal64,lokal32);
				OBJS_IDEV_WR_VALUE(idev,lokal64);
				MBUS_CMD_TRACE("SO: CUMULATION_COUNTER (write/replace) New: %llu\n",lokal64);
				l1->rec.stack = idx;
				return 1;
			}
			// ACTION:  ADD 
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_imp_add,	\
						DSCR_S0_IMP_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_IMP_ACTION_LEN;				
				TO_64(lokal64,lokal32);
				OBJS_IDEV_RD_VALUE(idev,lokal64_0);
				lokal64 = lokal64 + lokal64_0; 
				OBJS_IDEV_WR_VALUE(idev,lokal64);
				MBUS_CMD_TRACE("SO: CUMULATION_COUNTER (add) New: %llu\n",lokal64);
				l1->rec.stack = idx;
				return 1;
			}
			// ACTION:  CLEAR
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_imp_clear,	\
						DSCR_S0_IMP_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_IMP_ACTION_LEN;
				OBJS_IDEV_WR_VALUE(idev,0);
				MBUS_CMD_TRACE("SO: CUMULATION_COUNTER (clear) New: 0\n");
				l1->rec.stack = idx;
				return 1;
			}

			///STATE_PARAMETER_ACTIVATION-Register (Type 8Bit, Storage1)
			// ACTION:  CLEAR	
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_sta_clear,	\
						DSCR_S0_STA_ACTION_LEN) == 0)
			{	
				idx = idx + DSCR_S0_STA_ACTION_LEN;
				MBUS_CMD_TRACE("SO: STATE_PARAM_ACTIVATION (clear)\n");
				
				OBJS_IDEV_RD_INIT(idev,init);			// Initialisierungsflag lesen
				 if(init)	{
					OBJS_IDEV_WR_INIT_EXPORT(idev,1);	// InitExport aktualisieren!
					OBJS_IDEV_WR_INIT(idev,0) 			// Initialisierung beendet!
				}
				l1->rec.stack = idx;
				return 1;
			}

			///TIMEPOINT-Register (TypeF, Storage1)
			// ACTION  WRITE/REPLACE	
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_timepoint_write ,	\
						DSCR_S0_TIMEPOINT_ACTION_LEN) == 0)
			{
				idx = idx + DSCR_S0_TIMEPOINT_ACTION_LEN;
				// Datum und Zeit kopieren	(4 Byte)
				memcpy(&lokal32,&l1->rec.buf[idx],sizeof(uint32_t));
				idx=idx+4;
				stamp_32 = lokal32;
				MBUS_CMD_TRACE("SO: TIMEPOINT (write/replace)\n");
				l1->rec.stack = idx;
				return 1;
			}

			///FREEZE-Register (Type 64Bit,Storage1)
			// ACTION:  (Kurzbefehl write/replace)
			flg_write = 0;
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_frz_standard,	\
						DSCR_S0_FRZ_STANDARD_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_FRZ_STANDARD_LEN;
				flg_write = 1;
			}
			// ACTION:  (Langbefehl write/replace) 
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_frz_write,	\
						DSCR_S0_FRZ_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_FRZ_ACTION_LEN;
				flg_write = 1;			
			}
			// ACTION: WRITE/REPLACE
			if(	flg_write) {
				TO_64(lokal64,lokal32);
				OBJS_IDEV_WR_FREEZE(idev,lokal64);
				MBUS_CMD_TRACE("SO: FREEZE_COUNTER (write/replace) New: %llu\n",lokal64);
				l1->rec.stack = idx;
				return 1;
			}
			// ACTION:  ADD 
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_frz_add,	\
						DSCR_S0_FRZ_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_FRZ_ACTION_LEN;
				TO_64(lokal64_0,lokal32);
				OBJS_IDEV_RD_FREEZE(idev,lokal64);
				lokal64 =  lokal64 + lokal64_0;
				OBJS_IDEV_WR_FREEZE(idev,lokal64);
				MBUS_CMD_TRACE("SO: FREEZE_COUNTER (add) New: %llu\n",lokal64);
				l1->rec.stack = idx;
				return 1;
			}
			// ACTION:  FREEZE	
			// Das Komando "Freeze" wird nur für die Kanäle ausgeführt, wo "initflag"
			// inaktiv ist, d.h = 0. Wenn "initflag" = aktiv ist, d.h 1, wird das 
			// Komando "Freeze" nicht ausgeführt, da der Freeze-Buffer bereits mit einem
			// INITIALWERT (z.B. durch Tastendruck) belegt ist. 
			// Der Anwender muß vorher kontrollieren, ob STATE_PARAMETER_ACTIVATION
			// mit 0 oder mit 1 belegt ist.
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_frz_freeze,	\
						DSCR_S0_FRZ_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_FRZ_ACTION_LEN;
				OBJS_IDEV_RD_INIT(idev,init);	// Initialisierungsflag lesen
				if(!init)	{
					OBJS_IDEV_RD_VALUE(idev,lokal64);
					OBJS_IDEV_WR_FREEZE(idev,lokal64);
					MBUS_CMD_TRACE("SO: FREEZE_COUNTER (freeze) New: %llu\n",lokal64);
				}
				l1->rec.stack = idx;
				return 1;
			}
			// ACTION:  CLEAR
			if( memcmp(&l1->rec.buf[idx],	\
						dscr_S0_frz_clear,	\
						DSCR_S0_FRZ_ACTION_LEN ) == 0)
			{	
				idx = idx + DSCR_S0_FRZ_ACTION_LEN;
				OBJS_IDEV_WR_FREEZE(idev,0);
				MBUS_CMD_TRACE("SO: FREEZE_COUNTER (clear) New: 0\n");
				l1->rec.stack = idx;
				return 1;
			}
			break;	// MODE_COUNTER
	}	// end switch mode
	return 0;
}

/// ------------------------------------------------------------------
/// RSP_UD_DATA2 (OVERLOADED) Single-Modul, weil Aufruf mit Parameter chan=0
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
	unsigned char		lokal8	= 0;
	uint32_t 			lokal32 = 0;
	unsigned long long 	lokal64 = 0;
	unsigned int		mode = 0;
	unsigned int		i = 0;
	bool		 init;		// Initialisierungsflag
	
	MBUS_CMD_TRACE("OVERLOAD: mbus_cmd_rsp_ud_data2() channel:[%d]\n",0);

	bcc = m_cl2_header_build(l2, 0, C_RSP_UD);
	idx = l1->snd.cnt;	// da wir mit dem Index arbeiten wollen!

	/// MODUL-MODE feststellen, dev belegen ...
	OBJS_IDEV_RD_MODE(idev,mode);

	switch(mode) 
	{
		 default:
			MBUS_CMD_TRACE("MODUL-MODE ???\n");
			break;

		case MODE_DIRECT:
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// DATA_8 - Register (Type 8Bit,Storage0)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			memcpy(&l1->snd.buf[idx],dscr_DI_imp_standard,DSCR_DI_IMP_STANDARD_LEN);
			bcc+=	(unsigned char) (DSCR_DI_IMP_BCC);
			idx = idx + DSCR_DI_IMP_STANDARD_LEN;		
			OBJS_IDEV_RD_VALUE(idev,lokal8);
			l1->snd.buf[idx] = lokal8;
			bcc+= l1->snd.buf[idx];
			idx++;
			break;
		
		case MODE_S0:	
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// COUNTER - Register (Type 64Bit, Storage0)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			memcpy(&l1->snd.buf[idx],dscr_S0_imp_standard,DSCR_S0_IMP_STANDARD_LEN);
			bcc+=	(unsigned char) (DSCR_S0_IMP_BCC);
			idx = idx + DSCR_S0_IMP_STANDARD_LEN;		
			// HACK Test
			// Anordnung im Sendepuffer: ...0x88 0x77 0x66 0x55 0x44 0x33 0x22 0x11
			// SET_DEV_VALUE(0x8877665544332211ULL);	
			OBJS_IDEV_RD_VALUE(idev,lokal64);
			//  32-bit-Counter-Low
			lokal32 = (uint32_t)(lokal64);
			memcpy(&l1->snd.buf[idx],&lokal32,sizeof(uint32_t));
			idx=idx+4;
			// 32-bit-Counter-High
			lokal32  = (uint32_t)(lokal64 >> 32);
			memcpy(&l1->snd.buf[idx],&lokal32,sizeof(uint32_t));
			idx=idx+4;			
			//bcc noch berechnen ...
			for (i = 0; i < sizeof(u64); i++) {
				bcc+= l1->snd.buf[idx+i-sizeof(u64)];
			}
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// TIMEPOINT - Register (TypeF, Storage1)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%	
			memcpy(&l1->snd.buf[idx],dscr_S0_timepoint_standard,DSCR_S0_TIMEPOINT_STANDARD_LEN);
			bcc+=	DSCR_S0_TIMEPOINT_BCC;
			idx = 	idx + DSCR_S0_TIMEPOINT_STANDARD_LEN;
			// HACK Test
			// stamp_32 = 0x44332211;
			//  32-bit-TimeStamp
			lokal32 = stamp_32;
			memcpy(&l1->snd.buf[idx],&lokal32,sizeof(uint32_t));
			idx=idx+4;
			//bcc noch berechnen ...
			for (i = 0; i < sizeof(uint32_t); i++) {
				bcc+= l1->snd.buf[idx+i-sizeof(uint32_t)];
			}

			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// FREEZE - Register (Type 64Bit,Storage1)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			memcpy(&l1->snd.buf[idx],dscr_S0_frz_standard,DSCR_S0_FRZ_STANDARD_LEN);
			bcc+=	(unsigned char)(DSCR_S0_FRZ_BCC);
			idx = idx + DSCR_S0_FRZ_STANDARD_LEN;
			// HACK
			// lokal64 = 0x8070605040302010ULL;
			OBJS_IDEV_RD_FREEZE(idev,lokal64);			
			// 32-bit-Freeze-Low
			lokal32 = (uint32_t)(lokal64);
			memcpy(&l1->snd.buf[idx],&lokal32,sizeof(uint32_t));
			idx=idx+4;
			// 32-bit-Freeze-High
			lokal32  = (uint32_t) (lokal64 >> 32);
			memcpy(&l1->snd.buf[idx],&lokal32,sizeof(uint32_t));
			idx=idx+4;
			//bcc noch berechnen ...
			for (i = 0; i < sizeof(u64); i++) {
				bcc+= l1->snd.buf[idx+i-sizeof(u64)];
			}
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			// STATE_PARAMETER ACTIVATION - Register (Type 8Bit, Storage1)
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			memcpy(&l1->snd.buf[idx],dscr_S0_sta_standard,DSCR_S0_STA_STANDARD_LEN);
			bcc+=	(unsigned char) (DSCR_S0_STA_BCC);
			idx = 	idx + DSCR_S0_STA_STANDARD_LEN;

			OBJS_IDEV_RD_INIT(idev,init);	// Initialisierungsflag lesen
			if(init)
				l1->snd.buf[idx]  = 1;		// PARAMETER ACTIVATION Active
			else
				l1->snd.buf[idx]  = 0;		// PARAMETER ACTIVATION Inactive

			bcc+=	l1->snd.buf[idx];
			idx++;

		break;	// MODE_COUNTER
	}	// end switch mode	

	///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	/// SOFTWARE
	///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	idx = _cmd_rsp_ud_software(l2,l1,0,idx,&bcc);

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
