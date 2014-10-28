#
# Makefile for the kernel mct_paa_di10 drivers
# Attention: Only build as kernel-modules!
#

ifeq ($(CONFIG_MCT_PAA_DI10),m)
	ifeq ($(CONFIG_MCT_PAA_SLOTS_MAX),6)
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_0.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_1.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_2.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_3.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_4.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_5.o
		mct_paa_di10_0-objs := 	mct_di10_0.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_1-objs := 	mct_di10_1.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_2-objs := 	mct_di10_2.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_3-objs := 	mct_di10_3.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_4-objs := 	mct_di10_4.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_5-objs := 	mct_di10_5.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_0-objs 		+= ../../mct_json.o
		mct_paa_di10_1-objs 		+= ../../mct_json.o
		mct_paa_di10_2-objs 		+= ../../mct_json.o
		mct_paa_di10_3-objs 		+= ../../mct_json.o
		mct_paa_di10_4-objs 		+= ../../mct_json.o
		mct_paa_di10_5-objs 		+= ../../mct_json.o
		#++++++++++++++++++
		# interface MBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MBUS
			mct_paa_di10_0-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_1-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_2-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_3-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_4-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_5-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
		endif
		#++++++++++++++++++
		# interface MODBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MODBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MODBUS
			mct_paa_di10_0-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_1-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_2-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_3-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_4-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_5-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
		endif
	endif
	ifeq ($(CONFIG_MCT_PAA_SLOTS_MAX),5)
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_0.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_1.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_2.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_3.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_4.o
		mct_paa_di10_0-objs := 	mct_di10_0.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_1-objs := 	mct_di10_1.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_2-objs := 	mct_di10_2.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_3-objs := 	mct_di10_3.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_4-objs := 	mct_di10_4.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_0-objs 		+= ../../mct_json.o
		mct_paa_di10_1-objs 		+= ../../mct_json.o
		mct_paa_di10_2-objs 		+= ../../mct_json.o
		mct_paa_di10_3-objs 		+= ../../mct_json.o
		mct_paa_di10_4-objs 		+= ../../mct_json.o
		#++++++++++++++++++
		# interface MBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MBUS
			mct_paa_di10_0-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_1-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_2-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_3-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_4-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
		endif
		#++++++++++++++++++
		# interface MODBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MODBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MODBUS
			mct_paa_di10_0-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_1-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_2-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_3-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_4-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
		endif
	endif
	ifeq ($(CONFIG_MCT_PAA_SLOTS_MAX),4)
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_0.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_1.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_2.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_3.o
		mct_paa_di10_0-objs := 	mct_di10_0.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_1-objs := 	mct_di10_1.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_2-objs := 	mct_di10_2.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_3-objs := 	mct_di10_3.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_0-objs 		+= ../../mct_json.o
		mct_paa_di10_1-objs 		+= ../../mct_json.o
		mct_paa_di10_2-objs 		+= ../../mct_json.o
		mct_paa_di10_3-objs 		+= ../../mct_json.o
		#++++++++++++++++++
		# interface MBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MBUS
			mct_paa_di10_0-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_1-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_2-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_3-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
		endif
		#++++++++++++++++++
		# interface MODBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MODBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MODBUS
			mct_paa_di10_0-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_1-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_2-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_3-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
		endif
	endif
	ifeq ($(CONFIG_MCT_PAA_SLOTS_MAX),3)
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_0.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_1.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_2.o
		mct_paa_di10_0-objs := 	mct_di10_0.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_1-objs := 	mct_di10_1.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_2-objs := 	mct_di10_2.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_0-objs 		+= ../../mct_json.o
		mct_paa_di10_1-objs 		+= ../../mct_json.o
		mct_paa_di10_2-objs 		+= ../../mct_json.o
		#++++++++++++++++++
		# interface MBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MBUS
			mct_paa_di10_0-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_1-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_2-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
		endif
		#++++++++++++++++++
		# interface MODBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MODBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MODBUS
			mct_paa_di10_0-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_1-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_2-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
		endif
	endif
	ifeq ($(CONFIG_MCT_PAA_SLOTS_MAX),2)
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_0.o
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_1.o
		mct_paa_di10_0-objs := 	mct_di10_0.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_1-objs := 	mct_di10_1.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_0-objs 		+= ../../mct_json.o
		mct_paa_di10_1-objs 		+= ../../mct_json.o
		#++++++++++++++++++
		# interface MBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MBUS
			mct_paa_di10_0-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
			mct_paa_di10_1-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
		endif
		#++++++++++++++++++
		# interface MODBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MODBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MODBUS
			mct_paa_di10_0-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
			mct_paa_di10_1-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
		endif
	endif
	ifeq ($(CONFIG_MCT_PAA_SLOTS_MAX),1)
		obj-$(CONFIG_MCT_PAA_DI10) += mct_paa_di10_0.o
		mct_paa_di10_0-objs := 	mct_di10_0.o mct_di10.o mct_di10_objs.o mct_di10_sysfs.o mct_di10_sysfs.o
		mct_paa_di10_0-objs 		+= ../../mct_json.o
		#++++++++++++++++++
		# interface MBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MBUS
			mct_paa_di10_0-objs += mct_di10_mbus.o \
								mct_di10_mbus_cmd.o \
								../../if/mbus_cl1.o \
								../../if/mbus_cl2.o \
								../../if/mbus_app.o	\
								../../if/mbus_if.o
		endif
		#++++++++++++++++++
		# interface MODBUS
		#++++++++++++++++++
		ifeq ($(CONFIG_MCT_PAA_DI10_DEV_IF_MODBUS),y)
			EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_IF_MODBUS
			mct_paa_di10_0-objs += mct_di10_modbus.o \
								mct_di10_modbus_cmd.o \
								../../if/modbus_sl1.o \
								../../if/modbus_sl2.o \
								../../if/modbus_app.o \
								../../if/modbus_if.o
		endif
	endif
endif #$(CONFIG_MCT_PAA_DI10),m

# spezial di10-dev debugging
ifeq ($(CONFIG_MCT_PAA_DI10_DEV_DEBUG),y)
	EXTRA_CFLAGS += -DMCT_PAA_DI10_DEV_DEBUG
endif

