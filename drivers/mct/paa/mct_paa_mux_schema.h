/*********************************************************************************

 Copyright MC-Technology GmbH 2010
  
 Autor:	Dipl.-Ing. Steffen Kutsche		

	Dieses File beinhaltet das aktuelle JSON-SCHEMA für den
 	Treiber. Leerzeichen, Tabs, Linefeeds wurden entfernt!

	Hinweise:	Die Länge sollte immer < 4kByte sein
				Minimierung mit http://fmarcia.info/jsmin/test.html 
				
				Die UUID wurde nach RFC4112 mit folgendem Generator erstellt:	
				http://www.famkruithof.net/uuid/uuidgen
				
				Last Validation-Check: 27.09.2010 - Valid JSON
				http://www.jsonlint.com/
				http://chris.photobooks.com/json/
*********************************************************************************/
#define UUID_SCHEMATA	"90762b05-c7b8-13df-bd3b-0800200c9a60"

/// **********************
/// JSON SCHEMATA
/// **********************
#define _QUO(x) #x
#define QUO(x) _QUO(x)
#define JSON_SCHEMA_PAA_MUX_EN "{\
\"uuid\":"QUO(UUID_SCHEMATA)",\
\"type\":\"object\",\
\"description\":\"paa-master-driver (on SPI device)\",\
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
\"readonly\":true,\
\"properties\":\
{\
"QUO(PROPERTY_SPI_LAYER_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(PROPERTY_BUS_LAYER_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
}\
}\
},\
"QUO(OBJECT_LBUS_S)":\
{\
\"type\": \"object\",\
\"readonly\":true,\
\"description\":\"lower bus monitoring\",\
\"properties\":\
{\
"QUO(PROPERTY_SLOT_Ss)":\
{\
\"type\": \"number\",\
\"readonly\":false,\
\"maximum\":"QUO(LBUS_DEF_SLOTS_MAX)"\
},\
"QUO(OBJECT_TIMEOUT_Ss)":\
{\
\"type\":\"array\",\
\"description\":\"timeouts\",\
\"items\":\
{\
\"type\":\"object\",\
\"description\":\"timeout object\",\
\"properties\":\
{\
"QUO(PROPERTY_SLOT_S)":\
{\
\"type\":\"number\"\
},\
"QUO(PROPERTY_TIMEOUT_CODE_S)":\
{\
\"type\":\"string\",\
\"pattern\":\"^0x([\\\\da-fA-F]{2})$\"\
},\
"QUO(PROPERTY_TIMEOUT_COUNTER_S)":\
{\
\"type\":\"string\",\
\"pattern\":\"^0x([\\\\da-fA-F]{2})$\"\
}\
},\
\"minItems\":"QUO(LBUS_DEF_SLOTS_MIN)",\
\"maxItems\":"QUO(LBUS_DEF_SLOTS_MAX)"\
}\
},\
"QUO(OBJECT_SLOT_Ss)":\
{\
\"type\":\"array\",\
\"description\":\"paa-client-driver slots\",\
\"readonly\":true,\
\"items\":\
{\
\"type\":\"object\",\
\"description\":\"paa-client-driver slot\",\
\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(PROPERTY_VERSION_S)":\
{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
"QUO(PROPERTY_CFG_S)":\
{\
\"type\":\"string\",\
\"pattern\":\"^0x([\\\\da-fA-F]{2})$\"\
},\
"QUO(PROPERTY_CFG_IN_S)":\
{\
\"type\":\"string\",\
\"pattern\":\"^0x([\\\\da-fA-F]{2})$\"\
},\
"QUO(PROPERTY_CFG_OUT_S)":\
{\
\"type\":\"string\",\
\"pattern\":\"^0x([\\\\da-fA-F]{2})$\"\
},\
"QUO(PROPERTY_CFG_CFG_S)":\
{\
\"type\":\"string\",\
\"pattern\":\"^0x([\\\\da-fA-F]{2})$\"\
}},\
\"minItems\":"QUO(LBUS_DEF_SLOTS_MIN)",\
\"maxItems\":"QUO(LBUS_DEF_SLOTS_MAX)"\
}}}}}}"
