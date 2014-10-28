/*********************************************************************************

 Copyright MC-Technology GmbH 2010
  
 Autor:	Dipl.-Ing. Steffen Kutsche		

	Dieses File beinhaltet das aktuelle JSON-SCHEMA für den
 	Treiber. Leerzeichen, Tabs, Linefeeds wurden entfernt!

	Hinweise:	Die Länge sollte immer < 4kByte sein
				Minimierung mit http://fmarcia.info/jsmin/test.html 
				
				Die UUID wurde nach RFC4112 mit folgendem Generator erstellt:	
				http://www.famkruithof.net/uuid/uuidgen

				Last Validation-Check: 04.11.2010 - Valid JSON
				http://www.jsonlint.com/
				http://chris.photobooks.com/json/
*********************************************************************************/
#define UUID_SCHEMATA	"302faa10-ba59-11df-851a-0800200c9a60"

/// **********************
/// JSON SCHEMATA
/// **********************
#define _QUO(x) #x
#define QUO(x) _QUO(x)

#define JSON_SCHEMA_PAA_DI4_EN "{\
\"uuid\":"QUO(UUID_SCHEMATA)",\
\"type\":\"object\",\
\"description\":\"di4 paa-client-driver (digital I-driver)\",\
\"properties\":\
{\
"QUO(OBJECT_DRIVER_S)":\
{\
\"type\":\"string\",\
\"readonly\":true,\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(OBJECT_CONTROL_S)":\
{\
\"type\":\"object\",\
\"properties\":\
{\
"QUO(PROPERTY_COMMAND_S)":\
{\
\"type\":\"number\",\
\"options\":\
[\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_STOP)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_STOP)"\
},\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_START)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_START)"\
}\
]\
},\
"QUO(PROPERTY_STATE_S)":\
{\
\"type\":\"number\",\
\"readonly\":true,\
\"optional\":true,\
\"options\":\
[\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_IDLE)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_IDLE)"\
},\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_START)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_START)"\
},\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_RUNNING)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_RUNNING)"\
},\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_STOP)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_STOP)"\
},\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_ERROR)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_ERROR)"\
}\
]\
}\
}\
},\
"QUO(OBJECT_PHYSIC_S)":\
{\
\"type\":\"object\",\
\"readonly\":true,\
\"properties\":\
{\
"QUO(PROPERTY_LAYER_BUS_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(PROPERTY_INFO_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
}\
}\
},\
"QUO(OBJECT_DEVICE_Ss)":\
{\
\"items\":\
{\
\"type\":\"object\",\
\"description\":\"di4 paa-client-port (digital I-port)\",\
\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":\
{\
\"type\":\"string\",\
\"readonly\":true,\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(OBJECT_VALUE_Ss)":\
{\
\"items\":\
{\
\"type\":\"object\",\
\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(PROPERTY_SIZE_S)":\
{\
\"type\":\"number\",\
\"readonly\":true,\
\"maximum\":"QUO(SIZE_VALUE1)"\
},\
"QUO(PROPERTY_VALUE_S)":\
{\
\"type\":\"number\",\
\"readonly\":true,\
\"maximum\":"QUO(SIZE_MAX_DEZ_1bit)"\
}\
}}}}}}}}"
