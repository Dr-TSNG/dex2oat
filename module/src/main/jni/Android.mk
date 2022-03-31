LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE           := dex2oat
LOCAL_SRC_FILES        := dex2oat.cpp utils.cpp
LOCAL_STATIC_LIBRARIES := cxx
LOCAL_LDLIBS           := -llog
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE           := dex2oatd
LOCAL_SRC_FILES        := daemon.cpp utils.cpp
LOCAL_STATIC_LIBRARIES := cxx
LOCAL_LDLIBS           := -llog
include $(BUILD_EXECUTABLE)

$(call import-module,prefab/cxx)
