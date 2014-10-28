/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010,2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

	Einstellung spi_uart-Trace über define möglich!
	Für die Produktion sollte dieser Trace abgeschaltet
	sein!
	#define db_spi_uart

*********************************************************************************/
#include <linux/err.h>
#include <linux/platform_device.h>	
#include <linux/spi/spi.h>	

#include "../mct_types.h"
#include "../mct_debug.h"			// debug-support
#include "mct_paa_mux_objs.h"
#include "mct_paa_spi.h"
#include "mct_paa_msg.h"
#include "mct_paa.h"


/// Local Debug
// *** developer
/*
static u8 spi_uart_state; 
#define db_spi_uart(state,message)	{	if(state != spi_uart_state) {	\
	printk("spi.uart:%s\n",message);\
	spi_uart_state = state; }}
*/
// *** production
#define db_spi_uart(state,message)

/// Makros für DRIVER-STRUKTUR
#define DR_GET_STATE(val)	{	\
			spin_lock_bh(&_mux->Op_Control->lock);	\
			val = _mux->Op_Control->P_Control.value[1] ;	\
			spin_unlock_bh(&_mux->Op_Control->lock);	}

#define DR_SET_STATE(val)	{	\
			spin_lock_bh(&_mux->Op_Control->lock);	\
			_mux->Op_Control->P_Control.value[1] = val ;	\
			spin_unlock_bh(&_mux->Op_Control->lock);	}

#define DR_SET_ERRORS_CODE(idx,val)	{	\
			spin_lock_bh(&_mux->Op_Errors->lock);	\
			_mux->Op_Errors->P_Code[idx].value = val ;	\
			spin_unlock_bh(&_mux->Op_Errors->lock);	\	}

#define DR_GET_ERRORS_CODE(idx,val)	{	\
			spin_lock_bh(&_mux->Op_Errors->lock);	\
			val = _mux->Op_Errors->P_Code[idx].value;	\
			spin_unlock_bh(&_mux->Op_Errors->lock);	\	}
/*
#define DR_GET_SLOTS(val)	{	\
			LBUS_LOCK;	\
			val = _mux->Op_LBus->P_Slots.value;	\
			LBUS_UNLOCK;	}
*/
#define DR_GET_SLOTS(val)	val = _mux->Op_LBus->P_Slots.value;	


#define CT_SET_UART(val) 	atomic_set(&_ldev->uart,val );
#define CT_GET_UART(val) 	val = atomic_read(&_ldev->uart);

#define LBUS(part)			_mux->lbus.part

//---------------------------------------------------------------
///	PAA-completion-spi-init
//---------------------------------------------------------------
void spi_completion_uart_init (void * data)	{
	struct paa_mux 		* _mux = (struct paa_mux *) data;
	struct lower_device	* _ldev = (struct lower_device *) _mux->ldev;
	db_spi_uart(UART_INIT_CPL_REQ,"INIT_CPL");
	CT_SET_UART(UART_INIT_CPL_ACK);
}

//---------------------------------------------------------------
///	PAA-completion-spi-state
//---------------------------------------------------------------
void spi_completion_uart_pump (void * data)	{
	struct paa_mux 		* 	_mux = (struct paa_mux *) data;
	struct lower_device	* 	_ldev = (struct lower_device *) _mux->ldev;
	u8						_read= 0;
	u8						_inchar=0;
	u8						_reg_lsr=0;
	u8						_reg_rxlvl=0;
	u8						_reg_txlvl=0;
	unsigned char			_slot=0;
	unsigned char			_slots=0;
	unsigned char			_cmd = 0;
	unsigned char*			_req=NULL;
	unsigned char			_off_cnt=0;
	unsigned char			_len=0;
	unsigned char			_outlen=0;
	unsigned char			_inlen=0;
	unsigned char			_crc=0;
	unsigned char			_pos=0;
	struct 	paa_message	*	_msg=NULL;
	struct 	paa_message *	_msg_tmp=NULL;
	struct 	paa_device	*	_paa=NULL;
	unsigned char			_err_cnt=0;
	unsigned char			_tx_buf[PAA_BUFFER_SIZE];
	unsigned char			_rx_buf[PAA_BUFFER_SIZE];

	void	(*_cpl)(void *_ctx) = NULL;
	void	*_ctx;	
	DR_GET_SLOTS(_slots);				// unterstütze Slotanzahl ermitteln

	///********************
	/// PAA-CLIENT-MESSAGES
	///********************
	if(list_empty(&_mux->transfer_list)) {
		goto PAA_CLIENT_MESSAGE_EMPTY;
	}
	else {
		spin_lock(&_mux->transfer_lock);
		list_for_each_entry_safe(_msg, _msg_tmp, &_mux->transfer_list, transfer_item) {
			_paa = _msg->paa;
			_slot= _paa->slot_select;			 
			// printk("Modalias: %s Slot:%d Cmd:%d\n", _paa->modalias, _slot, _cmd);
			if(_slot <= (_slots-1)) {
				_cmd = _msg->tx_typ;
				_len = _msg->tx_len;
				// Länge überwachen!	
				if(_len > PAA_BUFFER_SIZE)
					_len = PAA_BUFFER_SIZE;
				memcpy(_tx_buf,_msg->tx_buf,_len);

				if(LBUS(ctrl[_slot].offline)) {
					/// FEHLER: SLOT OFFLINE!
					_msg->rx_typ = PAA_MSG_ERROR;
					_msg->rx_len = sizeof(LABEL_SLOT_OFFLINE); //+1; HACK 22.02.2011
					memcpy(&_msg->rx_buf,LABEL_SLOT_OFFLINE,_msg->rx_len);
					goto PAA_CLIENT_MESSAGE_COMPLETE;
				}
				// Message-Type auswerten
				switch(_cmd)	{
					case PAA_MSG_STATE:
						// printk("PAA_MSG_STATE\n");
						if((LBUS(ctrl[_slot].ext_cfg) == 1) && (LBUS(ctrl[_slot].int_cfg) == 1)){
							_len  = LBUS(ctrl[_slot].cfg_cfg);
							
							if(_len > LBUS_DEF_DATA_LEN)
								_len = LBUS_DEF_DATA_LEN;
							/// LBUS Nachricht
							memcpy(_rx_buf,LBUS(ctrl[_slot].buf_cfg),_len);	
							_msg->rx_typ = PAA_MSG_CONFIG;	// CONFIG-Antwort beendet
							_msg->rx_len = _len;
							memcpy(&_msg->rx_buf,_rx_buf,_msg->rx_len);
						}
						else {
							_msg->rx_typ = PAA_MSG_STATE;	// CONFIG-Antwort pending!
							_msg->rx_len = 0;
						}
						break;
					
					case PAA_MSG_INFO:
						_len = strlen(LBUS(ctrl[_slot].buf_nam))+1;
						// Länge überwachen!	
						if(_len > PAA_BUFFER_SIZE)
							_len = PAA_BUFFER_SIZE;
						memcpy(_rx_buf,LBUS(ctrl[_slot].buf_nam),_len);
#ifdef MCT_PAA_DEBUG					
						printk("hardware              : %s\n",_rx_buf);
#endif		
						if(!strcmp(_tx_buf,_rx_buf)) {
							_msg->rx_typ = PAA_MSG_INFO;
							_msg->rx_len = sizeof(LABEL_SLOT_ONLINE); //+1; HACK 22.02.2011
							memcpy(&_msg->rx_buf,LABEL_SLOT_ONLINE,_msg->rx_len);
						}
						else {	
							/// FEHLER: SLOT CONFUSION!
							_msg->rx_typ = PAA_MSG_ERROR;
							_msg->rx_len = sizeof(LABEL_SLOT_CONFUSED); // +1; HACK 13.01.2012
							memcpy(&_msg->rx_buf,LABEL_SLOT_CONFUSED,_msg->rx_len);
						}
						goto PAA_CLIENT_MESSAGE_COMPLETE;
						break;

					case PAA_MSG_CONFIG:
						///++++++++++++++
						/// Cfg-Module
						///++++++++++++++
						// _tx_buf 	= [MSG: 0...CFG-Data]
						// _len		= [MSG: 0...CFG-Länge]
						_len  = LBUS(ctrl[_slot].cfg_cfg);
						if(_len > LBUS_DEF_DATA_LEN)
							_len = LBUS_DEF_DATA_LEN;
						memcpy(LBUS(ctrl[_slot].buf_cfg), _tx_buf,_len);
						LBUS(ctrl[_slot].ext_cfg) = 1; 	
						_msg->rx_typ = PAA_MSG_STATE;	// CONFIG-Antwort pending!
						_msg->rx_len = 0;						
						break;

					case PAA_MSG_DATA:
						// _tx_buf 	= [MSG: 0...OUT-Data]
						// _len		= [MSG: 0...OUT-Länge]
						// Falls ein Gerät OUT-Pins hat, dann enthält der
						// _tx_buf die Daten und eine Längenangabe!
						_outlen = LBUS(ctrl[_slot].cfg_out);	
						_inlen  = LBUS(ctrl[_slot].cfg_in);
						memcpy(_rx_buf,LBUS(ctrl[_slot].buf_in),_outlen + _inlen);
						// _rx_buf = [LBUS: IN-DATA]
						// Der Puffer _rx_buf enthält die letzten aktuellen Daten
						// die ein Gerät an den LBUS gesendet hat.
						// Die Länge der Daten ist immer die Summe aus _outlen und
						// _inlen, wobei mindestens einer der beiden Werte größer 0
						// sein muss.
						memcpy(LBUS(ctrl[_slot].buf_out), _tx_buf,_outlen);
						LBUS(ctrl[_slot].ext_ini) = 1; 
						///++++++++++++++
						/// In-Module
						///++++++++++++++
						_msg->rx_typ = PAA_MSG_DATA;
						_msg->rx_len = _outlen +_inlen;
						memcpy(&_msg->rx_buf,_rx_buf,_msg->rx_len);
						break;

					case PAA_MSG_BYEBYE:
						// RESET ans Gerät und anschließend in den PING-Mode
						_msg->rx_typ = PAA_MSG_BYEBYE;
						_msg->rx_len = sizeof(PAA_MSG_STR_BYEBYE); //+1; HACK 22.10.2011
						memcpy(&_msg->rx_buf,PAA_MSG_STR_BYEBYE,_msg->rx_len);
						LBUS(snd[_slot].cmd) = LBUS_REQ_RES;
						break;	

					case PAA_MSG_INIT:
					default: 
						/// FEHLER: Msg vom Master nicht unterstützt!
						_msg->rx_typ = PAA_MSG_ERROR;
						_msg->rx_len = sizeof(PAA_MSG_STR_UNSUPPORTED); //+1; HACK 22.10.2011
						memcpy(&_msg->rx_buf,PAA_MSG_STR_UNSUPPORTED,_msg->rx_len);
						break;	
				}
			}
			else {	
					/// FEHLER: SLOT MISSING!
					_msg->rx_typ = PAA_MSG_ERROR;
					_msg->rx_len = sizeof(LABEL_SLOT_MISSING); //+1; HACK 22.10.2011
					memcpy(&_msg->rx_buf,LABEL_SLOT_MISSING,_msg->rx_len);
			}

PAA_CLIENT_MESSAGE_COMPLETE:
			_cpl = _msg->complete;
			_ctx = _msg->context;
			list_del(_mux->transfer_list.next);
			spin_unlock(&_mux->transfer_lock);
			_cpl(_ctx);
			spin_lock(&_mux->transfer_lock);
		}
		spin_unlock(&_mux->transfer_lock);
	}
PAA_CLIENT_MESSAGE_EMPTY:

	///*******************
	/// SPI-UART-Register
	///*******************
	_reg_lsr 	= _ldev->uart_cmdi[3];	// line state
	_reg_rxlvl 	= _ldev->uart_cmdi[5];	// receive level
	_reg_txlvl 	= _ldev->uart_cmdi[7];	// transmit level
	if( LSR_ERROR(_reg_lsr))	{
		DEBUG(MCT_DEBUG_LEVEL3,"spi.uart:Error LSR %x Register Reset!\n",_reg_lsr);
		CT_SET_UART(UART_ERROR);
		return;
	}

	///*******************
	/// LBUS-REC-Telegramm
	///*******************
	// LBUS-Telegramm-Aufbau prüfen!
	for(_read=0;_ldev->read; _ldev->read--,_read++)	{ 
		_inchar = _ldev->dual_buffer[DUAL_RD_DATA +_read];		
		//printk("%x-",_inchar);	
		switch(LBUS(rec.state))	{
			/// IDLE
			case LBUS_REC_STATE_IDLE: 	// Start erwarten
				if(_inchar == LBUS_SIGN_START) {
					LBUS(rec.pos) = 1;
					LBUS(rec.err) = 0;
					LBUS(rec.err_slv)=0;
					LBUS(rec.crc) = 0;
					LBUS(rec.state) = LBUS_REC_STATE_START;
				}
				/// Slip-Mode ...
				break;
			/// START
			case LBUS_REC_STATE_START:	// Adresse erwarten
				LBUS(rec.pos)++;
			#ifdef LBUS_MASTER
				if(_inchar <= LBUS_ADD_MAX)
			#else
				if(_inchar <= LBUS_ADD_MAX || _inchar == LBUS_ADD_FF)
			#endif 
				{				
					LBUS(rec.adr) = _inchar;
					LBUS(rec.crc) += _inchar; 
					LBUS(rec.state) = LBUS_REC_STATE_A_FLD;
				} else {
					DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Adresse %02d\n", _inchar);
					LBUS(rec.err) = LBUS_ERR_ADD;
					LBUS(rec.state) = LBUS_REC_STATE_RDY;
				}
				break;
			/// ADRESSE
			case LBUS_REC_STATE_A_FLD:	// Kommando erwarten
				LBUS(rec.pos)++;
				#ifdef LBUS_MASTER
				if( (_inchar == LBUS_RSP_RES) ||
					(_inchar == LBUS_RSP_WHO) || 
					(_inchar == LBUS_RSP_PING) ||
					(_inchar == LBUS_RSP_CFG) ||
					(_inchar == LBUS_RSP_DATA)) 
				#else	
				if( (_inchar == LBUS_BUS_UP) ||
					(_inchar == LBUS_BUS_DOWN) ||
					(_inchar == LBUS_REQ_RES) ||
					(_inchar == LBUS_REQ_WHO) || 
					(_inchar == LBUS_REQ_PING) ||
					(_inchar == LBUS_REQ_CFG) ||
					(_inchar == LBUS_REQ_DATA)) 
				#endif
					{
					LBUS(rec.cmd) = _inchar;	// Kommando merken!
					LBUS(rec.crc) += _inchar;
					LBUS(rec.state) = LBUS_REC_STATE_C_FLD;
				} else {
					/// Kommando ungültig!
					DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Kommando\n");
					LBUS(rec.err) = LBUS_ERR_CMD;
					LBUS(rec.state) = LBUS_REC_STATE_RDY;
				}
				break;
			/// COMMAND
			case LBUS_REC_STATE_C_FLD:
				LBUS(rec.pos)++;
				if(_inchar <= LBUS_ERR_TIME) {	// 0 dann CRC-Feld
					LBUS(rec.err_slv) = _inchar;
					LBUS(rec.state) = LBUS_REC_STATE_E_FLD;
				}
				else	{
					DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Slave-Code\n");
					LBUS(rec.err) = LBUS_ERR_ERROR;
					LBUS(rec.state) = LBUS_REC_STATE_RDY;
				}
				break;
			/// ERROR
			case LBUS_REC_STATE_E_FLD:			// Länge erwarten
				LBUS(rec.pos)++;
				LBUS(rec.dat_idx) = 0;			// Datenanfang
				LBUS(rec.dat_len) = _inchar;	// Länge übernehmen
				if(_inchar == 0) {				// 0 dann CRC-Feld
					LBUS(rec.state) = LBUS_REC_STATE_CRC;
					break;
				}
				if(_inchar <= LBUS_DEF_DATA_LEN) {
					LBUS(rec.crc) += _inchar;
					LBUS(rec.state) = LBUS_REC_STATE_DATA;
				}
				else {
					DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Datenlaenge\n");
					LBUS(rec.dat_len) = 0;
					LBUS(rec.err) = LBUS_ERR_SIZE;
					LBUS(rec.state) = LBUS_REC_STATE_RDY;
				}
				break;
			/// DATEN
			case LBUS_REC_STATE_DATA:
				LBUS(rec.pos)++;
				LBUS(rec.dat[LBUS(rec.dat_idx)]) = _inchar;	// Kopiere Datenbyte
				LBUS(rec.dat_idx)++; 
				LBUS(rec.crc) += _inchar;
				if(LBUS(rec.dat_len) == LBUS(rec.dat_idx) )	{
					LBUS(rec.state) = LBUS_REC_STATE_CRC;
				}
				break;
			/// CHECKSUMME
			case LBUS_REC_STATE_CRC:
				LBUS(rec.pos)++;
				LBUS(rec.crc) += _inchar;
				//printk("CRC calc: %d\n",LBUS(rec.crc));
				if(!LBUS(rec.crc)) {
					LBUS(rec.state) = LBUS_REC_STATE_STOP;
				}
				else {
					DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error CRC\n");
					LBUS(rec.err) = LBUS_ERR_CRC;
					LBUS(rec.state) = LBUS_REC_STATE_RDY;
				}
				break;
			/// STOP
			case LBUS_REC_STATE_STOP:
				LBUS(rec.pos)++;
				if(_inchar == LBUS_SIGN_STOP) {
					LBUS(rec.state) = LBUS_REC_STATE_RDY;
				}
				else {
					DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Stop-Zeichen\n");
					LBUS(rec.err) = LBUS_ERR_END;
					LBUS(rec.state) = LBUS_REC_STATE_RDY;	
				}
				break;
			case LBUS_REC_STATE_RDY:
				// Solange bis alle Zeichen im Puffer ausgewertet sind,
				// geht's hier durch. Wenn ein Fehler auftrat, wird dieser
				// in rec.error gespeichert und die weiteren Zeichen im Puffer
				// ignoriert!	
				break;
			default:
				DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error unmoeglich!?\n");
				LBUS(rec.state) = LBUS_REC_STATE_RDY;				
				break;
		}
		/// Längenkontrolle über alles!
		if(LBUS(rec.pos) > (LBUS_DEF_TLG_LEN + LBUS_DEF_DATA_LEN)) {
			DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Telegrammlaenge\n");
			LBUS(rec.err) = LBUS_ERR_LEN;
			LBUS(rec.state) = LBUS_REC_STATE_RDY;
		} 
	}	// End for
	_slot 	= LBUS(slot);			// aktiven slot holen

	///*******************
	/// LBUS-REC-Timeout
	///*******************
	switch(LBUS(mode)) {
		case LBUS_MODE_UP:
		case LBUS_MODE_UP_WAIT:
		case LBUS_MODE_DOWN:
		case LBUS_MODE_DOWN_WAIT:
		case LBUS_MODE_SLEEP:
			goto NEXT_WRITE;
			break;
		case LBUS_MODE_REC:
			if(LBUS(time) > 0 ) {
				LBUS(time)--;
			}
			else {	
				DEBUG(MCT_DEBUG_LEVEL3,"LBUS: Error Timeout slot:%d\n",_slot);
				LBUS(rec.err) = LBUS_ERR_TIME;
				LBUS(rec.state) = LBUS_REC_STATE_RDY;
			}
			break;
	}
	
	///*******************
	/// LBUS_REC-Protokoll
	///*******************
	if (LBUS(rec.state) == LBUS_REC_STATE_RDY) {	
		if(LBUS(rec.err) == LBUS_ERR_TIME) {
		   /// Telegramm-Timeouts
			// Ein Gerät wird als offline markiert, wenn es zu einem
			// Telegramm-Timeout in den Modi: REQ_WHO, REQ_PING, REQ_CFG kommt. 
			_cmd = 	LBUS(snd[_slot].cmd);
			switch (_cmd) {
				case LBUS_REQ_WHO:
					memset(&LBUS(ctrl[_slot]), 0, sizeof(struct t_attr));
					LBUS(ctrl[_slot].offline) = 1;	// Kein Gerät mit der Nummer am Bus!
					break;

				case LBUS_REQ_PING:
					// PING-Timeout nach MUX_TIMEOUT_ON_PING_MAX
					spin_lock_bh(&_mux->Op_Timeouts->lock);
					_err_cnt = _mux->Op_Timeouts->P_Counter[_slot].value++;
					spin_unlock_bh(&_mux->Op_Timeouts->lock);

					if(_err_cnt <  MUX_TIMEOUT_ON_PING_MAX) {
						// printk("ping timeout count: %d\n",_err_cnt);
					}
					else {
						spin_lock_bh(&_mux->Op_Timeouts->lock);
						_mux->Op_Timeouts->P_Code[_slot].value = LBUS_REQ_PING;
						spin_unlock_bh(&_mux->Op_Timeouts->lock);
		
						memset(&LBUS(ctrl[_slot]), 0, sizeof(struct t_attr));
						sprintf((char*)(LBUS(ctrl[_slot].buf_nam)),"<offline:ping>");
						LBUS(ctrl[_slot].offline) = 1;	// Gerät antwortet nicht mehr
					}
					break;

				case LBUS_REQ_CFG:
					// CONFIG-Timeout nach MUX_TIMEOUT_ON_CONFIG_MAX
					spin_lock_bh(&_mux->Op_Timeouts->lock);
					_err_cnt = _mux->Op_Timeouts->P_Counter[_slot].value++;
					spin_unlock_bh(&_mux->Op_Timeouts->lock);

					if(_err_cnt <  MUX_TIMEOUT_ON_CONFIG_MAX) {
						// printk("config timeout count: %d\n",_err_cnt);
					}
					else {
						spin_lock_bh(&_mux->Op_Timeouts->lock);
						_mux->Op_Timeouts->P_Code[_slot].value = LBUS_REQ_CFG;
						spin_unlock_bh(&_mux->Op_Timeouts->lock);
						
						memset(&LBUS(ctrl[_slot]), 0, sizeof(struct t_attr));
						sprintf((char*)(LBUS(ctrl[_slot].buf_nam)),"<offline:config>");
						LBUS(ctrl[_slot].offline) = 1;	// Gerät antwortet nicht mehr
					}
					break;

				case LBUS_REQ_DATA:
					// DATA-Timeout nach MUX_TIMEOUT_ON_DATA_MAX
					spin_lock_bh(&_mux->Op_Timeouts->lock);
					_err_cnt = _mux->Op_Timeouts->P_Counter[_slot].value++;
					spin_unlock_bh(&_mux->Op_Timeouts->lock);

					if(_err_cnt <  MUX_TIMEOUT_ON_DATA_MAX) {
						// printk("data timeout count: %d\n",_err_cnt);
					}
					else {
						spin_lock_bh(&_mux->Op_Timeouts->lock);
						_mux->Op_Timeouts->P_Code[_slot].value = LBUS_REQ_DATA;
						spin_unlock_bh(&_mux->Op_Timeouts->lock);

						memset(&LBUS(ctrl[_slot]), 0, sizeof(struct t_attr));
						sprintf((char*)(LBUS(ctrl[_slot].buf_nam)),"<offline:data>");
						LBUS(ctrl[_slot].offline) = 1;	// Gerät antwortet nicht mehr
					}
					break;
			}
		} else {
			// 	
			spin_lock_bh(&_mux->Op_Timeouts->lock);
			_mux->Op_Timeouts->P_Counter[_slot].value =0;
			_mux->Op_Timeouts->P_Code[_slot].value = 0;
			spin_unlock_bh(&_mux->Op_Timeouts->lock);
		}
		
		if  (LBUS(rec.err) != 0) {
			// Alle Empfangstelegrammfehler erzwingen eine SLOT-Umschaltung
			// Dadurch wird das Telegramm nicht
			LBUS(rec.err) = 0;
			goto NEXT_SLOT;
		}

		if(LBUS(snd[_slot].cmd) != (LBUS(rec.cmd) & 0x7f)) {
			/// falsches Telegramm (logic)
			DEBUG(MCT_DEBUG_LEVEL3, "LBUS: Error unexpected answer! slot:%d snd:%d rec:%d\n",_slot, LBUS(snd[_slot].cmd), LBUS(rec.cmd));
		}
		else { 
			switch(LBUS(rec.cmd)) {
				case LBUS_RSP_WHO:
					db_lbus_rsp("LBUS_RSP_WHO",_slot);
					LBUS(ctrl[_slot].offline) 	= 0;	// Gerät am Bus!
					LBUS(ctrl[_slot].cfg)		= LBUS(rec.dat[RSP_WHO_CFG_IDX]);	
					LBUS(ctrl[_slot].cfg_cfg)	= LBUS(rec.dat[RSP_WHO_CFG_IDX]) & RSP_WHO_CFG_SIZE_MASK;
					LBUS(ctrl[_slot].cfg_msk)	= LBUS(rec.dat[RSP_WHO_CFG_IDX]) & RSP_WHO_CFG_HANDMASK_BIT;
					LBUS(ctrl[_slot].cfg_in)	= LBUS(rec.dat[RSP_WHO_CFG_IN_IDX]);
					LBUS(ctrl[_slot].cfg_out)	= LBUS(rec.dat[RSP_WHO_CFG_OUT_IDX]);
					memset(LBUS(ctrl[_slot].buf_in),0,LBUS_DEF_DATA_LEN);
					memset(LBUS(ctrl[_slot].buf_out),0,LBUS_DEF_DATA_LEN);
					_len = sprintf((char*)(LBUS(ctrl[_slot].buf_nam)),(char*)(&LBUS(rec.dat[RSP_WHO_NAME_IDX])));
					//HACK	
					// Gerätenamen um die Slotnummer erweitern und dann
					// komplett vergleichen!
					LBUS(ctrl[_slot].buf_nam[_len]) = '_';
					LBUS(ctrl[_slot].buf_nam[_len+1]) = _slot + 0x30;
					LBUS(ctrl[_slot].buf_nam[_len+2]) = '\0';	

					memcpy(	LBUS(ctrl[_slot].buf_ver),\
							&LBUS(rec.dat[RSP_WHO_VERSION_IDX]),\
							RSP_WHO_VERSION_SIZE);
					sprintf( LBUS(ctrl[_slot].buf_ver),"%c-%d.%d",	
 							LBUS(ctrl[_slot].buf_ver[0]),\
							(LBUS(ctrl[_slot].buf_ver[1]) >> 4),\
							(LBUS(ctrl[_slot].buf_ver[1]) & 0x0f));

					// Output-Geräte in den Ping-Modus
					if(LBUS(ctrl[_slot].cfg_out) ) {			
						LBUS(snd[_slot].cmd) = LBUS_REQ_PING; 
						break;
					}
					// Input-Geräte mit Konfiguration in den Ping-Modus 
					if( LBUS(ctrl[_slot].cfg) & RSP_WHO_CFG_CONFIG_BIT ) {  
						LBUS(snd[_slot].cmd) = LBUS_REQ_PING; 	
						break;
					}											
					// Input-Geräte (nur Input ohne Konfiguration) in den Data Modus
					LBUS(snd[_slot].cmd) = LBUS_REQ_DATA;			
					break;

				// Im Ping-Mode wartet das Gerät auf eine Konfiguration,
				// oder auf eine Initialisierung!
				case LBUS_RSP_PING:
					db_lbus_rsp("LBUS_RSP_PING",_slot);
					// Ping beantwortet!

					//-------------------
					// Konfiguration ?
					//-------------------
					// Das Gerät benötigt wichtige Konfigurationseinstellungen.
					// 1) Extern kann in FS[slot].cfg_cfg die LÄNGE der benötigten
					// Konfiguration "gelesen" werden!
					// 2) Extern muss in FS[slot].buf_cfg die Konfigurationssequenz
					// eingetragen werden 
					// 3) Extern muss das Flag FS[_slot].ext_cfg= 1 gesezt werden!
					if((LBUS(ctrl[_slot].ext_cfg) == 1) && (LBUS(ctrl[_slot].int_cfg) == 0)) { 
						db_lbus_rsp("LBUS_RSP_PING_CFG",_slot);
						//+++++++++++++++
						// CFG-PROLOG
						//+++++++++++++++
						// start, adresse, command, error, len 
						LBUS(snd[_slot].msg_cfg[LBUS_IDX_STA]) = LBUS_SIGN_START;
						_crc = 0;
						LBUS(snd[_slot].msg_cfg[LBUS_IDX_ADD]) = _slot;
						_crc += LBUS(snd[_slot].msg_cfg[LBUS_IDX_ADD]);		
						LBUS(snd[_slot].msg_cfg[LBUS_IDX_CMD]) = LBUS_REQ_CFG;
						_crc += LBUS(snd[_slot].msg_cfg[LBUS_IDX_CMD]);
						LBUS(snd[_slot].msg_cfg[LBUS_IDX_ERR]) = 0;
						LBUS(snd[_slot].msg_cfg[LBUS_IDX_LEN]) = LBUS(ctrl[_slot].cfg_cfg);
						_crc +=	LBUS(snd[_slot].msg_cfg[LBUS_IDX_LEN]);
						_pos = LBUS_IDX_DATA;
						//+++++++++++++++
						// CFG-BEREICH
						//+++++++++++++++
						memcpy(	&LBUS(snd[_slot].msg_cfg[LBUS_IDX_DATA]),\
								&LBUS(ctrl[_slot].buf_cfg),\
								LBUS(ctrl[_slot].cfg_cfg));

						for (_len = 0; _len < LBUS(ctrl[_slot].cfg_cfg); _len++) {
							_crc += LBUS(snd[_slot].msg_cfg[_pos+_len]);
						}
						_pos += LBUS(ctrl[_slot].cfg_cfg);		
						//+++++++++++++++
						// CFG-EPILOG
						//+++++++++++++++
						_crc = (~_crc) + 1;		
						LBUS(snd[_slot].msg_cfg[_pos]) = _crc;
						LBUS(snd[_slot].msg_cfg[_pos+1]) = LBUS_SIGN_STOP;
						LBUS(snd[_slot].cmd) = LBUS_REQ_CFG;
						break;
					}
					//-------------------
					// Initialisierung
					//-------------------
					// Das Gerät hat Ausgänge die initialisiert werden  müssen!
					// 1) Extern müssen die Outputs in FS[_slot].buf_out geschrieben sein! 
					// 2) Extern muss dann das Flag FS[_slot].ext_ini= 1 gesetzt werden!
					// 3) Der Outputbuffer wird in die Message kopiert und an das Gerät
					//	  gesendet!
					if(LBUS(ctrl[_slot].ext_ini) == 1) {
						db_lbus_rsp("LBUS_RSP_PING_INI",_slot);
						LBUS(snd[_slot].cmd) = LBUS_REQ_DATA;
						goto LABEL_RSP_DATA_AFTER_PING;
					}	
					break;

				case LBUS_RSP_CFG:
					db_lbus_rsp("LBUS_RSP_CFG",_slot); 
					memcpy(&LBUS(ctrl[_slot].buf_cfg),&LBUS(rec.dat[0]),LBUS(rec.dat_len));	
					/*
					printk("SLOT: %d cfg-size: %d Config:",_slot, LBUS(ctrl[_slot].cfg_cfg));
					for(len = 0; _len < LBUS(ctrl[_slot].cfg_cfg); _len++) {
						printk("-%d",LBUS(ctrl[_slot].buf_cfg[_len]));
					}
					printk("\n"
					*/
					if(LBUS(ctrl[_slot].cfg_out))
						// Gerät hat noch Outputs, dann Initialisierung!
						LBUS(snd[_slot].cmd) = LBUS_REQ_PING;
					else
						// Gerät hat nur Inputs, dann Data-Mode	
						LBUS(snd[_slot].cmd) = LBUS_REQ_DATA; 						
					//Geräte Konfiguration erfolgreich abgeschlossen!
					LBUS(ctrl[_slot].int_cfg) = 1;	
					break;

				case LBUS_RSP_RES:
					db_lbus_rsp("LBUS_RSP_RES",_slot);
					LBUS(ctrl[_slot].int_cfg) = 0;	// Konfiguration verworfen
					LBUS(ctrl[_slot].ext_cfg) = 0;	// Keine externe Konfiguration da!
					LBUS(ctrl[_slot].int_ini) = 0;	// Initial-Werte verworfen!
					LBUS(ctrl[_slot].ext_ini) = 0;	// Keine externen Werte da!
					// Reset Err-Counter	
					spin_lock_bh(&_mux->Op_Timeouts->lock);
					_mux->Op_Timeouts->P_Counter[_slot].value=0;
					spin_unlock_bh(&_mux->Op_Timeouts->lock);

					LBUS(snd[_slot].cmd) = LBUS_REQ_PING; 
					break;

				case LBUS_RSP_DATA:
					db_lbus_rsp("LBUS_RSP_DATA",_slot); 
					if(!LBUS(ctrl[_slot].int_ini))
						LBUS(ctrl[_slot].int_ini)=1;	
					
					_len = LBUS(ctrl[_slot].cfg_in) + LBUS(ctrl[_slot].cfg_out);
					
					if(LBUS(rec.err) == 0){
						memcpy(LBUS(ctrl[_slot].buf_in),&LBUS(rec.dat[0]),_len);
					}

LABEL_RSP_DATA_AFTER_PING:
					if(	LBUS(ctrl[_slot].cfg_out)) {
						// Das Gerät hat Ausgänge die aktualisiert werden müssen!
						// 1) Es werden die Daten aus FS[_slot].buf_out kopiert. 
						// 2) Data-Telegramm wird gebaut und CRC wird berechnet
						//+++++++++++++++
						// DATA-PROLOG
						//+++++++++++++++
						// start, adresse, command, error, len 
						LBUS(snd[_slot].msg_data[LBUS_IDX_STA]) = LBUS_SIGN_START;
						_crc = 0;
						LBUS(snd[_slot].msg_data[LBUS_IDX_ADD]) = _slot;
						_crc += LBUS(snd[_slot].msg_data[LBUS_IDX_ADD]);		
						LBUS(snd[_slot].msg_data[LBUS_IDX_CMD]) = LBUS_REQ_DATA;
						_crc += LBUS(snd[_slot].msg_data[LBUS_IDX_CMD]);
						LBUS(snd[_slot].msg_data[LBUS_IDX_ERR]) = 0;
						LBUS(snd[_slot].msg_data[LBUS_IDX_LEN]) = LBUS(ctrl[_slot].cfg_out);
						_crc +=	LBUS(snd[_slot].msg_data[LBUS_IDX_LEN]);
						_pos = LBUS_IDX_DATA;
						//+++++++++++++++
						// DATA-BEREICH
						//+++++++++++++++
						memcpy(	&LBUS(snd[_slot].msg_data[LBUS_IDX_DATA]),\
								&LBUS(ctrl[_slot].buf_out),\
								LBUS(ctrl[_slot].cfg_out));
						for (_len = 0; _len < LBUS(ctrl[_slot].cfg_out); _len++) {
							//printk("-%x",LBUS(snd[_slot].msg_data[_pos+_len]));
							_crc += LBUS(snd[_slot].msg_data[_pos+_len]);
						}
						//printk("\n");
						
						_pos += LBUS(ctrl[_slot].cfg_out);		
						//+++++++++++++++
						// DATA-EPILOG
						//+++++++++++++++
						_crc = (~_crc) +1;		
						LBUS(snd[_slot].msg_data[_pos]) = _crc;
						LBUS(snd[_slot].msg_data[_pos+1]) = LBUS_SIGN_STOP;
					}	
					/// DATA-TELEGRAMM vorbereitet!
					LBUS(snd[_slot].cmd) = LBUS_REQ_DATA;	
					break;

				default: 
					break;
			}
		}	// if else  
NEXT_SLOT:	
		 _off_cnt = 0;	
		 while(_off_cnt < _slots) {
			_slot++;
			if(_slot ==  _slots)
				_slot = 0;
			if(LBUS(ctrl[_slot].offline)) {	
				_off_cnt++;
			}
			else {	
				break;
			}
		}
		/// Überhaupt ein Gerät am BUS?	
		if(	_off_cnt == _slots) {
			printk("LBUS: No devices!\n");
			LBUS(slot) 		= 0;
			LBUS(mode) 		= LBUS_MODE_DOWN;
		} else {
			LBUS(slot) 		= _slot;		// neuen Slot sichern
			LBUS(mode) 		= LBUS_MODE_SND;
		}
		LBUS(rec.state) = LBUS_REC_STATE_IDLE;
	}

NEXT_WRITE:
	/// ***********************************
	/// LSR-DATA-WRITE
	/// *********************************** 
	if( LSR_TX_EMPTY(_reg_lsr)){
		/// STARTEN ...
		switch(LBUS(mode)) {
				/// BUS-Hoch
			case LBUS_MODE_UP:
				for(_slot = 0; _slot < _slots; _slot++)
					LBUS(ctrl[_slot].offline) = 0;	// alle Slots erstmal online!
				_ldev->write = LBUS_DEF_TLG_LEN;	// Sendeauftrag!
				db_lbus(LBUS_MODE_UP_STR);			// debug_lbus
				memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],LBUS(msg_up),LBUS_DEF_TLG_LEN);
				LBUS(mode) = LBUS_MODE_UP_WAIT;
				break;	
				/// BUS-Idle
			case LBUS_MODE_UP_WAIT: 
				db_lbus(LBUS_MODE_UP_WAIT_STR);		// debug_lbus
				LBUS(mode) = LBUS_MODE_SND;			// Achtung, fällt durch
				/// BUS-als Sender
			case LBUS_MODE_SND:
				_slot 	= LBUS(slot);				// aktuellen Slot holen
				_cmd 	= LBUS(snd[_slot].cmd);		// aktuelles Kommando holen 
				_req 	= LBUS(snd[_slot].msgs[_cmd]);	// aktuelles Telegramm
				switch(_cmd) {
					case LBUS_REQ_WHO:
						 db_lbus_req("LBUS_REQ_WHO",_slot);
						_ldev->write = LBUS_DEF_TLG_LEN;		// länge 
						memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],_req,_ldev->write);
						LBUS(time) = TIMER_TIMEOUT_SLOW;
						break;

					case LBUS_REQ_RES:				
						db_lbus_req("LBUS_REQ_RES",_slot);
						_ldev->write = LBUS_DEF_TLG_LEN;		// länge 
						memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],_req,_ldev->write);
						LBUS(time) = TIMER_TIMEOUT_SLOW;
						break;

					case LBUS_REQ_PING:
			 			db_lbus_req("LBUS_REQ_PING",_slot);
						_ldev->write = LBUS_DEF_TLG_LEN;		// länge 
						memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],_req,_ldev->write);
						LBUS(time) = TIMER_TIMEOUT_FAST;
						break;

					case LBUS_REQ_CFG:
						db_lbus_req("LBUS_REQ_CFG",_slot);
						_ldev->write = LBUS_DEF_TLG_LEN + LBUS(ctrl[_slot].cfg_cfg); //länge
						memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],_req,_ldev->write);
						//for (_len = 0; _len < _ldev->write; _len++) {
						//	printk("-%x",LBUS(snd[_slot].msg_cfg[_len]));
						//}
						LBUS(time) = TIMER_TIMEOUT_SLOW;
						break;

					case LBUS_REQ_DATA:
						db_lbus_req("LBUS_REQ_DATA",_slot); 
						/// Inputs, vorbelegt
						_ldev->write = LBUS_DEF_TLG_LEN;	
						 if(LBUS(ctrl[_slot].cfg_out)) {
						/// Outputs, vorbelegt
							_ldev->write = _ldev->write + LBUS(ctrl[_slot].cfg_out);
						 }	
						memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],_req,_ldev->write);
						
						if(LBUS(ctrl[_slot].cfg) & RSP_WHO_CFG_SLOWDEV_BIT)
							LBUS(time) = TIMER_TIMEOUT_FAST *2;
						else
							LBUS(time) = TIMER_TIMEOUT_FAST;
						break;

					default:
						LBUS(time) = TIMER_TIMEOUT_SLOW;
						break;
				}
				LBUS(mode) = LBUS_MODE_REC;
				break;
				/// BUS-als Empfänger
			case LBUS_MODE_REC:
				break;
				/// BUS-runter-fahren
			case LBUS_MODE_DOWN:
				_ldev->write = LBUS_DEF_TLG_LEN;	// Sendeauftrag!
				db_lbus(LBUS_MODE_DOWN_STR);		// debug_lbus
				memcpy(	&_ldev->dual_buffer[DUAL_WR_DATA],LBUS(msg_down),LBUS_DEF_TLG_LEN);
				LBUS(mode) = LBUS_MODE_DOWN_WAIT;
				break;
				/// BUS-runter
			case LBUS_MODE_DOWN_WAIT:
				db_lbus(LBUS_MODE_DOWN_WAIT_STR);	// debug_lbus		
				// Reset wichtiger LBUS-Struktur-Elemente um
				// einen erneuten BUS-Scann vorzubereiten! 
				DR_GET_SLOTS(_slots);
				LBUS(time) = TIMER_TIMEOUT_SLOW;
				memset(&LBUS(rec), 0, sizeof(struct t_rec));
				for(_slot=0; _slot < _slots; _slot++) {
					LBUS(snd[_slot].cmd) = LBUS_REQ_WHO;
					memset(&LBUS(ctrl[_slot]), 0, sizeof(struct t_attr));
					LBUS(ctrl[_slot].offline) = 1;
				}
	
				LBUS(mode) = LBUS_MODE_SLEEP;		// Achtung fällt Durch
				
				/// BUS-inaktive
			case LBUS_MODE_SLEEP:
				db_lbus(LBUS_MODE_SLEEP_STR);		// debug_lbus
				break;
			default:
				break;
		}
	}	
	/// ***********************************
	/// LSR-DATA-READ new job!
	/// *********************************** 	
	if(LSR_RX_DATAS(_reg_lsr))
		_ldev->read = _reg_rxlvl; 	// schon wieder neue Zeichen in Lese-FiFo?	
	
	CT_SET_UART(UART_PUMP);
}

void spi_timer_function(unsigned long data)	{
	struct paa_mux 		* 	_mux  = (struct paa_mux *) data;
	struct lower_device	* 	_ldev = (struct lower_device *) _mux->ldev;	
	enum e_state 			_state = 0;
	int 					_uart = 0;
	u8						_i = 0;
	struct 	paa_message	*	_msg = NULL;
	struct 	paa_message *	_msg_tmp = NULL;
	struct 	paa_device	*	_paa = NULL;
	void	(*_cpl)(void *_ctx) = NULL;
	void	*_ctx;	


	_mux->OT.timer.expires = jiffies + _mux->OT.granularity;
	if(atomic_read( &_mux->OT.atomic ) ) {
		complete( &_mux->OT.completion ); 			// final spi timer call
	} else {
		// ***************************
		// DRIVER-STATE 	
		// ***************************	
		DR_GET_STATE(_state);						// driver-status
		CT_GET_UART(_uart);							// uart-status	
		switch(_state)	{
			case e_stop: 							// Stoppen!
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver stopp!\n", __FILE__,__FUNCTION__);	
				if(LBUS(mode) == LBUS_MODE_IDLE) {
					// Treiber 1. Start mit ARG command= 0, 
					// dann kein "Down-Telegramm" an die Clients,
					// da die UART noch gar nicht initialisiert ist!
					// Der LBUS wird in den Status LBUSUART_PUMP_CPL_REQ_SLEEP versetzt.
					LBUS(mode) = LBUS_MODE_SLEEP;
					db_lbus(LBUS_MODE_SLEEP_STR);
				}
				else { 
					// Der LBUS informiert über ein "Down-Telegramm" alle Clients,
					// Der LBUS wird in den Status LBUS_SLEEP versetzt.
					LBUS(mode) = LBUS_MODE_DOWN;
				}
				CT_SET_UART(UART_PUMP);
				DR_SET_STATE(e_running);	
				break;
	
			case e_start:							// Starten!			
				switch(_uart) {
				case UART_INIT:
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver started!\n", __FILE__,__FUNCTION__);
					db_spi_uart(UART_INIT,"INIT");
					CT_SET_UART(UART_INIT_CPL_REQ);
					spi_message_init(&_ldev->msg);
					_ldev->msg.complete = spi_completion_uart_init; 
					_ldev->msg.context = (void*) data;	
					/// SC16IS750-INIT	Message
					_ldev->uart_cmdo = _ldev->uart_init;
					for(_i = 0; _i< sizeof(paa_spi_uart_init)/2; _i++) {
						spi_message_add_tail(&_ldev->InitX[_i], &_ldev->msg);
					}
					/// async write	
					spi_async(_ldev->spi, &_ldev->msg);	
					break;
				case UART_INIT_CPL_ACK:
					db_spi_uart(UART_INIT_CPL_ACK,"INIT_CPL_ACK");
					LBUS(mode) = LBUS_MODE_UP;	//LowerBus hochziehen!
					//printk("_ldev->read: %d",_ldev->read);
					//printk("_ldev->write: %d",_ldev->write);
					CT_SET_UART(UART_PUMP);
					DR_SET_STATE(e_running);
					break;
				default:
					break;
				};
				break;
			
			case e_running:
				switch(_uart)	{
				case UART_PUMP:
					db_spi_uart(UART_PUMP,"PUMP");
					if( LBUS(mode) == LBUS_MODE_SLEEP) {
						CT_SET_UART(UART_INIT);
						DR_SET_STATE(e_idle); 				// Idle-Mode
						break;
					}
					CT_SET_UART(UART_PUMP_CPL_REQ);	
					spi_message_init(&_ldev->msg);
					_ldev->msg.complete = spi_completion_uart_pump; 
					_ldev->msg.context = (void*)data;
					/// READ JOB ?
					if( _ldev->read)	{
						_ldev->RxX.len = (_ldev->read)+1; 
						_ldev->RxX.cs_change = 1;	
						_ldev->RxX.tx_buf = &_ldev->dual_buffer[DUAL_RD_TX];
						_ldev->dual_buffer[DUAL_RD_CMD] = RHR;	//lesejob
						_ldev->RxX.rx_buf = &_ldev->dual_buffer[DUAL_RD_RX];
						spi_message_add_tail(&_ldev->RxX, &_ldev->msg);
					}
					/// WRITE JOB
					if( _ldev->write)	{
						_ldev->TxX.len = (_ldev->write)+1;
						_ldev->TxX.cs_change = 1;	
						_ldev->TxX.tx_buf = &_ldev->dual_buffer[DUAL_WR_TX];
						_ldev->dual_buffer[DUAL_WR_CMD] = THR;	// schreibjob
						_ldev->write = 0;
						spi_message_add_tail(&_ldev->TxX, &_ldev->msg);
					}
					/// SC16IS750-STATE Message
					_ldev->uart_cmdo = _ldev->uart_state;
					for(_i = 0; _i< sizeof(paa_spi_uart_state)/2; _i++) {
						spi_message_add_tail(&_ldev->StateX[_i], &_ldev->msg);
					}
					/// async write		
					spi_async(_ldev->spi, &_ldev->msg);
					break;

				case UART_ERROR:
					db_spi_uart(UART_ERROR,"ERROR");
					CT_SET_UART(UART_PUMP_CPL_REQ);
					spi_message_init(&_ldev->msg);
					_ldev->msg.complete = spi_completion_uart_pump; 
					_ldev->msg.context = (void*) data;
					/// SC16IS750-ERROR Message
					_ldev->uart_cmdo = _ldev->uart_error;
					_ldev->uart_cmdo[1] = 0x07;
					for(_i = 0; _i< sizeof(paa_spi_uart_error)/2; _i++) {
						spi_message_add_tail(&_ldev->ErrorX[_i], &_ldev->msg);
					}
					/// async write
					spi_async(_ldev->spi, &_ldev->msg);	
					break;
				};
				break;

			case e_idle:
				if(list_empty(&_mux->transfer_list)) {
					break;
				}
				else {
					// Nachrichten von Clients werden alle mit  PAA_MSG_OFFLINE beendet!
					spin_lock(&_mux->transfer_lock);
					list_for_each_entry_safe(_msg, _msg_tmp, &_mux->transfer_list, transfer_item) {
						_paa = _msg->paa;
						_msg->rx_typ = PAA_MSG_ERROR;
						_msg->rx_len = sizeof(LABEL_SLOT_OFFLINE)+1;
						memcpy(&_msg->rx_buf,LABEL_SLOT_OFFLINE,_msg->rx_len);
						_cpl = _msg->complete;
						_ctx = _msg->context;
						list_del(_mux->transfer_list.next);
						spin_unlock(&_mux->transfer_lock);
						_cpl(_ctx);
						spin_lock(&_mux->transfer_lock);
					}
					spin_unlock(&_mux->transfer_lock);
				}
				break;

			default:		// e_error, unknown 
				break;	
		};
		add_timer( &_mux->OT.timer); // Timer aufziehen
	} // end else
}

//---------------------------------------------------------------
//	spi-probe
//---------------------------------------------------------------
static int spi_probe(struct spi_device *spi)	{	
	struct lower_device * ct; 
#ifdef MCT_PAA_DEBUG
	dev_printk(KERN_INFO , &spi->dev , "%s()\n", __FUNCTION__ );
#endif
	ct = kzalloc(sizeof(struct lower_device), GFP_ATOMIC);
	if(ct==NULL) {						// kein Speicher
//		kfree(ct);
		printk("kzalloc faild!\n");
		dev_err(&spi->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	/// PRE-INIT UART-Messages
	// uart-ini-Message
	memcpy(&ct->uart_init, paa_spi_uart_init, sizeof(paa_spi_uart_init));
	paa_spi_uart_transfer_preinit(	ct->InitX,\
							ct->uart_init,\
							sizeof(paa_spi_uart_init),\
							ct->uart_cmdi);
	// uart-state-Message
	memcpy(&ct->uart_state, paa_spi_uart_state,sizeof(paa_spi_uart_state));
	paa_spi_uart_transfer_preinit(	ct->StateX,\
							ct->uart_state,\
							sizeof(paa_spi_uart_state),\
							ct->uart_cmdi);
	// uart-error-Message
	memcpy(&ct->uart_error, paa_spi_uart_error,sizeof(paa_spi_uart_error));
	paa_spi_uart_transfer_preinit(	ct->ErrorX,\
							ct->uart_error,\
							sizeof(paa_spi_uart_error),\
							ct->uart_cmdi);
	// uart-read-Message
	ct->RxX.tx_buf = &ct->dual_buffer[DUAL_RD_TX];
	ct->RxX.rx_buf = &ct->dual_buffer[DUAL_RD_RX];
	ct->dual_buffer[DUAL_RD_CMD] = RHR;
	ct->RxX.cs_change = 1;	
	// uart-write-Message
	ct->TxX.tx_buf = &ct->dual_buffer[DUAL_WR_TX];
	ct->TxX.rx_buf = &ct->dual_buffer[DUAL_WR_RX];	// dummy
	ct->dual_buffer[DUAL_WR_CMD] = THR;
	ct->TxX.cs_change = 1;	
	/// SPI-UART initialisieren
	atomic_set(&ct->uart,UART_INIT);		
	/// LINK spi->dev to lower_device and backlink
	spi_set_drvdata(spi,ct);
	ct->spi = spi;	
	/// LINK paa_mux to lower_device
	Op_PaaMux->ldev =  ct;
	return 0;
}; 
//---------------------------------------------------------------
//	spi-remove
//---------------------------------------------------------------
#ifdef MODULE
static int spi_remove(struct spi_device *spi)	{	
	struct lower_device *ct = dev_get_drvdata(&spi->dev);		
// Module-Device-Debugging
#ifdef MCT_PAA_DEBUG
	dev_printk(KERN_INFO , &spi->dev , "%s()\n", __FUNCTION__ );	
#endif
	kfree(ct);	
	return 0;
};
#endif

struct spi_driver spi_drv = {
	.driver = {
		.name = "sc16is740",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe 	= spi_probe,
	.remove = __exit_p(spi_remove),
};
