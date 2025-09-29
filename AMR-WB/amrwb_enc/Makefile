REGEN_DATA ?= 0
BITRATE ?= 6600

ifeq ($(REGEN_DATA), 1)
$(info Regenerating data...)
TARGET := $(shell ./../bin/gendata.sh $(BITRATE) >/dev/null 2>&1 && echo amrwb_enc)
$(info Generate data done)
else
$(info Using pre-generated data...)
TARGET := amrwb_enc
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . src

INCDIRS = . inc ../data

STDCLIB ?= newlib_full

COMMON_FLAGS := -O3

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
