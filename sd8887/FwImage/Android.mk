LOCAL_PATH := $(call my-dir)
#
#PLATFORM_SDK_VERSION >= 14, 14 is 4.0.1_r1
#
ifneq ($(findstring $(PLATFORM_SDK_VERSION),1 2 3 4 5 6 7 8 9 10 11 12 13),)
wifi_firmware1 := $(TARGET_OUT_ETC)/firmware/mrvl/sd8887_uapsta.bin
$(wifi_firmware1) : $(LOCAL_PATH)/sd8887_uapsta.bin | $(ACP)
	$(transform-prebuilt-to-target)
wifi_firmware1 := $(TARGET_OUT_ETC)/firmware/mrvl/sd8787_uapsta.bin
$(wifi_firmware2) : $(LOCAL_PATH)/sd8787_uapsta.bin | $(ACP)
	$(transform-prebuilt-to-target)
wifi_firmware2 := $(TARGET_OUT_ETC)/firmware/mrvl/sd8787_uapsta.bin
$(wifi_firmware3) : $(LOCAL_PATH)/sd8777_uapsta.bin | $(ACP)
	$(transform-prebuilt-to-target)

ALL_PREBUILT += $(wifi_firmware1)
ALL_PREBUILT += $(wifi_firmware2)
ALL_PREBUILT += $(wifi_firmware3)

endif

