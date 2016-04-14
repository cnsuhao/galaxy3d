LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libFreeType
LOCAL_SRC_FILES				:=	../../../lib/depend/freetype-2.6/project_android/obj/local/$(TARGET_ARCH_ABI)/libFreeType.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libJPEG
LOCAL_SRC_FILES				:=	../../../lib/depend/jpeg-8d/project_android/obj/local/$(TARGET_ARCH_ABI)/libJPEG.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libPNG
LOCAL_SRC_FILES				:=	../../../lib/depend/lpng1512/project_android/obj/local/$(TARGET_ARCH_ABI)/libPNG.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libJSON
LOCAL_SRC_FILES				:=	../../../lib/depend/json/project_android/obj/local/$(TARGET_ARCH_ABI)/libJSON.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libBullet
LOCAL_SRC_FILES				:=	../../../lib/depend/bullet3-2.83.6/project_android/obj/local/$(TARGET_ARCH_ABI)/libBullet.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libOpenAL
LOCAL_SRC_FILES				:=	../../../lib/depend/openal-soft-winphone-master/project_android/obj/local/$(TARGET_ARCH_ABI)/libOpenAL.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libViry3D
LOCAL_SRC_FILES				:=	../../../lib/project_android/obj/local/$(TARGET_ARCH_ABI)/libViry3D.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:=	libGame
LOCAL_CFLAGS				:=	-Wall -DANDROID
LOCAL_CPPFLAGS				:=	-std=c++11 -fexceptions -frtti

LOCAL_C_INCLUDES			:=	\
	../../../lib/src \
	../../../lib/src/gles2

LOCAL_SRC_FILES				:=	\
	jni.cpp \
	../../src/LauncherMerged.cpp

LOCAL_STATIC_LIBRARIES		:=	libViry3D libJSON libBullet libOpenAL libPNG libJPEG libFreeType
LOCAL_LDLIBS				:=	-llog -lGLESv2 -lz -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
