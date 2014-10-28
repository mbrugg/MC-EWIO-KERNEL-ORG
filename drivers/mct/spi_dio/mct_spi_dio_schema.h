/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011

	Dieses File beinhaltet das aktuelle JSON-SCHEMA für den
 	Treiber. Leerzeichen, Tabs, Linefeeds wurden entfernt!

	Hinweise:	Die Länge sollte immer < 4kByte sein
				Minimierung mit http://fmarcia.info/jsmin/test.html 
				
				Die UUID wurde nach RFC4112 mit folgendem Generator erstellt:	
				http://www.famkruithof.net/uuid/uuidgen
				
				Last Validation-Check: 30.11.2011 - Valid JSON
				http://www.jsonlint.com/
				http://chris.photobooks.com/json/
*********************************************************************************/
#define UUID_SCHEMATA	"a2662d91-0741-11df-8a39-0800200c9a60"

/// **********************
/// JSON SCHEMATA
/// **********************
#define _QUO(x) #x
#define QUO(x) _QUO(x)
#define JSON_SCHEMA_SPI_DIO_EN "{\
\"uuid\":"QUO(UUID_SCHEMATA)",\
\"type\":\"object\",\
\"description\":\"digital I/O-driver (SPI based)\",\
\"properties\":\
{\
"QUO(OBJECT_DRIVER_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"readonly\":true\
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
\"optional\":true,\
\"readonly\":true,\
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
\"properties\":\
{\
"QUO(PROPERTY_LAYER_SPI_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"readonly\":true\
}\
}\
},\
"QUO(OBJECT_DEVICE_Ss)":\
{\
\"type\":\"array\",\
\"items\":\
[\
{\
\"type\":\"object\",\
\"description\":\"digital O-port\",\
\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"readonly\":true\
},\
"QUO(OBJECT_INIT_Ss)":\
{\
\"type\":\"object\",\
\"readonly\":true,\
\"properties\":\
{\
"QUO(PROPERTY_VALUE_S)":\
{\
\"type\":\"number\",\
\"maximum\":"QUO(SIZE_MAX_DEZ_4bit)"\
},\
"QUO(PROPERTY_SIZE_S)":\
{\
\"type\":\"number\",\
\"maximum\":"QUO(SIZE_VALUE4)"\
},\
"QUO(PROPERTY_LINK_S)":\
{\
\"type\":\"number\",\
\"maximum\":"QUO(SIZE_MAX_DEZ_4bit)"\
}\
}\
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
\"maximum\":"QUO(SIZE_MAX_DEZ_1bit)"\
}}}}}},\
{\
\"type\":\"object\",\
\"description\":\"digital I-port\",\
\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"readonly\":true\
},\
"QUO(OBJECT_VALUE_Ss)":\
{\
\"items\":\
{\
\"type\":\"object\",\
\"properties\":{\
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
}}}}}]}}}"

