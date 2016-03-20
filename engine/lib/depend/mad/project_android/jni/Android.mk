LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:= libMad
LOCAL_CFLAGS				:= -Wall -DFPM_ARM
LOCAL_ARM_MODE				:= arm

LOCAL_C_INCLUDES			:= ../../

LOCAL_SRC_FILES				:= \
	../../bit.c \
    ../../decoder.c \
    ../../fixed.c \
    ../../frame.c \
    ../../huffman.c \
    ../../id3tag\compat.c \
    ../../id3tag\crc.c \
    ../../id3tag\field.c \
    ../../id3tag\file.c \
    ../../id3tag\id3_frame.c \
    ../../id3tag\frametype.c \
    ../../id3tag\genre.c \
    ../../id3tag\latin1.c \
    ../../id3tag\parse.c \
    ../../id3tag\render.c \
    ../../id3tag\tag.c \
    ../../id3tag\ucs4.c \
    ../../id3tag\utf16.c \
    ../../id3tag\utf8.c \
    ../../id3tag\util.c \
    ../../id3tag\id3_version.c \
    ../../layer12.c \
    ../../layer3.c \
    ../../stream.c \
    ../../synth.c \
    ../../timer.c \
    ../../version.c

include $(BUILD_STATIC_LIBRARY)
