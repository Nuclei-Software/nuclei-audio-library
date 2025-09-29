REGEN_DATA ?= 0
PATTERN ?= 1

ifeq ($(REGEN_DATA), 1)
$(info Regenerating data...)
TARGET := $(shell ./data/bin/gendata.sh -p $(PATTERN) >/dev/null 2>&1 && echo evs_demo)
$(info Generate data done)
else
$(info Using pre-generated data...)
TARGET := evs_demo
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . basic_math basic_op data lib_com lib_dec lib_enc

INCDIRS = . basic_math basic_op lib_com lib_dec lib_enc

STDCLIB ?= newlib_small

DOWNLOAD ?= ddr

COMMON_FLAGS := -O3

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
