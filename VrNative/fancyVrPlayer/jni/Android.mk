LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include ../../VRLib/import_vrlib.mk		# import VRLib for this module.  Do NOT call $(CLEAR_VARS) until after building your module.
										# use += instead of := when defining the following variables: LOCAL_LDLIBS, LOCAL_CFLAGS, LOCAL_C_INCLUDES, LOCAL_STATIC_LIBRARIES 

include ../../VRLib/cflags.mk										
										
LOCAL_ARM_MODE := arm

LOCAL_MODULE    := ovrapp
LOCAL_SRC_FILES := MovieScreen.cpp ShaderManager.cpp OvrApp.cpp 

include $(BUILD_SHARED_LIBRARY)

# native activities need this, regular java projects don't
# $(call import-module,android/native_app_glue)

include $(CLEAR_VARS)
LOCAL_MODULE := libijkffmpeg
LOCAL_SRC_FILES := ../../../ijkplayer/libs/libijkffmpeg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libijkplayer
LOCAL_SRC_FILES := ../../../ijkplayer/libs/libijkplayer.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libijksdl
LOCAL_SRC_FILES := ../../../ijkplayer/libs/libijksdl.so
include $(PREBUILT_SHARED_LIBRARY)
