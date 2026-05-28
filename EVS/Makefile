# The default test pattern is pattern 1
# 0 means not regenerate test pattern
PATTERN ?= 0
ifneq ($(PATTERN),0)
TARGET := $(shell ./data/bin/gendata.sh -p $(PATTERN) >/dev/null 2>&1 && echo evs_demo)
$(info Using data generated from pattern $(PATTERN))
else
TARGET := evs_demo
$(info Using pre-generated data...)
endif

NUCLEI_SDK_ROOT ?= ../../../..

ifeq ($(wildcard src),)
$(info src directory not found, running scripts/download_and_patch.sh...)
$(shell bash scripts/download_and_patch.sh >/dev/null 2>&1)
ifeq ($(wildcard src),)
$(error Failed to auto-generate src directory. Try running scripts/download_and_patch.sh manually)
endif
endif

SRCDIRS = . data \
		  src/c-code/basic_math \
		  src/c-code/basic_op \
		  src/c-code/lib_com \
		  src/c-code/lib_dec \
		  src/c-code/lib_enc

INCDIRS = . \
		  src/c-code/basic_math \
		  src/c-code/basic_op \
		  src/c-code/lib_com \
		  src/c-code/lib_dec \
		  src/c-code/lib_enc

STDCLIB ?= newlib_small

DOWNLOAD ?= ddr

COMMON_FLAGS := -O3

# Use the following flags to enable link-time optimization, but it is takes longer to build
# COMMON_FLAGS := -Ofast -flto=auto -ffast-math -funroll-loops
# LDFLAGS := -flto-partition=none

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
