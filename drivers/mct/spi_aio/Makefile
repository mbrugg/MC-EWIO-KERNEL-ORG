#
# Makefile for the kernel mct_spi_aio (analog inputs and outputs over SPI-PIC) driver
#
obj-$(CONFIG_MCT_SPI_AIO) 	:= mct_spi_aio.o

mct_spi_aio-objs		:= 	mct_aio.o \
							mct_spi_aio_uart.o \
							mct_spi_aio_objs.o \
							mct_spi_aio_sysfs.o
# modules
ifeq ($(CONFIG_MCT_SPI_AIO),m)
mct_spi_aio-objs 		+= ../mct_json.o ../mct_boxes.o
endif

# spezial aio-dev debugging
ifeq ($(CONFIG_MCT_SPI_AIO_DEV_DEBUG),y)
	EXTRA_CFLAGS += -DMCT_SPI_AIO_DEV_DEBUG
endif

#++++++++++++++++++
# interface MBUS
#++++++++++++++++++
ifeq ($(CONFIG_MCT_SPI_AIO_DEV_IF_MBUS),y)
	EXTRA_CFLAGS += -DMCT_SPI_AIO_DEV_IF_MBUS
	mct_spi_aio-objs += mct_spi_aio_mbus.o \
						mct_spi_aio_mbus_cmd.o \
						../if/mbus_cl1.o \
						../if/mbus_cl2.o \
						../if/mbus_app.o \
						../if/mbus_if.o
endif

#++++++++++++++++++
# interface MODBUS
#++++++++++++++++++
ifeq ($(CONFIG_MCT_SPI_AIO_DEV_IF_MODBUS),y)
	EXTRA_CFLAGS += -DMCT_SPI_AIO_DEV_IF_MODBUS
	mct_spi_aio-objs += mct_spi_aio_modbus.o \
						mct_spi_aio_modbus_cmd.o \
						../if/modbus_sl2.o \
						../if/modbus_sl1.o \
						../if/modbus_app.o \
						../if/modbus_if.o
endif



