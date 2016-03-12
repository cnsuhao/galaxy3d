LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:= libOpenAL
LOCAL_CFLAGS				:= -Wall -DAL_LIBTYPE_STATIC -DAL_ALEXT_PROTOTYPES -DHAVE_GCC_DESTRUCTOR
LOCAL_CPPFLAGS				:= -std=c++11 -fexceptions

LOCAL_C_INCLUDES			:= \
	../ \
    ../../OpenAL32/Include \
	../../include

LOCAL_SRC_FILES			:= \
    ../../Alc/ALc.c \
    ../../Alc/alcConfig.c \
    ../../Alc/alcDedicated.c \
    ../../Alc/alcEcho.c \
    ../../Alc/alcModulator.c \
    ../../Alc/alcReverb.c \
    ../../Alc/alcRing.c \
    ../../Alc/alcThreadCpp11.cpp \
    ../../Alc/ALc_cpp11.cpp \
    ../../Alc/ALu.c \
    ../../Alc/backends\loopback.c \
    ../../Alc/backends\null.c \
	../../Alc/backends\opensl.c \
    ../../Alc/backends\thread_msg_queue_cpp11.cpp \
    ../../Alc/bs2b.c \
    ../../Alc/helpers.c \
    ../../Alc/hrtf.c \
    ../../Alc/mixer.c \
    ../../Alc/panning.c \
    ../../OpenAL32/alAuxEffectSlot.c \
    ../../OpenAL32/alBuffer.c \
    ../../OpenAL32/alEffect.c \
    ../../OpenAL32/alError.c \
    ../../OpenAL32/alExtension.c \
    ../../OpenAL32/alFilter.c \
    ../../OpenAL32/alListener.c \
    ../../OpenAL32/alSource.c \
    ../../OpenAL32/alState.c \
    ../../OpenAL32/alThunk.c

include $(BUILD_STATIC_LIBRARY)
