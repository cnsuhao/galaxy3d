LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:= libPng
LOCAL_CFLAGS				:= -Wall
LOCAL_SRC_FILES			:= \
    ../../../Galaxy3D/Lib/lpng1512/png.c \
    ../../../Galaxy3D/Lib/lpng1512/pngerror.c \
    ../../../Galaxy3D/Lib/lpng1512/pngget.c \
    ../../../Galaxy3D/Lib/lpng1512/pngmem.c \
    ../../../Galaxy3D/Lib/lpng1512/pngpread.c \
    ../../../Galaxy3D/Lib/lpng1512/pngread.c \
    ../../../Galaxy3D/Lib/lpng1512/pngrio.c \
    ../../../Galaxy3D/Lib/lpng1512/pngrtran.c \
    ../../../Galaxy3D/Lib/lpng1512/pngrutil.c \
    ../../../Galaxy3D/Lib/lpng1512/pngset.c \
    ../../../Galaxy3D/Lib/lpng1512/pngtrans.c \
    ../../../Galaxy3D/Lib/lpng1512/pngwio.c \
    ../../../Galaxy3D/Lib/lpng1512/pngwrite.c \
    ../../../Galaxy3D/Lib/lpng1512/pngwtran.c \
    ../../../Galaxy3D/Lib/lpng1512/pngwutil.c

LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_STATIC_LIBRARY)
