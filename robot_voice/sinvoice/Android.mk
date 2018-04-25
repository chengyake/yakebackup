LOCAL_PATH:= $(call my-dir)

########################################
# NXP Configuration
########################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
        $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := myvoice
#LOCAL_CERTIFICATE := platform
#LOCAL_REQUIRED_MODULES  := libnfc_jni

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)
