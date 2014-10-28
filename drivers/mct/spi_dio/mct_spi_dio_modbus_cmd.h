/*********************************************************************************
	Copyright MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		06.01.2012
 	
 	Description:  MODBUS Command-Implementation for mct_spi_dio-Driver

				01	= FUNC_RD_COILS				(Ausgänge 0-1 und Handschalter lesen)
				02	= FUNC_RD_DISCRETE_INPUTS	(Eingänge 0-3 direkt lesen)
				05	= FUNC_WR_SINGLE_COIL		(Ausgang 0 oder Ausgang 1 schalten)
				15	= FUNC_WR_MULTIPLE_COILS	(Ausgang/Ausgänge 0-1 schalten)
				43	= FUNC_ENCAPSULATED_INTERFACE_TRANSPORT (+0x14 == ID Lesen)

 *******************************************************************************/
#ifndef MCT_SPI_DIO_MODBUS_CMD_H_
	#define MCT_SPI_DIO_MODBUS_CMD_H_

#include "../if/modbus_sl2.h"
#include "../if/modbus_app.h"

// Hinweis:
// Es werden die Zustände der Relaisausgänge (0-1) und die Informationen
// der Handmaskenbits (0-1) gelesen.
// Anordnung bei Read Input Coils
// Bit 0..1: Tatsächlicher Relais-Zustand, 0: Aus, 1: Ein
// Bit 2..3: Ursache des Relais-Zustands, 0: Bus, 1: Schalter
extern int dio_mod_sl2_cmd_rsp_01(struct mod_l2 * l2);

// Hinweis:
// Es werden die Zustände der Eingänge (0-3) gelesen.
extern int dio_mod_sl2_cmd_rsp_02(struct mod_l2 * l2);

// Hinweis:
// Es wird ein Realisausgang (0-1) gesetzt. Beim Aufruf dieser Funktion,
// wird der aktuelle Zustand der Relaisausgänge gelesen.
// Danach erfolgt die Verknüpfung des entsprechenden Ralaisausgang und
// dann das Schreiben.
// Bevor ein erneuter Aufruf dieser Funktion erfolgt, muss von der Applikation
// sichergestellt werden, dass entweder genügend Zeit vergeht oder über die
// Prüfung (mit read coils()) der Zustand an den Relais sich einstellen konnte.
// Die Notwendigkeit ergibt sich aus der Laufzeit für das Prozessabbild, welches
// über den LBUS aufgefrischt wird. Als Richtwert kann gelten:
// 2 Slots belegt, dann ca. 40 - 50 ms 
// 6 Slots belegt, dann ca. 120 - 150 ms
extern int dio_mod_sl2_cmd_rsp_05(struct mod_l2 * l2);

// Hinweis:
// Es werden der/die Realisausgänge (0-1) gesetzt. Beim Aufruf dieser Funktion,
// wird der aktuelle Zustand der Relaisausgänge gelesen.
// Das Zeitregime für den Mehrfachaufruf dieser Funktion unmittelbar nacheinander
// muss analog der Funktion:  dio42_mod_sl2_cmd_rsp_05() erfolgen.
extern int dio_mod_sl2_cmd_rsp_15(struct mod_l2 * l2);


extern int dio_mod_sl2_cmd_rsp_mei_14_code(struct mod_l2 * l2, unsigned char code);

#endif	//MCT_SPI_DIO_MODBUS_CMD_H_