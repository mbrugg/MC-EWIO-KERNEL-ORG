#
# Makefile for the kernel mct_spi_dio (digital inputs and outputs over SPI) driver
#

obj-$(CONFIG_MCT_SPI_DIO) 	:= mct_spi_dio.o
	mct_spi_dio-objs := 	mct_dio.o \
							mct_spi_dio_objs.o	\
							mct_spi_dio_sysfs.o	\
							mct_spi_dio_icom.o

# modules
ifeq ($(CONFIG_MCT_SPI_DIO),m)
	mct_spi_dio-objs		+= ../mct_json.o
endif

# spezial dio-dev debugging
ifeq ($(CONFIG_MCT_SPI_DIO_DEV_DEBUG),y)
	EXTRA_CFLAGS += -DMCT_SPI_DIO_DEV_DEBUG
endif

#++++++++++++++++++
# interface MBUS
#++++++++++++++++++
ifeq ($(CONFIG_MCT_SPI_DIO_DEV_IF_MBUS),y)
	EXTRA_CFLAGS += -DMCT_SPI_DIO_DEV_IF_MBUS
	mct_spi_dio-objs += mct_spi_dio_mbus.o \
						mct_spi_dio_mbus_cmd.o \
						../if/mbus_cl1.o \
						../if/mbus_cl2.o \
						../if/mbus_app.o \
						../if/mbus_if.o
endif

#++++++++++++++++++
# interface MODBUS
#++++++++++++++++++
ifeq ($(CONFIG_MCT_SPI_DIO_DEV_IF_MODBUS),y)
	EXTRA_CFLAGS += -DMCT_SPI_DIO_DEV_IF_MODBUS
	mct_spi_dio-objs += mct_spi_dio_modbus.o \
						mct_spi_dio_modbus_cmd.o \
						../if/modbus_sl2.o \
						../if/modbus_sl1.o \
						../if/modbus_app.o \
						../if/modbus_if.o
endif
