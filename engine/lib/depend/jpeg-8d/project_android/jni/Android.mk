LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:= libJpeg
LOCAL_CFLAGS				:= -Wall
LOCAL_SRC_FILES			:= \
    ../../../Galaxy3D/Lib/jpeg-8d/jaricom.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcapimin.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcapistd.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcarith.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jccoefct.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jccolor.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcdctmgr.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jchuff.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcinit.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcmainct.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcmarker.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcmaster.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcomapi.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcparam.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcprepct.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jcsample.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jctrans.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdapimin.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdapistd.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdarith.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdatadst.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdatasrc.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdcoefct.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdcolor.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jddctmgr.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdhuff.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdinput.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdmainct.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdmarker.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdmaster.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdmerge.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdpostct.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdsample.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jdtrans.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jerror.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jfdctflt.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jfdctfst.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jfdctint.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jidctflt.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jidctfst.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jidctint.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jmemmgr.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jmemnobs.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jquant1.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jquant2.c \
    ../../../Galaxy3D/Lib/jpeg-8d/jutils.c

LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_STATIC_LIBRARY)
