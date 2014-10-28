/*********************************************************************************
	Copyright MC-Technology GmbH 2011

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$ 	25.08.2011
 	
 	Description:	MBUS Command-Implemetation for mct_pin_di-Driver

	Reset-Subcodes:	- APPLICATION_RESET_USER_DATA
					- APPLICATION_RESET_INSTALLATION_AND_STARTUP
					- APPLICATION_RESET_CALIBRATION

	07.07.2011		Kein Control-Signal mehr, Spannungsausfall wird im Status
					bei einem REQ_UD2 Telegramm geliefert!
					Nach dem 1. Auslesen wird der Status automatisch auf 0 gesetzt

	20.03.2012		Infos zum Applikation-Reset 
					Die Möglichkeit im S0-Mode über Application-Reset-Codes
					den Zähler zur Einmann-Inbetriebnahme zu aktivieren und
					zu deaktivieren wird nicht dokumentiert!
					Offiziell dokumentieren wir die Nutzung der Taster am EWIO-M
					um die S0-Zähler in die Einmann-Inbetriebnahme zu versetzen
	
 *******************************************************************************/
#ifndef MCT_PIN_DI_MBUS_CMD_H_
	#define MCT_PIN_DI_MBUS_CMD_H_

#include "../if/mbus_cl2.h"
#include "../if/mbus_app.h"

///	OVERLOAD mbus_cmd_rsp_ud_data2()
/// Hinweise:
// keine ...
extern int mbus_cmd_rsp_ud_data2 (struct m_app * la);

///	OVERLOAD mbus_cmd_snd_ud_usr_data()
/// Hinweise:
//	Es werden hier nur fixe Datenstrukturen erwartet die ab DIF1-Feld 
//	ausgewertet werden. Die einzelnen DIF-Sequenzen können in beliebiger
//	Reihenfolge im Telegramm angeordnet (shuffle-Mode) sein. Die einzelnen
//	DIF-Sequencen werden entsprechend der Anordnung im Telegramm 
//	immer sequentiell abgearbeitet!
 	
//	Der Wert l1->rec.stack darf nur manipuliert werden, wenn alle 
//	Bedingungen für den DATA-RECORD erfüllt werden!
//	Sonst ist l1->rec.stack zu belassen und mit return(0) zu beenden!
	
//	Der Parameter mask ist eine BIT-Maske, welche die Channel-Info 
//	über die aktiven Channel beinhaltet.
//	Es werden bis zu MBUS_CHANNEL_MAX Channel unterstützt, wobei
//	die Variable l2->chan_cfg die aktuell konfigurierte Anzahl angibt!
extern int mbus_cmd_snd_ud_usr_data	(struct m_l2 * l2, unsigned char mask);


/// OVERLOAD  mbus_cmd_snd_ud_application_reset()
/// Hinweise:
// <DI-, COUNTER-Mode>
// support:	"APPLICATION_RESET_USER_DATA"

// <COUNTER-Mode>
// support: "APPLICATION_RESET_INSTALLATION_AND_STARTUP"
// (1) Bewirkt, daß die jeweilige Kalibrations-Flag des aktiven Kanals
// gesetzt werden und das S0-Register und das Freeze-Register wird auf 0 
// gestellt.

// (2) Wird in diesem Zustand das S0-Register/Freeze-Register vom Master 
// abgerufen, so enthält immer nur das Freeze-Register die aktuellen 
// S0-Impulse.
// d.h. Die Impulse werden in dem Freeze-Register kumuliert.

// (2.1) Sendet der Master in diesem Zustand Freeze-Befehle,
// werden diese auf Grund der gesetzten Kalibrierungs-Flags abgewiesen.
// (ignoriert), da die Geräte im Kalibrierungsmode arbeiten. 

// support: "APPLICATION_RESET_CALIBRATION"  bzw. VIFE_FD_STATE_OF_PARAMETER_ACTIVATION (clear)
// (3) Sendet der Master den M-Bus Befehl, so werden die Kalibrierungs-Flags 
// der jeweiligen aktiven Kanäle wieder gelöscht. 
// d.h. Die Werte der Freeze-Register werden vorher auf die S0-Register 
// kopiert. Wenn kein Neustart des Treibers erfolgte, haben die S0-Register 
// folglich die Werte der Freeze-Register. 
// Alle neuen Impulse werden in die S0-Register kumuliert.

// Zählerabgleich: 
// Aktuellen Messwert des Zählers vor Ort aufschreiben. 
// Variante a) per Internet Remote auf dem EWIO-M über den MBUS-Treiber 
//			   APPLICATION_RESET_INSTALLATION_AND_STARTUP auslösen und in den
//			   Calibrations-Mode schalten.
// 
// Variante b) per Taste auf dem EWIO-M intern in den 
//			   Calibrations-Mode schalten.
//			   Die Taste wird direkt vom Treiber bearbeitet.
 
extern int mbus_cmd_snd_ud_application_reset(struct m_l2 * l2, unsigned char mask);

/// OVERLOAD  mbus_cmd_snd_ud_global_readout()
/// Hinweise:
// DIF  = 0x7F 	
// VIF	= 0x7E	(optional)
// Alle DatenRecords werden markiert zum einmaligen auslesen, bei REQ_UD2
// aber nur dann, wenn der Wert noch nicht zur readout-Liste addiert wurde!
extern int mbus_cmd_snd_ud_global_readout(struct m_l2 * l2, unsigned char mask);

#endif	//MCT_PIN_DI_MBUS_CMD_H_