/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.12.2011

 	Description:  mct_spi_aio DRIVER (interne Hardware)
	
*********************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>		// local_bh_enable
#include <linux/spi/spi.h>
#include <asm/bitops.h>
#include <asm/atomic.h>

#include "../mct_json.h"		// json-support
#include "../mct_debug.h"		// debug-support
#include "mct_spi_aio_objs.h"
#include "mct_spi_aio_sysfs.h"
#include "mct_aio.h"

#ifdef MCT_SPI_AIO_DEV_IF_MBUS		
	#include "mct_spi_aio_mbus.h"	/// MBUS	
#endif

#ifdef MCT_SPI_AIO_DEV_IF_MODBUS		
	#include "mct_spi_aio_modbus.h"	/// MODBUS	
#endif

static DECLARE_COMPLETION( device_is_free );

#define PIC_TICKER		300 	// ( ca. 3 Sekunden!)

// kein sync-Schutz notwendig!
#define DR_GET_TYPE(chan,val)	val = box_get_sel(&aio_O_Inputs[chan].Op_Type->P_Type);
#define CT_SET_Ticker(val)	atomic_set(&_ct->pic_RcvTlgTicker, val);
#define CT_SET_UART(val) 	atomic_set(&_ct->uart,val );
#define CT_GET_UART(val) 	val = atomic_read(&_ct->uart);

//---------------------------------------------------------------
///	AIO-completion-spi-init
//---------------------------------------------------------------
void aio_completion_spi_uart_init (void * data)	{
	OBJS_CREATE_DRV_PTR(data);	
	aio_control * _ct	= drv->ct;
//	printk("UART_INIT_CPL:set\n");
	CT_SET_UART(UART_INIT_CPL_ACK);
};

//---------------------------------------------------------------
///	AIO-completion-spi-retrieve
//---------------------------------------------------------------
void aio_completion_spi_uart_retrieve (void * data)	{
	OBJS_CREATE_DRV_PTR(data);	
	aio_control *_ct = drv->ct;
	unsigned char uart_act_pattern[UART_PATTERN_SIZE];
	unsigned char _i;
	unsigned char _tries;
	uart_act_pattern[0] = _ct->cmdi_buffer[3];	// devisor-latch-low
	uart_act_pattern[1] = _ct->cmdi_buffer[5];	// devisor-latch high
	uart_act_pattern[2] = _ct->cmdi_buffer[9];	// enhanced feature register
	uart_act_pattern[3] = _ct->cmdi_buffer[13];	// scratchpad register
	uart_act_pattern[4] = _ct->cmdi_buffer[15];	// interrupt enable register
	uart_act_pattern[5] = _ct->cmdi_buffer[17];	// I/O pins control register
	uart_act_pattern[6] = _ct->cmdi_buffer[19];	// I/O interrupt enable register
	uart_act_pattern[7] = _ct->cmdi_buffer[21];	// I/O pin direction register
	
	for(_i = 0; _i < UART_PATTERN_SIZE; _i++) {
		if(uart_act_pattern[_i] != uart_reg_pattern[_i]) {
			OBJS_DRV_RD_ERROR(drv,ERROR_UART_TYPE_Retrieve, _tries);
			if(_tries < UART_RETRIEVE_MAX) {
				_tries++;
				printk(KERN_ERR "%s %s(): %s - try:%02d pattern[%02d] (%02x != %02x)\n",__FILE__, __FUNCTION__,"UART: Retrieve while Init!", _tries, _i, uart_reg_pattern[_i], uart_act_pattern[_i]);
				OBJS_DRV_WR_ERROR(drv,ERROR_UART_TYPE_Retrieve,_tries);	
				CT_SET_UART(UART_INIT);
				OBJS_DRV_WR_STATE(drv,e_start);  // neu Starten
			}
			else {
				printk(KERN_ERR "%s %s(): %s - tries:%02d\n",__FILE__, __FUNCTION__,"UART: Retrieve while Init BREAK!", _tries);
				CT_SET_UART(UART_INIT);
				OBJS_DRV_WR_STATE(drv,e_stop); 	// endgültig Stoppen!
			}
			return;						// Raus wenn Retrieve mit Fehler abgeschlossen wurde!
		}
	}
//	printk("UART_RETRIEVE_CPL_ACK:set\n");	
	CT_SET_UART(UART_RETRIEVE_CPL_ACK);
};

//---------------------------------------------------------------
///	AIO-completion-spi-state
//---------------------------------------------------------------
void aio_completion_spi_uart_pump (void * data)	{
	OBJS_CREATE_DRV_PTR(data);	
	OBJS_CREATE_IDEV_PTR_NULL;
	aio_control * 	_ct	= drv->ct;	
	u8		_reg_lsr;
	u8		_reg_rxlvl;
	u8		_reg_txlvl;
	u8		_read;
	u8		_inchar;
	u8		_kanal = 0;
	u32		_input32;
	u8		_pic_version = 0;
//	_reg_ios 	= _ct->cmdi_buffer[5];	// Hand-Register lesen
//	_reg_ios	= _reg_ios ^ IODIR_OUT_MASK;	// invertierte LED's
	_reg_lsr 	= _ct->cmdi_buffer[7];	// line state
	_reg_txlvl 	= _ct->cmdi_buffer[11];	// transmit level
	_reg_rxlvl 	= _ct->cmdi_buffer[9];	// receive level

	// ***********************************
	// LSR-ERROR
	// ***********************************
	if(_reg_lsr & LSR_ERR_FLAGS)	{
		DEBUG(MCT_DEBUG_LEVEL3,"SPI-UART: Error LSR %x Register Reset!\n",_reg_lsr);
		OBJS_DRV_WR_ERROR(drv, ERROR_UART_TYPE_State ,(_reg_lsr & LSR_ERR_FLAGS));
		CT_SET_UART(UART_ERROR);
		return;
	}
	// ***********************************
	// LSR-DATA-READ
	// *********************************** 
	_read = 0;
	while(_ct->pic.rec.signs)	{ 			// sind Zeichen in der Lese-Fifo?
		_inchar = _ct->dual_buffer[DUAL_RD_DATA + _read];
//		printk("%x-",_inchar);
		_ct->pic.rec.signs--;
 		_read = _read+1;
		switch(_ct->pic.rec.state)	{
			case PIC_REC_IDLE:			// Start-Frame-Kennung erwartet
				if(_inchar & 0x80)	{		
				_ct->pic.rec.ist = 0;
				if( (~_inchar) & 0x40)	{	// Messwert-Frame
					_ct->pic.rec.soll 	= 6;
					_ct->pic.rec.ist 	= 1;
					_ct->pic.rec.kanal 	= (_inchar >> 4) & 3;
					_ct->pic.rec.mode 	= _inchar & 0x0F;
					_ct->pic.rec.state 	= PIC_REC_GET_MESSWERT;
					break;
				}
				if( (_inchar & 0xFE) == 0xF0)	{	// Version-Frame
					_ct->pic.rec.soll	= 2;
					_ct->pic.rec.ist	= 1;
					_pic_version = (_inchar & 0x01) << 7;
					_ct->pic.rec.state	= PIC_REC_VERSION;
					break;
				}
				if( (_inchar & 0xFE) == 0xFE)	{	// Abgleich-Frame
					_ct->pic.rec.soll	= 7;
					_ct->pic.rec.ist	= 1;
					_ct->pic.rec.state	= PIC_REC_ABGLEICH;
					break;
				}

				if( (_inchar & 0xFE) == 0xC0) {		// Life-Frame
					_ct->pic.rec.soll	= 1;
					_ct->pic.rec.ist	= 1;
					_ct->pic.rec.state 	= PIC_REC_IDLE;
					CT_SET_Ticker(PIC_TICKER);	
					if(_inchar & 0x01) {
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: PIC2-ACK???\n");
						OBJS_DRV_WR_ERROR(drv, ERROR_UART_TYPE_Pic ,_inchar);
					}
					else {
						// DEBUG(MCT_DEBUG_LEVEL3,"PIC: AA-ACK\n");
					}
					break;
				}
				OBJS_DRV_INC_ERROR(drv,ERROR_UART_TYPE_Frame);
				DEBUG(MCT_DEBUG_LEVEL3,"PIC: Frame Type???\n");
			}
//			printk("0x%2.2x",_inchar );
			break;

			case PIC_REC_GET_MESSWERT:		// Messwert erwartet	
				_ct->pic.rec.ist++;
				switch(_ct->pic.rec.ist)	{
				case 2:	_ct->pic.rec.ain = (_inchar & 0x0F) << 28;
						break;
				case 3:	_ct->pic.rec.ain = _ct->pic.rec.ain | (_inchar & 0x7F) << 21;
						break;
				case 4:	_ct->pic.rec.ain = _ct->pic.rec.ain | (_inchar & 0x7F) << 14;
						break;
				case 5:	_ct->pic.rec.ain = _ct->pic.rec.ain | (_inchar & 0x7F) << 7;
						break;
				case 6:	_ct->pic.rec.ain = _ct->pic.rec.ain | (_inchar & 0x7F);
/*						printk("%d %d %x\n",\
						_ct->pic.rec.kanal,	\
						_ct->pic.rec.mode,	\
						_ct->pic.rec.ain);
						*/
						///*******************
						/// PROTECT ACCESS
						///*******************
						_kanal = _ct->pic.rec.kanal;
						_input32 = _ct->pic.rec.ain;
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Inp-Value[%i]:%x\n",_kanal,_input32);	
						OBJS_CREATE_IDEV_PTR(_kanal);
						OBJS_IDEV_WR_INPUT(idev,_input32);	/// EXPORT Inputs
						CT_SET_Ticker(PIC_TICKER);	
						_ct->pic.rec.state = PIC_REC_IDLE;
						break;

				default: 
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Messwert???\n");
						_ct->pic.rec.state = PIC_REC_IDLE;
						break;
				}	
				break;

			case PIC_REC_VERSION:
					_ct->pic.rec.ist++;
					if(_ct->pic.rec.ist == 2)	{
						_pic_version = _inchar & 0xEF;
						_ct->pic.rec.state = PIC_REC_IDLE;
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Version %d.%d found!\n", (_pic_version >> 4), _pic_version & 0x0F);
						OBJS_DRV_WR_PIC_VERSION(drv,_pic_version); /// EXPORT PIC-VERSION	
						CT_SET_Ticker(PIC_TICKER);	
						_ct->pic.snd.state = PIC_SND_ABGLEICH_0;
					}
					else	{
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Version???\n");
					}
					_ct->pic.rec.state = PIC_REC_IDLE;
					break;	

			case PIC_REC_ABGLEICH:
				_ct->pic.rec.ist++;
				switch(_ct->pic.rec.ist)	{
					case 2:
						if(_inchar == 64) {
							_ct->pic.snd.state = PIC_SND_ABGLEICH_1;
							_ct->pic.rec.aia_kanal = 0;
							break;
						}
						if(_inchar == 65) {
							_ct->pic.snd.state = PIC_SND_CONFIG;
							_ct->pic.rec.aia_kanal = 1;
							break;
						}
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Abgleich - U/R-Input[???]\n");
						_ct->pic.rec.state = PIC_REC_IDLE;
						break;
					case 3:	_ct->pic.rec.aia = 0;
							_ct->pic.rec.aia = (_inchar & 0x0F) << 28; break;
					case 4:	_ct->pic.rec.aia = _ct->pic.rec.aia | (_inchar & 0x7F) << 21; break;
					case 5:	_ct->pic.rec.aia = _ct->pic.rec.aia | (_inchar & 0x7F) << 14; break;
					case 6:	_ct->pic.rec.aia = _ct->pic.rec.aia | (_inchar & 0x7F) << 7; break;
					case 7:	_ct->pic.rec.aia = _ct->pic.rec.aia | (_inchar & 0x7F);
						///*******************
						/// PROTECT ACCESS
						///*******************
						_input32 = _ct->pic.rec.aia;
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Abgleich - U/R-Input[%d]:%x \n", _ct->pic.rec.aia_kanal,_input32);
						OBJS_CREATE_IDEV_PTR(_ct->pic.rec.aia_kanal);
						OBJS_IDEV_WR_OFFSET(idev,_input32);	/// EXPORT Offset
						CT_SET_Ticker(PIC_TICKER);							
						_ct->pic.rec.state = PIC_REC_IDLE;
						break;

					default:
						DEBUG(MCT_DEBUG_LEVEL3,"PIC: Abgleich - U/R-Input[%d]:???\n",_ct->pic.rec.aia_kanal);
						_ct->pic.rec.state = PIC_REC_IDLE;
						break;
				}	
				break;
		}
	}
	// schon wieder neue Zeichen in Lese-FiFo?	
	_ct->pic.rec.signs = _reg_rxlvl;
	// ***********************************
	// LSR-DATA-WRITE
	// *********************************** 
	_ct->pic.snd.pending = TX_FIFO_SIZE - _reg_txlvl;
	_ct->pic.snd.write = 0;
	CT_SET_UART(UART_RUN);
}

//---------------------------------------------------------------
///	AIO-Timer
//---------------------------------------------------------------
static void aio_timer_function(unsigned long data) {
	OBJS_CREATE_DRV_PTR(data);	
	OBJS_CREATE_ODEV_PTR_NULL;
	aio_control *_ct = drv->ct;
	enum e_state state;
	int _uart;	
	u8	_i = 0;
	int	_out16 = 0;
	u8	_type =0;
	u16	_use;

	_ct->timer.expires = jiffies + TIMER_SPI_GRANULARITY;
	if(atomic_read( &_ct->timer_stop_job ) ) {
		complete( &_ct->timer_stop_quit ); 		// final spi timer call
	} else {
		// ***************************
		// DRIVER-STATE 	
		// ***************************	
		OBJS_DRV_RD_STATE(drv,state);			// driver-status
		CT_GET_UART(_uart);				// uart-status	
		switch(state)	{
			case e_stop: 				// Stoppen!
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver stopped!\n", __FILE__,__FUNCTION__);
				CT_SET_UART(UART_INIT);
				OBJS_DRV_WR_STATE(drv,e_idle);	// Driver.State geht in den Idle-Mode
				break;				
				
			case e_reset:	
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver reset!\n",__FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_start);	// Driver.State geht in den Start-Mode
			case e_start:
			switch(_uart) {
				case UART_INIT:
					DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): driver started!\n", __FILE__,__FUNCTION__);
//					printk("UART_INIT\n");
					CT_SET_UART(UART_INIT_CPL_REQ);
					spi_message_init(&_ct->msg);
					_ct->msg.complete = aio_completion_spi_uart_init; 
					_ct->msg.context = (void*) data;	
					/// SC16IS750-INIT	Message
					memcpy(&_ct->cmdo_buffer, uart_tlg_init,sizeof(uart_tlg_init));
					for(_i = 0; _i< sizeof(uart_tlg_init)/2; _i++) {
						spi_message_add_tail(&_ct->InitX[_i], &_ct->msg);
					}
					/// async write	
					spi_async(_ct->spi, &_ct->msg);
					break;
				
				case UART_INIT_CPL_ACK:
//					printk("UART_INIT_CPL_ACK:get\n");
					CT_SET_UART(UART_RETRIEVE_CPL_REQ);
					spi_message_init(&_ct->msg);
					_ct->msg.complete = aio_completion_spi_uart_retrieve; 
					_ct->msg.context = (void*) data;	
					/// SC16IS750-RETRIEVE	Message
					memcpy(&_ct->cmdo_buffer, uart_tlg_retrieve,sizeof(uart_tlg_retrieve));
					for(_i = 0; _i< sizeof(uart_tlg_retrieve)/2; _i++) {
						spi_message_add_tail(&_ct->RetrieveX[_i], &_ct->msg);
					}
					/// async write	
					spi_async(_ct->spi, &_ct->msg);										
					break;

				case UART_RETRIEVE_CPL_ACK:
//					printk("UART_RETRIEVE_CPL_ACK:get\n");
					CT_SET_UART(UART_PUMP);	
					/// SETUP PIC
					memset(&_ct->pic,0,sizeof(struct t_pic));
					CT_SET_Ticker(PIC_TICKER);
					OBJS_DRV_WR_STATE(drv,e_running); 
					break;

				default:
					break;
				};
				break;
			
			case e_running:
				switch(_uart)	{
					case UART_PUMP:
	//					printk("UART_PUMP\n");
						CT_SET_UART(UART_PUMP_CPL_REQ);	
						spi_message_init(&_ct->msg);
						_ct->msg.complete = aio_completion_spi_uart_pump; 
						_ct->msg.context = (void*) data;
						/// READ JOB ?
						if(_ct->pic.rec.signs)	{
							//printk("rd  %x\n",_ct->read);
							_ct->RxX.len = _ct->pic.rec.signs+1; 
							spi_message_add_tail(&_ct->RxX, &_ct->msg);
						}
						/// WRITE JOB
						if(_ct->pic.snd.write)	{
							_ct->TxX.len = _ct->pic.snd.write+1;
							spi_message_add_tail(&_ct->TxX, &_ct->msg);
						}
						/// SC16IS750-STATE Message
						memcpy(&_ct->cmdo_buffer, uart_tlg_state,sizeof(uart_tlg_state));
						for(_i = 0; _i< sizeof(uart_tlg_state)/2; _i++) {
							spi_message_add_tail(&_ct->StateX[_i], &_ct->msg);
						}
						/// async write		
						spi_async(_ct->spi, &_ct->msg);	
					break;

					/// WORKING PARTs	
					case UART_RUN:
						//printk("UART_RUN\n");
						if(!_ct->pic.snd.write) {
							/// KEEP-LIFE?
							if(atomic_dec_and_test(&_ct->pic_RcvTlgTicker))	{
								switch(_ct->pic.snd.state)	{
									case PIC_SND_VERSION:
										printk(KERN_ERR "%s %s(): %s",__FILE__, __FUNCTION__,"PIC: Timeout on version!\n");
										OBJS_DRV_WR_ERROR(drv,ERROR_UART_TYPE_Timeout,PIC_TIMEOUT_ON_VERSION);
										break;
									case PIC_SND_ABGLEICH:
										printk(KERN_ERR "%s %s(): %s",__FILE__, __FUNCTION__,"PIC: Timeout on adjust!!\n");
										OBJS_DRV_WR_ERROR(drv,ERROR_UART_TYPE_Timeout ,PIC_TIMEOUT_ON_ABGLEICH);
										break;
									case PIC_SND_CONFIG:
										printk(KERN_ERR "%s %s(): %s",__FILE__, __FUNCTION__,"PIC: Timeout on config!\n");
										OBJS_DRV_WR_ERROR(drv,ERROR_UART_TYPE_Timeout ,PIC_TIMEOUT_ON_CONFIG);
										break;
									case PIC_SND_SET_MESSWERT:
										printk(KERN_ERR "%s %s(): %s",__FILE__, __FUNCTION__,"PIC: Timeout on measure!\n");
										OBJS_DRV_WR_ERROR(drv,ERROR_UART_TYPE_Timeout,PIC_TIMEOUT_ON_MEASURE);
										break;
							}
							CT_SET_UART(UART_INIT);
							OBJS_DRV_WR_STATE(drv,e_reset);
							break;
							}
						}

						if(!_ct->pic.snd.write) {
							switch(_ct->pic.snd.state)	{
								case PIC_SND_IDLE:
									/// PIC-Version anfragen
									_ct->pic.snd.write = 1;	
									_ct->pic.snd.cmd[0] = 0xF0;
									memcpy(	&_ct->dual_buffer[DUAL_WR_DATA],\
											&_ct->pic.snd.cmd,_ct->pic.snd.write);
									_ct->pic.snd.state = PIC_SND_VERSION;
								break;

								case PIC_SND_ABGLEICH_0:
									/// Abgleich-Wert 0 anfragen
									_ct->pic.snd.write = 2;	
									_ct->pic.snd.cmd[0] = 0xFE;
									_ct->pic.snd.cmd[1] = 64;
									memcpy(	&_ct->dual_buffer[DUAL_WR_DATA],\
											&_ct->pic.snd.cmd,_ct->pic.snd.write);
									_ct->pic.snd.state = PIC_SND_ABGLEICH;
								break;

								case PIC_SND_ABGLEICH_1:
									/// Abgleich-Wert 0 anfragen
									_ct->pic.snd.write = 2;	
									_ct->pic.snd.cmd[0] = 0xFE;
									_ct->pic.snd.cmd[1] = 65;
									memcpy(	&_ct->dual_buffer[DUAL_WR_DATA],\
											&_ct->pic.snd.cmd,_ct->pic.snd.write);
									_ct->pic.snd.state = PIC_SND_ABGLEICH;
								break;

								case PIC_SND_CONFIG:
									/// INPUT: A/D-Messbereich festlegen
									for(_i=0; _i < DEVICE_AIO_IN_MAX; _i++)	{
										DR_GET_TYPE(_i,_type );
										DEBUG(MCT_DEBUG_LEVEL3,"PIC: Cfg Inp-Mode[%i]:%d\n",_i,_type);
										_ct->pic.snd.cmd[_i] = 0x80;			// Kommando	
										_ct->pic.snd.cmd[_i] |= (_i & 3) << 4;	// Kanal
										_ct->pic.snd.cmd[_i] |= _type & 0x0F;	// Type-Messbereich
									}
									/// OUTPUT: 10bit D/A Initialwert rausschreiben!
									for(_i=0; _i < DEVICE_AIO_OUT_MAX; _i++)	{
										OBJS_CREATE_ODEV_PTR(_i);
										OBJS_ODEV_RD_OUTPUT(odev,_out16);	
										DEBUG(MCT_DEBUG_LEVEL3,"PIC: Cfg Outp-Value[%i]:%d\n",_i,_out16);
										_ct->pic.snd.aou[_i] = _out16;			// Cachen
										_ct->pic.snd.cmd[4+(3*_i)] = 0xC0;		// Kommando
										_ct->pic.snd.cmd[4+(3*_i)] |= (_i & 3) << 2;	// Kanal
										if(_i < DEVICE_AIO_OUT_12_MAX)	{
										  /// 10 bit Auflösung (Spannung)
										  _ct->pic.snd.cmd[4+(3*_i)] |= (_out16 >>9) & 0x01; 
										  _ct->pic.snd.cmd[5+(3*_i)]  = (_out16 >>2) & 0x7F;
										  _ct->pic.snd.cmd[6+(3*_i)]  = (_out16 <<5 )& 0x60;
										}	
										else {	
										  /// 11 bit Auflösung (Strom)
										  _ct->pic.snd.cmd[4+(3*_i)] |= (_out16 >>10) & 0x01; 
										  _ct->pic.snd.cmd[5+(3*_i)]  = (_out16 >>3) & 0x7F;
										  _ct->pic.snd.cmd[6+(3*_i)]  = (_out16 <<4 )& 0x70;
										}
									}
									// Länge des PIC-Initialisierungstelegramms
									_ct->pic.snd.write =  4 + 3*DEVICE_AIO_OUT_MAX;
									memcpy(	&_ct->dual_buffer[DUAL_WR_DATA],\
											&_ct->pic.snd.cmd,\
											_ct->pic.snd.write);
									_ct->pic.snd.state = PIC_SND_SET_MESSWERT;
								break;	
	
								case PIC_SND_SET_MESSWERT:
									/// IMPORT: OUTPUTs 
									for(_i=0, _use=0; _i < DEVICE_AIO_OUT_MAX; _i++)	{
										OBJS_CREATE_ODEV_PTR(_i);
										OBJS_ODEV_RD_OUTPUT(odev,_out16);
										if(_ct->pic.snd.aou[_i] != _out16) {
											DEBUG(MCT_DEBUG_LEVEL3,"PIC: Outp-Value[%i]:%d \n",_i,_out16);
											_ct->pic.snd.aou[_i] = _out16;			// Cache
											_ct->pic.snd.cmd[0+(3*_use)] = 0xC0;		// Kommando
											_ct->pic.snd.cmd[0+(3*_use)] |= (_i & 3) << 2;	// Kanal
											if(_i < DEVICE_AIO_OUT_12_MAX)	{
												/// 10 bit Auflösung (Spannung)
												DEBUG(MCT_DEBUG_LEVEL3,"PIC: 10 bit\n");
												_ct->pic.snd.cmd[0+(3*_use)] |= (_out16 >>9) & 0x01; 
												_ct->pic.snd.cmd[1+(3*_use)]  = (_out16 >>2) & 0x7F;	
												_ct->pic.snd.cmd[2+(3*_use)]  = (_out16 <<5) & 0x60;
											}	
											else {	
												/// 11 bit Auflösung (Strom)
												DEBUG(MCT_DEBUG_LEVEL3,"PIC: 11 bit\n");
												_ct->pic.snd.cmd[0+(3*_use)] |= (_out16 >>10) & 0x01; 
												_ct->pic.snd.cmd[1+(3*_use)]  = (_out16 >>3) & 0x7F;	
												_ct->pic.snd.cmd[2+(3*_use)]  = (_out16 <<4) & 0x70;	
											}
											_use++;
										}
										// Länge des PIC-Initialisierungstelegramms
										_ct->pic.snd.write =  3*_use;
										memcpy(	&_ct->dual_buffer[DUAL_WR_DATA],\
												&_ct->pic.snd.cmd,\
												_ct->pic.snd.write);
									}
									break;	
							}						
						}
						CT_SET_UART(UART_PUMP);
					break;

					case UART_ERROR:
	//					printk("UART_ERROR\n");				
						CT_SET_UART(UART_PUMP_CPL_REQ);
						spi_message_init(&_ct->msg);
						_ct->msg.complete = aio_completion_spi_uart_pump; 
						_ct->msg.context = (void*) data;
						/// SC16IS750-ERROR Message
						memcpy(&_ct->cmdo_buffer, uart_tlg_error,sizeof(uart_tlg_error));
						_ct->cmdo_buffer[1] = 0x07;				// Reset FiFo RX-TX
						for(_i = 0; _i< sizeof(uart_tlg_error)/2; _i++) {
							spi_message_add_tail(&_ct->ErrorX[_i], &_ct->msg);
						}
						/// async write
						spi_async(_ct->spi, &_ct->msg);	
						_ct->pic.snd.state = PIC_SND_SET_MESSWERT;	// Fix: 06.07.2010 Messwerterfassung fortsetzen
						break;
				};				
				break;

			default:		// e_idle e_error 
				break;	
		};
		add_timer( &_ct->timer); // Timer aufziehen
	} // end else
};

//---------------------------------------------------------------
/// SECTION: SPI-UART-Driver (Input and Output)
//---------------------------------------------------------------
//---------------------------------------------------------------
//	Uart-Probe
//---------------------------------------------------------------
static int uart_probe(struct spi_device *spi)	{
	aio_control * ct;
	trace_call_dev(&spi->dev);
	ct = kzalloc(sizeof(aio_control), GFP_KERNEL);
	if(!ct) {						// kein Speicher
		kfree(ct);
		dev_err(&spi->dev,"%s\n","kzalloc faild!");
		return -ENOMEM;
	} 
	DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): SPI Hz:%d Mode:%d Alias:%s\n",__FILE__, __FUNCTION__, spi->max_speed_hz,spi->mode,spi->modalias);

	/// PRE-INIT UART-Messages
	// uart-ini-Message
	uart_transfer_preinit(	ct->InitX,\
						ct->cmdo_buffer,\
						ct->cmdi_buffer,\
						uart_tlg_init,\
						sizeof(uart_tlg_init));

	// uart-retrieve-Message
	uart_transfer_preinit(	ct->RetrieveX,\
						ct->cmdo_buffer,\
						ct->cmdi_buffer,\
						uart_tlg_retrieve,\
						sizeof(uart_tlg_retrieve));

	// uart-state-Message
	uart_transfer_preinit(	ct->StateX,\
						ct->cmdo_buffer,\
						ct->cmdi_buffer,\
						uart_tlg_state,\
						sizeof(uart_tlg_state));

	// uart-error-Message
	uart_transfer_preinit(	ct->ErrorX,\
						ct->cmdo_buffer,\
						ct->cmdi_buffer,\
						uart_tlg_error,\
						sizeof(uart_tlg_error));

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
	
	/// LINK spi->dev to aio_control
	dev_set_drvdata(&spi->dev,ct);	 			// bereich merken
	ct->spi = spi;	
	
	atomic_set(&ct->uart,UART_INIT);		
	
	spin_lock_init(&ct->protect);

	/// SPI-TIMER-INIT
	init_completion(&ct->timer_stop_quit);		// completion  object
	init_timer(&ct->timer);						// timer-initialisieren
	aio_O_Mct.ct = ct;
	ct->timer.data = (unsigned long) &aio_O_Mct;
	ct->timer.function = aio_timer_function;
	ct->timer.expires = jiffies + TIMER_SPI_GRANULARITY;
	add_timer( &ct->timer);						// timer-start
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer added!\n",__FILE__, __FUNCTION__);
	return 0;	
}; 

//---------------------------------------------------------------
//	Uart-Remove
//---------------------------------------------------------------
#ifdef MODULE
static  int  uart_remove(struct spi_device *spi)	{	
	aio_control *ct = dev_get_drvdata(&spi->dev);	
	trace_call_dev(&spi->dev);
	/// SPI-TIMER-STOP
	atomic_set( &ct->timer_stop_job, 1);		// Timer nicht mehr einhängen
	wait_for_completion(&ct->timer_stop_quit);	
	if( timer_pending( &ct->timer ) ) {
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer active!\n", __FILE__,__FUNCTION__);
	}
	if( del_timer_sync( &ct->timer ) )	{
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer not active!\n", __FILE__,__FUNCTION__);
	}
	else	{
		DEBUG(MCT_DEBUG_LEVEL1, "%s %s(): timer stop!\n", __FILE__,__FUNCTION__);
	}	
	kfree(ct);	
	return 0;
};
#endif

//---------------------------------------------------------------
///	Uart Driver (spi)
//---------------------------------------------------------------
static struct spi_driver uart_drv = {
	.driver = {
		.name = "sc16is750",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe 	= uart_probe,
	.remove = __exit_p (uart_remove),
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: AIO-DRIVER (platform) with 2 DEVICES  (platform) 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AIO-DRIVER 
static struct platform_driver aio_drv = {
		.driver = {		
		.name = DRIVER_NAME,		// ... siehe oben, per define! 
		.bus = &platform_bus_type, 	//sys/bus/platform/drivers/DRIVER_NAME
	},					// ... /sys/bus/platform/devices/DEVICE_NAME.Index
};
// OUTPUT-DEVICE-PTR (platform-device)
static struct platform_device * outputs[DEVICE_AIO_OUT_MAX]; 
// INPUT-DEVICE-PTR (platform-device)
static struct platform_device * inputs[DEVICE_AIO_IN_MAX]; 

static void ao_dev_release(struct device * dev)	{
	complete(&device_is_free);
}
static int  ao_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err = 0;
	int idx;

	for (idx = 0; idx< DEVICE_AIO_OUT_MAX; idx++)	{	
		//	platform_set_drvdata(pdev,obj);
		outputs[idx] = NULL;
		if( idx < DEVICE_AIO_OUT_12_MAX)
			outputs[idx] = platform_device_alloc(DEVICE_OUT_U_NAME,idx);
		else
			outputs[idx] = platform_device_alloc(DEVICE_OUT_I_NAME,idx-DEVICE_AIO_OUT_12_MAX);

		if(outputs[idx]) {
			outputs[idx]->dev.release = ao_dev_release;		// Release-Function
			platform_device_add(outputs[idx]);
			platform_set_drvdata(outputs[idx], &aio_O_Outputs[idx]); 	// AO-Platform-Device den Datenzeiger mitgeben
			outputs[idx]->dev.driver = &drv->driver;
			lnk_err = device_bind_driver(&outputs[idx]->dev);			
			aio_ao_sysfs_attr_add(&outputs[idx]->dev);			// AO-Devices-ATTR erzeugen			
		}
		else	{
			// Resource-Meldung!
			if( idx < DEVICE_AIO_OUT_12_MAX)
				printk(KERN_ERR "%s %s: %s" DEVICE_OUT_U_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource ",idx);
			else
				printk(KERN_ERR "%s %s: %s" DEVICE_OUT_I_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource ",idx);

			ret = -ENOMEM;
		}
		if(ret)	{
			if( outputs[idx]) {	
				platform_device_unregister(outputs[idx]);	// AO-Platform-Device entfernen
				aio_ao_sysfs_attr_del(&outputs[idx]->dev);	// AO-Devices-ATTR entfernen
			}
		}
	}	// end for DEVICE_OUT_MAX
	return ret;
}

static void  ao_devs_destroy(void)	{
	int idx;
	for (idx = 0; idx< DEVICE_AIO_OUT_MAX; idx++)	{
		if(outputs[idx])	{	
			platform_set_drvdata(outputs[idx],NULL);	// AO-Platform-Device Datenzeiger aufheben	
			platform_device_unregister(outputs[idx]);	// AO-Platform-Device entfernen
			aio_ao_sysfs_attr_del(&outputs[idx]->dev);	// AO-Devices-ATTR entfernen 
		}
	}// end for DEVICE_OUT_MAX
}

static void ai_dev_release(struct device * dev)	{
	complete(&device_is_free);
}

static int  ai_devs_build(struct platform_driver * drv)	{
	int ret = 0;
	int lnk_err	= 0;
	int idx;
	for (idx = 0; idx< DEVICE_AIO_IN_MAX; idx++)	{
		inputs[idx]  = platform_device_alloc(DEVICE_IN_NAME,idx); 			
		if(inputs[idx]) {
			inputs[idx]->dev.release = ai_dev_release;				// Release-Function
			platform_device_add(inputs[idx]);
			platform_set_drvdata(inputs[idx], &aio_O_Inputs[idx]);	// AI-Platform Device den Datenzeiger mitgeben
			inputs[idx]->dev.driver = &drv->driver;
			lnk_err = device_bind_driver(&inputs[idx]->dev);			
			aio_ai_sysfs_attr_add(&inputs[idx]->dev);				// AI-Devices-ATTR erzeugen			
		}
		else	{
			// Resource-Meldung!
			printk(KERN_ERR "%s %s(): %s" DEVICE_IN_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource",idx);
			ret = -ENOMEM;
		}
		if(ret)	{
			if( inputs[idx]) {	
				platform_set_drvdata(inputs[idx],NULL);	// AI-Platform-Device Datenzeiger aufheben	
				platform_device_unregister(inputs[idx]);// AI-Platform-Device entfernen
				aio_ai_sysfs_attr_del(&inputs[idx]->dev);// AI-Devices-ATTR entfernen
			}
		}
	}// end for DEVICE_IN_MAX
	return ret;
}
static void  ai_devs_destroy(void)	{
	int idx;
	for (idx = 0; idx< DEVICE_AIO_IN_MAX; idx++)	{
		if(inputs[idx])	{	
			platform_set_drvdata(inputs[idx],NULL);		// AI-Platform-Device Datenzeiger aufheben			
			platform_device_unregister(inputs[idx]);	// AI-Platform-Device entfernen
			aio_ai_sysfs_attr_del(&inputs[idx]->dev);	// AI-Devices-ATTR entfernen 
		}
	} // end for DEVICE_IN_MAX
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///	SECTION: MODUL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int __init aio_init(void)	{
	int ret = 0;	
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	
	ret = platform_driver_register(&aio_drv);// AIO-Platformtreiber anmelden ...
	if(ret)	{ 								// Error-Message 1
		goto RESOURCE_RWND_NULL;	
	}
	aio_sysfs_attr_add(&aio_drv.driver);	// AIO-Driver-ATTR anlegen
	ret = 	ao_devs_build(&aio_drv);		// AO-Platform-Devices bauen
	if(ret) {								// Error-Message 2
		goto RESOURCE_RWND_DRIVER_SYSFS_PART;
	}
	ret = 	ai_devs_build(&aio_drv);		// AI-Platform-Devices bauen
	if(ret) {								// Error-Message 3
		goto RESOURCE_RWND_DRIVER_DODEVS_PART;
	}

	if(strlen(aio_json) != 0)				// modprobe driver json={....}
		aio_objects_json(aio_json);			// ...JSON-Parameter
	aio_objects_generic();					// ...Classic-Parameter				

#ifdef	MCT_SPI_AIO_DEV_IF_MBUS
	mbus_build(&aio_drv,0);					/// MBUS, Single-Instance
#endif	
#ifdef	MCT_SPI_AIO_DEV_IF_MODBUS
	modbus_build(&aio_drv,0);				/// MODBUS,Single-Instanz
#endif

	ret = spi_register_driver(&uart_drv);	// UART-SPI-DRIVER anmelden ...
	if(ret)	{								// Error-Message 3
		goto RESOURCE_RWND_OUTPUT_DEVICES_PART;
	}										// alle Resourcen erhalten!
	goto RESOURCE_OKAY;				
											// ***** REWIND on ERROR ************* 
RESOURCE_RWND_OUTPUT_DEVICES_PART:			// FEHLER: UART-SPI-Driver registrieren 
#ifdef	MCT_SPI_AIO_DEV_IF_MBUS	
	mbus_destroy();							/// MBUS
#endif
#ifdef	MCT_SPI_AIO_DEV_IF_MODBUS	
	modbus_destroy();						/// MODBUS
#endif
	ai_devs_destroy();						// AI-Platform-Devices entfernen!
RESOURCE_RWND_DRIVER_DODEVS_PART:			// FEHLER: AI-Platform-Devices bauen	
	ao_devs_destroy();						// AO-Platform-Devices entfernen!
RESOURCE_RWND_DRIVER_SYSFS_PART:			// FEHLER: AO-Platform-Devices bauen
	aio_sysfs_attr_del(&aio_drv.driver);	// AIO-Driver-ATTR entfernen
	platform_driver_unregister(&aio_drv);	// AIO-Platform-Driver abmelden
RESOURCE_RWND_NULL:							// FEHLER: AIO-Platformtreiber anmelden
	// Resource-Meldung!
	printk(KERN_ERR "%s %s(): %s" DRIVER_NAME ".%d error!\n",__FILE__, __FUNCTION__,"Resource",0);
	ret = -EIO;
RESOURCE_OKAY:
	return ret;
};

static void __exit aio_exit(void)	{		
	unsigned int idx;
	DEBUG(MCT_DEBUG_LEVEL1, "%s %s()\n",__FILE__, __FUNCTION__);
	spi_unregister_driver(&uart_drv);		// UART-SPI-Treiber abmelden
#ifdef	MCT_SPI_AIO_DEV_IF_MBUS	
	mbus_destroy();							/// MBUS
#endif
#ifdef	MCT_SPI_AIO_DEV_IF_MODBUS	
	modbus_destroy();						/// MODBUS
#endif
	ai_devs_destroy();						// AI-Platform-Devices entfernen	
	ao_devs_destroy();						// AO-Platform-Devices entfernen
	aio_sysfs_attr_del(&aio_drv.driver);	// AIO-Driver-ATTR entfernen
	platform_driver_unregister(&aio_drv);	// AIO-Platform-Driver abmelden ...	
	for(idx = 0; idx < DEVICE_AIO_IN_MAX + DEVICE_AIO_OUT_MAX; idx++) // warten ...
		wait_for_completion(&device_is_free);	
};

module_init( aio_init );
module_exit( aio_exit );

// Metainformations ...
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MC Technology GmbH, Dipl.-Ing. Steffen Kutsche");
MODULE_DESCRIPTION("SPI analog input/output driver (uart)");

