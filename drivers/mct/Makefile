#
# Makefile for the kernel MCT GmbH drivers.
#
# Object files in directory
# spezial core debugging
# Object files in subdirectories

ifeq ($(CONFIG_MCT_DEBUG),y)
	EXTRA_CFLAGS += -DMCT_DEBUG
endif

obj-$(CONFIG_MCT)			+= mct_boxes.o mct_json.o
obj-$(CONFIG_MCT_PIN_DI)	+= pin_di/
obj-$(CONFIG_MCT_SPI_DIO)	+= spi_dio/
obj-$(CONFIG_MCT_SPI_AIO)	+= spi_aio/
obj-$(CONFIG_MCT_PAA) 		+= paa/
