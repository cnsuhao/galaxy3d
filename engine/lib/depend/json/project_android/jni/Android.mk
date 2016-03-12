LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:= libJSON
LOCAL_CFLAGS				:= -Wall

LOCAL_C_INCLUDES			:= ../../src

LOCAL_SRC_FILES				:= \
    ../../src/json/json_reader.cpp \
	../../src/json/json_value.cpp \
	../../src/json/json_writer.cpp

LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_STATIC_LIBRARY)
