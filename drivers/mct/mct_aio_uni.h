/*********************************************************************************

 Copyright MC-Technology GmbH 2009
  
 Autor:	Dipl.-Ing. Steffen Kutsche		

  $27.08.12	Bezeichnung/Kennung geändert
		#define AI_PROP_MODE_A_S "mA:0-20" in "Ampere:0-0,02" 
*********************************************************************************/
#ifndef MCT_AIO_UNI_H_
	#define MCT_AIO_UNI_H_

// Standard-Template-Names (possible overload or expanded by user)
#define AIO_TPL_NAME_IN		"ai_"
#define AIO_TPL_NAME_OUT 	"ao_"

#define AI_PROP_MODE_PATTERN_DEF	"inactive"
#define AI_PROP_MODE_PATTERN_0		"Ohm"
#define AI_PROP_MODE_PATTERN_1		"Temp"
#define AI_PROP_MODE_PATTERN_2		"Volt"
#define AI_PROP_MODE_PATTERN_3		"Ampere"

// Input-Modes
#define AI_PROP_MODE_0_S	AI_PROP_MODE_PATTERN_DEF
#define AI_PROP_MODE_1_S	"Volt:0-10"
#define AI_PROP_MODE_2_S	"Temp:LM235"
#define AI_PROP_MODE_3_S	"Ohm:0"
#define AI_PROP_MODE_4_S	"Ohm:1"
#define AI_PROP_MODE_5_S	"Ohm:2"
#define AI_PROP_MODE_6_S	"Ohm:3"
#define AI_PROP_MODE_7_S	"Ohm:4"
#define AI_PROP_MODE_8_S	"Ohm:5"
#define AI_PROP_MODE_9_S	"Ohm:6"
#define AI_PROP_MODE_A_S	"Ampere:0-0,02"

// Input-Labels
#define AI_LAB_MODE_0_S 	"Eingang nicht aktiv!"
#define AI_LAB_MODE_1_S		"Spannungsmessung: 0V - 10V"
#define AI_LAB_MODE_2_S 	"IC Temperatur-Sensor: LM235 (Messwert in Volt)"
#define AI_LAB_MODE_3_S 	"Widerstandsmessung: 40 Ohm - 4 MOhm"
#define AI_LAB_MODE_4_S 	"Widerstandsmessung: 40 Ohm - 14 kOhm"
#define AI_LAB_MODE_5_S 	"Widerstandsmessung: 12 kOhm - 4 MOhm"
#define AI_LAB_MODE_6_S 	"Widerstandsmessung: 40 Ohm - 650 Ohm"
#define AI_LAB_MODE_7_S 	"Widerstandsmessung: 500 Ohm - 14 kOhm"
#define AI_LAB_MODE_8_S 	"Widerstandsmessung: 12 kOhm - 180 kOhm"
#define AI_LAB_MODE_9_S 	"Widerstandsmessung: 140 kOhm - 4 MOhm"
#define AI_LAB_MODE_A_S 	"Strommessung: 0 - 20 mA"


#endif
