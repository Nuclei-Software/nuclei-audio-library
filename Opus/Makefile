TARGET = opus_demo

NUCLEI_SDK_ROOT ?= ../../../..

CORE ?= n300

ARCH_EXT ?=

FIXED_POINT ?= 1

SRCDIRS = . src celt silk data

INCDIRS = . celt include silk src data

COMMON_FLAGS := -O3 -DHAVE_CONFIG_H

ifeq ($(FIXED_POINT), 1)
COMMON_FLAGS += -DUSE_FIXED_POINT -DRISCV_ASM
SRCDIRS += silk/fixed
INCDIRS += silk/fixed
else
SRCDIRS += silk/float
INCDIRS += silk/float
endif

LDLIBS = -lm

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
