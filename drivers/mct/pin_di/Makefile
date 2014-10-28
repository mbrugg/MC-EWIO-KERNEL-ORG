#
# Makefile for the kernel mct_pin_inp (Inputs over simple controller PINs) driver
#
obj-$(CONFIG_MCT_PIN_DI) += mct_pin_di_0.o
		mct_pin_di_0-objs := 	mct_di_0.o \
								mct_di.o \
								mct_pin_di_objs.o \
								mct_pin_di_timer.o \
								mct_pin_di_sysfs.o

obj-$(CONFIG_MCT_PIN_DI) += mct_pin_di_1.o
		mct_pin_di_1-objs := 	mct_di_1.o \
								mct_di.o \
								mct_pin_di_objs.o \
								mct_pin_di_timer.o \
								mct_pin_di_sysfs.o
obj-$(CONFIG_MCT_PIN_DI) += mct_pin_di_2.o
		mct_pin_di_2-objs := 	mct_di_2.o \
								mct_di.o \
								mct_pin_di_objs.o \
								mct_pin_di_timer.o \
								mct_pin_di_sysfs.o
obj-$(CONFIG_MCT_PIN_DI) += mct_pin_di_3.o
		mct_pin_di_3-objs := 	mct_di_3.o \
								mct_di.o \
								mct_pin_di_objs.o \
								mct_pin_di_timer.o \
								mct_pin_di_sysfs.o
# modules
ifeq ($(CONFIG_MCT_PIN_DI),m)
 	mct_pin_di_0-objs += ../mct_json.o 
	mct_pin_di_1-objs += ../mct_json.o 
	mct_pin_di_2-objs += ../mct_json.o 
	mct_pin_di_3-objs += ../mct_json.o 
endif

# spezial pin-dev debugging
ifeq ($(CONFIG_MCT_PIN_DI_DEV_DEBUG),y)
	EXTRA_CFLAGS += -DMCT_PIN_DI_DEV_DEBUG
endif

#++++++++++++++++++
# interface MBUS
#++++++++++++++++++
ifeq ($(CONFIG_MCT_PIN_DI_DEV_IF_MBUS),y)
	EXTRA_CFLAGS += -DMCT_PIN_DI_DEV_IF_MBUS
	mct_pin_di_0-objs += 	mct_pin_di_mbus.o \
							mct_pin_di_mbus_cmd.o \
							../if/mbus_cl1.o \
							../if/mbus_cl2.o \
							../if/mbus_app.o \
							../if/mbus_if.o

	mct_pin_di_1-objs += 	mct_pin_di_mbus.o \
							mct_pin_di_mbus_cmd.o \
							../if/mbus_cl1.o \
							../if/mbus_cl2.o \
							../if/mbus_app.o \
							../if/mbus_if.o

	mct_pin_di_2-objs += 	mct_pin_di_mbus.o \
							mct_pin_di_mbus_cmd.o \
							../if/mbus_cl1.o \
							../if/mbus_cl2.o \
							../if/mbus_app.o \
							../if/mbus_if.o

	mct_pin_di_3-objs += 	mct_pin_di_mbus.o \
							mct_pin_di_mbus_cmd.o \
							../if/mbus_cl1.o \
							../if/mbus_cl2.o \
							../if/mbus_app.o \
							../if/mbus_if.o
endif

#++++++++++++++++++
# interface MODBUS
#++++++++++++++++++
ifeq ($(CONFIG_MCT_PIN_DI_DEV_IF_MODBUS),y)
	EXTRA_CFLAGS += -DMCT_PIN_DI_DEV_IF_MODBUS
	mct_pin_di_0-objs += 	mct_pin_di_modbus.o \
							mct_pin_di_modbus_cmd.o \
							../if/modbus_sl1.o \
							../if/modbus_sl2.o \
							../if/modbus_app.o \
							../if/modbus_if.o
	mct_pin_di_1-objs += 	mct_pin_di_modbus.o \
							mct_pin_di_modbus_cmd.o \
							../if/modbus_sl1.o \
							../if/modbus_sl2.o \
							../if/modbus_app.o \
							../if/modbus_if.o
	mct_pin_di_2-objs += 	mct_pin_di_modbus.o \
							mct_pin_di_modbus_cmd.o \
							../if/modbus_sl1.o \
							../if/modbus_sl2.o \
							../if/modbus_app.o \
							../if/modbus_if.o
	mct_pin_di_3-objs += 	mct_pin_di_modbus.o \
							mct_pin_di_modbus_cmd.o \
							../if/modbus_sl1.o \
							../if/modbus_sl2.o \
							../if/modbus_app.o \
							../if/modbus_if.o
endif



