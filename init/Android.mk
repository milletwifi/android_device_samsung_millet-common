LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := system/core/init system/core/healthd bootable/recovery
LOCAL_CFLAGS := -Wall
LOCAL_SRC_FILES := init_millet.cpp
LOCAL_MODULE := libinit_millet

include $(BUILD_STATIC_LIBRARY)
