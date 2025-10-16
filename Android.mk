LOCAL_PATH := $(call my-dir)

ifneq ($(filter j5xn3g j5x3g j5xnlte j5xlte, $(TARGET_DEVICE)),)

include $(call all-subdir-makefiles,$(LOCAL_PATH))

endif
