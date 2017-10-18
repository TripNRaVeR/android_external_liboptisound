#
# Copyright 2017 TripNDroid Mobile Engineering
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liboptisound
LOCAL_MODULE_RELATIVE_PATH := soundfx

LOCAL_SRC_FILES := \
	optisound.cpp \
	Effect.cpp \
	EffectDSPMain.cpp \
	iir/Biquad.cpp \
	iir/Butterworth.cpp \
	iir/Cascade.cpp \
	iir/PoleFilter.cpp \
	iir/RootFinder.cpp \
	firgen.c \
	gverb.c \
	reverb.c \
	compressor.c \
	libHybridConv.c \
	Tube.c \
	wdfcircuits_triode.c

LOCAL_C_INCLUDES := \
    external/libfftw3/fftw3/include

LOCAL_SHARED_LIBRARIES := libfftw3

include $(BUILD_SHARED_LIBRARY)