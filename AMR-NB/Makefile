REGEN_DATA ?= 0
BITRATE ?= 12200

ifeq ($(REGEN_DATA), 1)
$(info Regenerating data...)
TARGET := $(shell ./data/bin/gendata.sh $(BITRATE) >/dev/null 2>&1 && echo demo_amrnb)
$(info Generate data done)
else
$(info Using pre-generated data...)
TARGET := demo_amrnb
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . enc/src dec/src data common/src

INCDIRS = . enc/src dec/src enc/include dec/include data common/include oscl

STDCLIB ?= newlib_small

COMMON_FLAGS := -O3

CXXFLAGS += -std=c++11

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
