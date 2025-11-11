# SPDX-License-Identifier: GPL-2.0-only
ccflags-y := -I$(srctree)/drivers/staging/media/eswin/es-media-ext
ccflags-y += -I$(srctree)/drivers/staging/media/eswin/es-media-ext/common/
ccflags-y += -I$(srctree)/drivers/staging/media/eswin/es-media-ext/module/
ccflags-y += -I$(srctree)/drivers/staging/media/eswin/es-media-ext/chn/
ccflags-y += -I$(srctree)/drivers/staging/media/eswin/es-media-ext/proc/
ccflags-y += -I$(srctree)/drivers/staging/media/eswin/es-media-ext/include/

es_media_ext_drv-objs := es_media_ext_drv_main.o \
						./chn/dev_channel.o	\
						./chn/class_chn_mgr.o \
						./module/dev_module.o \
						./module/class_module.o \
						./module/class_event.o \
						./proc/dev_proc.o \
						./proc/class_proc_data.o \
						./proc/class_proc_module.o \
						./proc/class_section.o \
						./proc/class_section_mgr.o \
						./common/dev_common.o

obj-$(CONFIG_VIDEO_ESWIN_MEDIA_EXT_DRIVER) += es_media_ext_drv.o
