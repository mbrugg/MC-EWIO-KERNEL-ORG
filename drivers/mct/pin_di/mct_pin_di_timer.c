/*********************************************************************************

 Copyright MC-Technology GmbH 2009
  
 Autor:	Dipl.-Ing. Steffen Kutsche		

Hinweis:	
	MCT_PIN_DI_TIMER_TRACE - siehe Makefile
	Fehlertrace ist immer aktiv und nicht abschaltbar!

*********************************************************************************/
#include <linux/spinlock.h>
#include <linux/interrupt.h>	// local_bh_enable
#include <mach/board.h>
#include <mach/gpio.h>

#include "mct_pin_di_objs.h"		// objects
#include "../mct_debug.h"
#include "mct_di.h"

///*********
/// INTERCOM
///*********
// externe server-functions
/// ClientX setzt im Server das Link-Bit, das Init-Bit
extern void icom_cli_set_svr_link (unsigned char client, unsigned char link);	
extern void icom_cli_set_svr_init (unsigned char client, unsigned char init);	
// temporär
static void	(* icom_fkt)  (unsigned char client, unsigned char paramx);
extern unsigned char di_slot;

void icom_cli_set_link(unsigned char client, unsigned char link) {
	icom_fkt = symbol_get( icom_cli_set_svr_link );
	if(icom_fkt) {
		icom_fkt(client,link);
		symbol_put( icom_cli_set_svr_link );			// ende
	}
}

void icom_cli_set_init(unsigned char client, unsigned char init) {
	icom_fkt = symbol_get( icom_cli_set_svr_init );
	if(icom_fkt) {
		icom_fkt(client,init);
		symbol_put( icom_cli_set_svr_init );			// ende
	}
}


static struct t_counter {	// S0-COUNTER
//	u64 counter;			// aktueller Zählerstand
	u8 h_sample;
	u8 l_sample;
	u8 h_end;
	u8 l_end;
	u8 foul;				// Fehlerspeicher bei Abtasung
//	u64 param_1;			// Grenzzählerstand ...
} _cnt;

/// ACT AS DIRECT
void timer_direct_func(unsigned long data)	{
	OBJS_CREATE_DRV_PTR(data);	
	OBJS_CREATE_IDEV_PTR;

	enum e_state state;
	int in = 0;
	u8  pin = 0;

	drv->OT.timer.expires = jiffies + drv->OT.granularity;	
	if(atomic_read( &drv->OT.atomic ) ) {
		complete( &drv->OT.completion  ); 	// final timer call
	} 	
	else {
		// ***********************************
		// ABTASTUNG der Ports - phys
		// ***********************************
		pin = drv->Op_Physic->P_Pin.value;
		in = at91_get_gpio_value(pin);
		// **************************************
		// STATUS-Auswertung
		// **************************************	
		OBJS_DRV_RD_STATE(drv,state);
		switch(state) {
			case e_start:	// status -> running setzen	
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): direct-device started!\n", __FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_running);
				break;
			case e_stop:	// e_stop & e_idle, ohne weitere Auswertung 
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): direct-device stopped!\n", __FILE__,__FUNCTION__);
				OBJS_DRV_WR_STATE(drv,e_idle);	/// Achtung fällt durch!
			case e_idle:
				goto RELOAD_DIRECT_TIMER;
				break;
			default:		// hier: e_running, e_error ...
				break;		// 
		}
		// **************************************
		// SIGNAL-Auswertung
		// **************************************	
		if(in == 0)	{	// Eingang = 0, dann ist
			OBJS_IDEV_WR_VALUE(idev,0);	// Value = 0
		}
		else	{			// Eingang = 1, dann ist
			OBJS_IDEV_WR_VALUE(idev,1);	// Value = 1
		}

RELOAD_DIRECT_TIMER:
		add_timer( &drv->OT.timer); // Timer aufziehen
	} // end else
}

/// ACT AS S0
void timer_s0_func(unsigned long data)	{
	OBJS_CREATE_DRV_PTR(data);	
	OBJS_CREATE_IDEV_PTR;

	enum e_state state;
	unsigned char iflag = 0;
	int in = 0;
	u8  pin = 0;

	drv->OT.timer.expires = jiffies + drv->OT.granularity;	
	if(atomic_read(&drv->OT.atomic)) {
		complete(&drv->OT.completion); 	// final timer call
	} 	
	else {
		// ***********************************
		// ABTASTUNG der Ports - phys
		// ***********************************
		pin = drv->Op_Physic->P_Pin.value;
		in = at91_get_gpio_value(pin);	
		// ***************************
		// STATUS-Auswertung
		// ***************************		
		OBJS_DRV_RD_STATE(drv,state);
		switch(state) {
			case e_start:	// status -> running setzen			
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): S0-device started!\n", __FILE__,__FUNCTION__);
				_cnt.h_sample = 0;	// interne Strukturelemente neu initialisieren
				_cnt.l_sample = 0;
				_cnt.h_end = 0;		
				_cnt.l_end = 0;
				_cnt.foul = 0;
				OBJS_DRV_WR_STATE(drv,e_init);
				break;

			case e_init:							/// INTERCOM
				icom_cli_set_link(di_slot,1);		//  Tastaturverbindung herstellen
				OBJS_DRV_WR_STATE(drv,e_running);
				break;

			case e_running:	 				
				OBJS_IDEV_RD_INIT_EXPORT(idev,iflag); /// INTERCOM	
				if(iflag) {							// JA
					OBJS_IDEV_RD_INIT(idev,iflag);	// init lesen ..
					icom_cli_set_init(di_slot,iflag);// init an Server
					OBJS_IDEV_WR_INIT_EXPORT(idev,0); // Intercom beendet
					
				}
				break;

			case e_stop:	// e_stop & e_idle, ohne weitere Auswertung 
				DEBUG(MCT_DEBUG_LEVEL2, "%s %s(): S0-device stopped!\n", __FILE__,__FUNCTION__);				/// INTERCOM
				icom_cli_set_init(di_slot,0);		// init = 0 an Server	
				icom_cli_set_link(di_slot,0);		// Tastaturverbindung beenden	
				OBJS_IDEV_WR_INIT(idev,0);			// Initialisierung abgebrochen!
				OBJS_IDEV_WR_INIT_EXPORT(idev,0); 	// Intercom beendet			
				OBJS_DRV_WR_STATE(drv,e_idle);		/// Achtung fällt durch

			case e_idle:					
				goto RELOAD_S0_TIMER;				// Timer zyklisch weiter ... 
				break;

			default:		// hier: e_error ...
				break;
		}
		// **************************************
		// SIGNAL-Auswertung
		// **************************************	
		if (in == 1)	{ 						/// *** HIGH-Pegel ***
			switch	(_cnt.l_sample)	{
				case S0_L_SAMPLE:				// L-Abtastung gerade vollständig
					_cnt.l_sample = 0;			// L-Abtastung kann neu beginnen
					_cnt.l_end = 1;				// *** L-Pegel GÜLTIG BEENDET!
					_cnt.foul = 0;				// eventuellen Abtastfehler löschen
					_cnt.h_sample = 1;			// H-Abtastung beginnt mit 1
					break;
				case 0:							// L-Abtastung nicht oder nicht mehr aktiv
					if (_cnt.h_sample < S0_H_SAMPLE)	// H-Abtastung noch notwendig ? 
						_cnt.h_sample++;		// ja wir müssen noch sammeln
					if (_cnt.foul != 0)			// falls H-Abtastfehler, dann langsam
						_cnt.foul--;			// "rausschieben"
					break;
				default:						// L-Abtastung "schwebend"
					//	printk ("y");
					if (_cnt.foul == 0)	{		// 1. L-Abtastfehler ?	
						_cnt.foul = _cnt.l_sample;	// L-Abtastfehler-Position merken
						goto RELOAD_S0_TIMER;	// *** L-Pegel DISPOSITION! sofort raus
					}										
					if(_cnt.foul+1 == _cnt.l_sample)// 2. L-Abtastfehler ?
						_cnt.h_sample = 2;		// H-Abtastung beginnt mit 2
					else
						_cnt.h_sample = 1;		// H-Abtastung beginnt mit 1
					_cnt.foul = 0;				// Abtastfehler löschen
					_cnt.l_sample = 0;			// L-Abtastung neu beginnen 
					_cnt.l_end = 0;				// *** L-Pegel UNGÜLTIG BEENDET!	
					break;
			} // end switch
		}
		else	{								/// *** LOW-Pegel ***
			switch	(_cnt.h_sample)	{
				case S0_H_SAMPLE:				// H-Abtastung gerade vollständig
					_cnt.h_sample = 0;			// H-Abtastung kann neu beginnen
					_cnt.h_end = 1;				// *** H-Pegel GÜLTIG BEENDET!
					_cnt.foul = 0;				// eventuellen Abtastfehler löschen
					_cnt.l_sample = 1;			// L-Abtastung beginnt mit 1
					break;
				case 0:										
					if (_cnt.l_sample < S0_L_SAMPLE) // L-Abtastung noch notwendig ? 
						_cnt.l_sample++;		// ja wir müssen noch sammeln
					if (_cnt.foul != 0)			// falls L-Abtastfehler, dann langsam
						_cnt.foul--;			// "rausschieben"
					break;
				default:						// H-Abtastung "schwebend"
					//	printk ("x");	
					if (_cnt.foul == 0)	{		// 1. H-Abtastfehler ?	
						_cnt.foul = _cnt.h_sample;	// H-Abtastfehler-Position merken
						goto RELOAD_S0_TIMER;	// *** H-Pegel DISPOSITION! sofort raus
					}
					if(_cnt.foul+1 == _cnt.h_sample)// 2. H-Abtastfehler ?
						_cnt.l_sample = 2;		// L-Abtastung beginnt mit 2
					else
						_cnt.l_sample = 1;		// H-Abtastung beginnt mit 1
					_cnt.foul = 0;				// Abtastfehler löschen
					_cnt.h_sample = 0;			// H-Abtastung neu beginnen 
					_cnt.h_end = 0;				// *** H-Pegel UNGÜLTIG BEENDET!	
					break;									
			}	// end switch
		}		// end low-pin	
												/// *** COUNT erfolgreich ***
		if (_cnt.h_end && _cnt.l_end ) {
			_cnt.h_end = 0;
			_cnt.l_end = 0;
			OBJS_IDEV_INC_VALUE(idev);		// Inputzähler zählt
		}	// end if S0

RELOAD_S0_TIMER:
		add_timer(&drv->OT.timer); // Timer aufziehen
	} // end else
}
