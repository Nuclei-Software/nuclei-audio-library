TARGET := aac_dec

# The default test pattern is pattern 1
# 0 means not regenerate test pattern
PATTERN ?= 0
ifneq ($(PATTERN), 0)
$(info Using data generated from pattern $(PATTERN))
TARGET := $(shell ./../bin/gendata.sh -p $(PATTERN) >/dev/null 2>&1 && echo aac_dec)
else
$(info Using pre-generated data...)
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . frontend libfaad

INCDIRS = . frontend include libfaad ../data

STDCLIB ?= newlib_small

DOWNLOAD ?= ddr

COMMON_FLAGS := -O3 -DHAVE_CONFIG_H

LDLIBS = -lm

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
