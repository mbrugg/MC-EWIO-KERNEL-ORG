/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$
 	
 	Description:  MBUS CIENT LAYER1
	 Layer 1 - Trace 	
 *******************************************************************************/
#include "mbus_def.h"		// basics
#include "mbus_cl1.h"		// client layer1

// Rückgabewert 0 = nix zu tuen
size_t m_cl1_record(char *tlg, size_t count, struct tl1_rec * rec ) {
	int	ret  = 0;
	char 	sign = 0;
	
	// Länge überprüfen	
	if(count > MBUS_TLG_MAX_LEN) {
	 #ifdef MBUS_CL1_TRACE
		printk("Error: M_REC_BUFFER[%d] Limit!\n",MBUS_TLG_MAX_LEN);
	#endif
		memset(rec,0,sizeof(struct tl1_rec));
		goto FINAL;
	}
	// Initialisiere Empfänger

	while(count)	
	{
		count--;
		sign = *tlg;
		tlg++;
		switch(rec->state) 					// Status-Maschine	
		{	// eine Startkennung erwartet:
			case M_REC_IDLE:
//				printk("M_REC_IDLE\n");
				switch(sign)	
				{
					case FT_1_2_FIX:		// Short-Frame
							memset(rec,0,sizeof(struct tl1_rec));
							rec->attr = M_ATTR_REC_FRA_S;
							rec->state = M_REC_C_FLD;
							break;
					case FT_1_2_VAR:		// Control-/Long-Frame, es folgt der Header
							memset(rec,0,sizeof(struct tl1_rec));
							rec->state = M_REC_L_FLD1;
							break;
					default:
							break;			/// Slip-Mode
				}
				break;
	
			case M_REC_L_FLD1:				// Control-/Long-Frame
//				printk("M_REC_L_FLD1\n");
				rec->fld_l = sign;
				if( sign == 0x03)			// Control-Frame
					rec->attr = M_ATTR_REC_FRA_C;
				else 						// Long-Frame
					rec->attr = M_ATTR_REC_FRA_L;

				if(sign < 3) {
				#ifdef MBUS_CL1_TRACE
					printk("Error: M_REC_L_FLD1 < 3\n");
				#endif
					memset(rec,0,sizeof(struct tl1_rec));
					goto FINAL;
				}	
				else {
					rec->state = M_REC_L_FLD2;
				}
				break;

			case M_REC_L_FLD2:				// Control-/Long-Frame
//				printk("M_REC_L_FLD2\n");
				if(rec->fld_l != sign) {	// Längenunterschied!
				#ifdef MBUS_CL1_TRACE
					printk("Error: M_REC_L_FLD1 <> M_REC_L_FLD2\n"); 
				#endif
					memset(rec,0,sizeof(struct tl1_rec));
					goto FINAL;	
				}
				rec->state = M_REC_START_CL;
				break;	

			case M_REC_START_CL:
//				printk("M_REC_START_CL\n");
				if(sign != FT_1_2_VAR)	{
				#ifdef MBUS_CL1_TRACE
					printk("Error: M_REC_START2\n");
				#endif 
					memset(rec,0,sizeof(struct tl1_rec));
					goto FINAL;
				 }
				 rec->state	= M_REC_C_FLD;
				break;

			case M_REC_C_FLD:				// SHORT-/Control-/Long-Frame	
//				printk("M_REC_C_FLD\n");
				rec->bcc +=sign;	
				switch(rec->attr) {
					case M_ATTR_REC_FRA_S:
						switch(sign)		{
							case C_SND_NKE:
							case C_REQ_UD2:
							case C_REQ_UD2_F:
							case C_REQ_UD2_V:
							case C_REQ_UD2_VF:
							case C_REQ_UD1:
							case C_REQ_UD1_F:
							case C_REQ_UD1_V:
							case C_REQ_UD1_VF:
								rec->fld_c = sign;	// C-Feld-Merker
								rec->state = M_REC_A_FLD;
								break;
							default:	
							#ifdef MBUS_CL1_TRACE
								printk("Error: Short-Telegramm M_REC_C_FLD[%d]\n",sign);
							#endif
								memset(rec,0,sizeof(struct tl1_rec));
								goto FINAL;
						}
						break;
					case M_ATTR_REC_FRA_C:
					case M_ATTR_REC_FRA_L:
							rec->len  = 1;		// Längenzähler starten, C-Feld zählt mit
							switch(sign)		{
								case C_SND_UD:
								case C_SND_UD_F:
								case C_SND_UD_V:
								case C_SND_UD_VF:
									rec->fld_c = sign;	// C-Feld-Merker
									rec->state = M_REC_A_FLD;
								break;
								default:
								#ifdef MBUS_CL1_TRACE
									printk("Error: Control/Long-Telegramm M_REC_C_FLD[%d]\n",sign);
								#endif
									memset(rec,0,sizeof(struct tl1_rec));
									goto FINAL;
						}
						break;
					default:	
					#ifdef MBUS_CL1_TRACE
						printk("Error: M_REC_C_FLD[%d] Telegrammtype is unknown!\n",sign);
					#endif
					memset(rec,0,sizeof(struct tl1_rec));
					goto FINAL;
				}
				break;

			case M_REC_A_FLD:
//				printk("M_REC_A_FLD\n");
				rec->bcc +=sign;
				rec->fld_a = sign;					// A-Feld-Merker
				if(rec->attr == M_ATTR_REC_FRA_S) {// Short-Frame, dann bcc
					rec->state = M_REC_BCC;
				}
				else	{
					rec->len++;						// Längenzähler, A-Feld zählt mit
					rec->state = M_REC_CI_FLD;		// Control-/Long-Frame dann ci-Feld
				}
				break;
	
			case M_REC_CI_FLD:
//				printk("M_REC_CI_FLD\n");
				rec->bcc +=sign;
				rec->fld_ci = sign;					// CI-Feld-Merker
				if(rec->attr == M_ATTR_REC_FRA_C)	{// Control-Frame, dann bcc
					rec->state = M_REC_BCC;
				}
				else	{
					rec->state = M_REC_DATA;		// Long-Frame, dann data
					rec->len++;						// Längenzähler, CI-Feld zählt mit
					rec->cnt = 0;					// Datenzähler starten!
				}
				break;

			case M_REC_DATA:
//				printk("M_REC_DATA [0x%02x]\n",sign);
				rec->bcc +=sign;
				rec->buf[rec->cnt] = sign;			// Daten in den Empfangspuffer!
				rec->len++;
				rec->cnt++;
				if( rec->len == rec->fld_l)			// Länge erreicht!	
				   rec->state = M_REC_BCC;
				break;	

			case M_REC_BCC:
//				printk("M_REC_BCC-\n");
				if(rec->bcc != sign)	{
				#ifdef MBUS_CL1_TRACE
					printk("Error: M_REC_BCC rec->bcc:0x%02x <> sign:0x%02x\n",rec->bcc,sign);
				#endif
					memset(rec,0,sizeof(struct tl1_rec));
					goto FINAL;	
				}
				rec->state	= M_REC_STOP;
				break;	 	

				// auf Stoppzeichen prüfen	
			case M_REC_STOP:
//				printk("M_REC_STOP\n");
				if(sign != FT_1_2_STP)	{
				#ifdef MBUS_CL1_TRACE
					printk("Error: M_REC_STOP sign:0x%02x\n",sign);
				#endif
					memset(rec,0,sizeof(struct tl1_rec));
					goto FINAL;
				}
			#ifdef MBUS_CL1_TRACE
				printk("LAYER 1 passed!\n");
				printk("C-FLD: 0x%02x A-FLD %d CI-FLD 0x%02x\n",rec->fld_c,rec->fld_a,rec->fld_ci);
			#endif
				ret = 1;
				rec->state = M_REC_IDLE;	//und warten ... auf neue Telegramme
				goto FINAL;
				break;	
			
			default:	// kann eigentlich nichtvorkommen!

			#ifdef MBUS_CL1_TRACE
				printk("Error: M_REC_UNKNOWN\n");
			#endif
				memset(rec,0,sizeof(struct tl1_rec));
				goto FINAL;
				break;
		}	// ENDE: EMPANGSSTATUSMACHINE
	}
FINAL:	
	return(ret);
};
