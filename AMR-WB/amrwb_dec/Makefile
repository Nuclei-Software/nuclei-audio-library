REGEN_DATA ?= 0
BITRATE ?= 6600

ifeq ($(REGEN_DATA), 1)
$(info Regenerating data...)
TARGET := $(shell ./../bin/gendata.sh $(BITRATE) >/dev/null 2>&1 && echo amrwb_dec)
$(info Generate data done)
else
$(info Using pre-generated data...)
TARGET := amrwb_dec
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . src

INCDIRS = . inc src ../data

STDCLIB ?= newlib_small

COMMON_FLAGS := -O3

CXXFLAGS += -std=c++11

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
