TARGET := minimp3_test

NUCLEI_SDK_ROOT ?= ../../../..

# The default test pattern is pattern 1
# 0 means not regenerate test pattern
PATTERN ?= 0
ifneq ($(PATTERN),0)
$(shell ./data/bin/gendata.sh -p $(PATTERN) >/dev/null 2>&1)
$(info Using data generated from pattern $(PATTERN))
else
$(info Using pre-generated data...)
endif

SRCDIRS = . data

INCDIRS = .

STDCLIB ?= newlib_small

DOWNLOAD ?= ddr

COMMON_FLAGS := -O3

LDLIBS := -lm

# Use the following flags to enable link-time optimization, but it is takes longer to build
# COMMON_FLAGS := -Ofast -flto=auto -ffast-math -funroll-loops
# LDFLAGS := -flto-partition=none

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
