/*********************************************************************************
	Copyright MCQ TECH GmbH 2012

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		27.01.2012

 	Description:

	Dieses File beinhaltet das aktuelle JSON-SCHEMA für den
 	Treiber. Leerzeichen, Tabs, Linefeeds wurden entfernt!

	Hinweise:	Die Länge sollte immer < 4kByte sein
				Minimierung mit http://fmarcia.info/jsmin/test.html 
				
				Die UUID wurde nach RFC4112 mit folgendem Generator erstellt:	
				http://www.famkruithof.net/uuid/uuidgen
				
				Last Validation-Check: 14.02.2012 - Valid JSON
				http://www.jsonlint.com/
				http://chris.photobooks.com/json/
				http://europass.cedefop.europa.eu/en/resources/for-developers/json/validation

*********************************************************************************/
#define UUID_SCHEMATA	"3ffc7990-2038-11e1-8bd2-0811200c9b77"

/// **********************
/// JSON SCHEMATA
/// **********************
#define _QUO(x) #x
#define QUO(x) _QUO(x)
#define JSON_SCHEMA_SPI_AIO_EN "{\
\"uuid\":"QUO(UUID_SCHEMATA)",\
\"type\":\"object\",\
\"description\":\"analog I/O-driver (SPI and PIC based)\",\
\"Mode_UR\":\
{\
\"type\":\"string\",\
\"optional\":true,\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"options\":[\
{\"value\":"QUO(AI_PROP_MODE_0_S)",\"label\":"QUO(AI_LAB_MODE_0_S)"},\
{\"value\":"QUO(AI_PROP_MODE_1_S)",\"label\":"QUO(AI_LAB_MODE_1_S)"},\
{\"value\":"QUO(AI_PROP_MODE_2_S)",\"label\":"QUO(AI_LAB_MODE_2_S)"},\
{\"value\":"QUO(AI_PROP_MODE_3_S)",\"label\":"QUO(AI_LAB_MODE_3_S)"},\
{\"value\":"QUO(AI_PROP_MODE_4_S)",\"label\":"QUO(AI_LAB_MODE_4_S)"},\
{\"value\":"QUO(AI_PROP_MODE_5_S)",\"label\":"QUO(AI_LAB_MODE_5_S)"},\
{\"value\":"QUO(AI_PROP_MODE_6_S)",\"label\":"QUO(AI_LAB_MODE_6_S)"},\
{\"value\":"QUO(AI_PROP_MODE_7_S)",\"label\":"QUO(AI_LAB_MODE_7_S)"},\
{\"value\":"QUO(AI_PROP_MODE_8_S)",\"label\":"QUO(AI_LAB_MODE_8_S)"},\
{\"value\":"QUO(AI_PROP_MODE_9_S)",\"label\":"QUO(AI_LAB_MODE_9_S)"}\
]},\
\"Mode_I\":\
{\
\"type\":\"string\",\
\"optional\":true,\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"options\":[\
{\"value\":"QUO(AI_PROP_MODE_0_S)",\"label\":"QUO(AI_LAB_MODE_0_S)"},\
{\"value\":"QUO(AI_PROP_MODE_A_S)",\"label\":"QUO(AI_LAB_MODE_A_S)"}\
]},\
\"Format_10Bit\":{\
\"type\":\"number\",\
\"maximum\":"QUO(SIZE_MAX_DEZ_10bit)"\
},\
\"Format_11Bit\":{\
\"type\":\"number\",\
\"maximum\":"QUO(SIZE_MAX_DEZ_11bit)"\
},\
\"Format_10Chr\":{\
\"type\":\"string\",\
\"maxLength\":"QUO(STRING_U32_SIZE)",\
\"format\":\"IEEE754-32\"\
},\
\"Format_10ChrRo\":{\
\"type\":\"string\",\
\"maxLength\":"QUO(STRING_U32_SIZE)",\
\"format\":\"IEEE754-32\",\
\"readonly\":true\
},\
\"Format_1HexRo\":{\
\"type\":\"string\",\
\"regex\":\"/^0x([\\\\da-fA-F]{2})$/\",\
\"readonly\":true\
},\
\"Size_32Ro\":{\
\"type\":\"number\",\
\"optional\":true,\
\"readonly\":true,\
\"maximum\":"QUO(SIZE_VALUE32)"\
},\
\"Name_50Ro\":{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)",\
\"readonly\":true\
},\
\"Name_50\":{\
\"type\":\"string\",\
\"maxLength\":"QUO(PROPERTY_SIZE)"\
},\
\"Prop_10Chr\":{\"properties\":{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50\"},\
"QUO(PROPERTY_SIZE_S)":{\"$ref\":\"#.Size_32Ro\"},\
"QUO(PROPERTY_VALUE_S)":{\"$ref\":\"#.Format_10Chr\"}\
}},\
\"Prop_10ChrEx\":{\"properties\":{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50\"},\
"QUO(PROPERTY_SIZE_S)":{\"$ref\":\"#.Size_32Ro\"},\
"QUO(PROPERTY_VALUE_S)":{\"$ref\":\"#.Format_10Chr\"},\
"QUO(PROPERTY_OFFSET_S)":{\"$ref\":\"#.Format_10ChrRo\"}\
}},\
\"Prop_10Bit\":{\"properties\":{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50\"},\
"QUO(PROPERTY_SIZE_S)":{\"$ref\":\"#.Size_32Ro\"},\
"QUO(PROPERTY_VALUE_S)":{\"$ref\":\"#.Format_10Bit\"}\
}},\
\"Prop_11Bit\":{\"properties\":{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50\"},\
"QUO(PROPERTY_SIZE_S)":{\"$ref\":\"#.Size_32Ro\"},\
"QUO(PROPERTY_VALUE_S)":{\"$ref\":\"#.Format_11Bit\"}\
}},\
\"properties\":\
{\
"QUO(OBJECT_DRIVER_S)":{\"$ref\":\"#.Name_50Ro\"},\
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
},\
{\
\"value\":"QUO(PROPERTY_COMMAND_AND_STATE_RESET)",\
\"label\":"QUO(LABEL_COMMAND_AND_STATE_RESET)"\
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
"QUO(PROPERTY_LAYER_SPI_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(PROPERTY_LAYER_PIC_S)":{\"$ref\":\"#.Name_50Ro\"}\
}\
},\
"QUO(OBJECT_ERROR_S)":\
{\
\"type\":\"object\",\
\"properties\":{\
"QUO(PROPERTY_ERROR_UART_Retrieve)":{\"$ref\":\"#.Format_1HexRo\"},\
"QUO(PROPERTY_ERROR_UART_Timeout)":{\"$ref\":\"#.Format_1HexRo\"},\
"QUO(PROPERTY_ERROR_UART_Frame)":{\"$ref\":\"#.Format_1HexRo\"},\
"QUO(PROPERTY_ERROR_UART_State)":{\"$ref\":\"#.Format_1HexRo\"},\
"QUO(PROPERTY_ERROR_PIC_PIC)":{\"$ref\":\"#.Format_1HexRo\"}\
}\
},\
"QUO(OBJECT_DEVICE_Ss)":{\"description\":\"analog I/O-port\",\
\"items\":[\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_10ChrEx\"},\
"QUO(PROPERTY_MODE_S)":{\"$ref\":\"#.Mode_UR\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_10ChrEx\"},\
"QUO(PROPERTY_MODE_S)":{\"$ref\":\"#.Mode_UR\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_10Chr\"},\
"QUO(PROPERTY_MODE_S)":{\"$ref\":\"#.Mode_I\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_10Chr\"},\
"QUO(PROPERTY_MODE_S)":{\"$ref\":\"#.Mode_I\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_10Bit\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_10Bit\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_11Bit\"}\
}\
},\
{\"type\":\"object\",\"optional\":true,\"properties\":\
{\
"QUO(PROPERTY_NAME_S)":{\"$ref\":\"#.Name_50Ro\"},\
"QUO(OBJECT_VALUE_S)":{\"$ref\":\"#.Prop_11Bit\"}\
}\
}]\
}}}"
