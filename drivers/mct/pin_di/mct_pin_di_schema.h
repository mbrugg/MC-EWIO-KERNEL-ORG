/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		02.11.2011
 	
 	Description:
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
#define UUID_SCHEMATA	"f646a4bc-0745-11df-8a39-0812270c9a60"

/// **********************
/// JSON SCHEMATA
/// **********************
#define _QUO(x) #x
#define QUO(x) _QUO(x)

#define JSON_SCHEMA_PIN_DI_EN "{\
\"uuid\":"QUO(UUID_SCHEMATA)",\
\"type\":\"object\",\
\"description\":\"digital I-driver (PIN based)\",\
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
\"optional\":true,\
\"properties\":\
{\
"QUO(PROPERTY_PIN_S)":\
{\
\"type\":\"number\",\
\"readonly\":true,\
\"maxLength\":"QUO(SIZE_VALUE2)"\
},\
"QUO(PROPERTY_TRI_S)":\
{\
\"type\":\"number\",\
\"readonly\":true,\
\"maxLength\":"QUO(SIZE_VALUE1)"\
}\
}\
},\
"QUO(OBJECT_DEVICE_S)":\
{\
\"type\":\"object\",\
\"description\":\"digital I-port (direct,S0)\",\
\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"readonly\":true\
},\
"QUO(OBJECT_MODE_S)":\
{\
\"type\":\"object\",\
\"properties\":\
{\
"QUO(PROPERTY_MODE_S)":\
{\
\"type\":\"string\",\
\"enum\":\
[\
"QUO(PROPERTY_MODE_DIRECT_S)",\
"QUO(PROPERTY_MODE_S0_S)"\
]\
},\
"QUO(PROPERTY_VALUE_S)":\
{\
\"type\":\"number\",\
\"optional\":true,\
\"readonly\":true,\
\"maxLength\":"QUO(SIZE_VALUE1)"\
}\
}\
},\
"QUO(OBJECT_VALUE_S)":\
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
\"optional\":true,\
\"readonly\":true,\
\"maxLength\":"QUO(SIZE_VALUE2)"\
},\
"QUO(PROPERTY_VALUE_S)":\
{\
\"type\": \"string\",\
\"readonly\":true,\
\"maxLength\":"QUO(STRING_LLD_SIZE)"\
}\
}\
},\
"QUO(OBJECT_FREEZE_S)":\
{\
\"type\":\"object\",\
\"optional\":true,\
\"readonly\":true,\
\"properties\":\
{\
"QUO(PROPERTY_INIT_S)":\
{\
\"type\":\"number\",\
\"maximum\":1,\
\"minimum\":0\
},\
"QUO(PROPERTY_SIZE_S)":\
{\
\"type\":\"number\",\
\"maxLength\":"QUO(SIZE_VALUE2)"\
},\
"QUO(PROPERTY_VALUE_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(STRING_LLD_SIZE)"\
}\
}\
}\
}}}\
}"
