#ifndef MCT_VERSIONS_H_
	#define MCT_VERSIONS_H_

#define DRIVER_INTERFACE	"if"	//

#define BUS_INTERN			"0"
#define BUS_LBUS			"1"
#define BUS_MBUS			"2"
#define BUS_MODBUS			"3"
#define BUS_BACNET			"4"
#define BUS_DALI			"5"

// Single Driver
#define DRIVER_SPI_DIO_IDENT		"1"
#define DRIVER_SPI_AIO_IDENT		"2"
// Multi-Driver
#define DRIVER_PIN_DI_IDENT		"0"
#define DRIVER_PAA_DI4_IDENT		"3"
#define DRIVER_PAA_DI10_IDENT		"4"
#define DRIVER_PAA_DO4_IDENT		"5"
#define DRIVER_PAA_DIO42_IDENT		"6"
#define DRIVER_PAA_AI8_IDENT		"7"
#define DRIVER_PAA_AO4_IDENT		"8"

#define DRIVER_PIN_DI_IDENT_NUM		0	// Multi-Driver	:4
#define DRIVER_PIN_DIO_IDENT_NUM	1	// Single-Driver:1
#define DRIVER_PIN_AIO_IDENT_NUM	2	// Single-Driver:1
#define DRIVER_PAA_DI4_IDENT_NUM	3	// Multi-Driver	:6
#define DRIVER_PAA_DI10_IDENT_NUM	4	// Multi-Driver	:6
#define DRIVER_PAA_DO4_IDENT_NUM	5	// Multi-Driver	:6
#define DRIVER_PAA_DIO42_IDENT_NUM	6	// Multi-Driver	:6
#define DRIVER_PAA_AI8_IDENT_NUM	7	// Multi-Driver	:6
#define DRIVER_PAA_AO4_IDENT_NUM	8	// Multi-Driver	:6


// RTU-Modbus-Primär-Adressen mapping
#define MODBUS_RTU_TO_EXTERN_BASE_ADDR	0	// für externe Module 
#define MODBUS_RTU_TO_LBUS_BASE_ADDR	100	// für LBUS-Module
#define MODBUS_RTU_TO_INTERN_BASE_ADDR	200	// für interne Module


#endif
