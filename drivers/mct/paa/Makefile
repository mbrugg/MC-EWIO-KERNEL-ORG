#
# Makefile for the kernel MCT GmbH drivers.
#
# Object files in directory
# spezial core debugging
# Object files in subdirectories

ifeq ($(CONFIG_MCT_PAA_DEBUG),y)
	EXTRA_CFLAGS += -DMCT_PAA_DEBUG
endif

obj-$(CONFIG_MCT_PAA)			+= mct_paa.o
obj-$(CONFIG_MCT_PAA_SPI) 		+= mct_paa_master.o
mct_paa_master-objs := 	mct_paa_mux.o \
						mct_paa_mux_objs.o \
						mct_paa_mux_attr.o \
						mct_paa_lbus_mstr.o	\
						mct_paa_spi.o	\
						mct_paa_spi_uart.o
obj-$(CONFIG_MCT_PAA_DI4)		+= di4/
obj-$(CONFIG_MCT_PAA_DI10)		+= di10/
obj-$(CONFIG_MCT_PAA_DO4)		+= do4/
obj-$(CONFIG_MCT_PAA_DIO42)		+= dio42/
obj-$(CONFIG_MCT_PAA_AI8)		+= ai8/
obj-$(CONFIG_MCT_PAA_AO4)		+= ao4/

# modules
ifeq ($(CONFIG_MCT_PAA_SPI),m)
 	mct_paa_master-objs += ../mct_json.o
endif