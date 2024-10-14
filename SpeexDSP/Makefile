TARGET = speexdsp_demo

NUCLEI_SDK_ROOT ?= ../../../..

CORE ?= n300

ARCH_EXT ?=

FIXED_POINT ?= 1

SRCDIRS = . src data

INCDIRS = . inc data

COMMON_FLAGS := -O2 -DHAVE_CONFIG_H

ifeq ($(FIXED_POINT), 1)
INCDIRS += data/fixed
COMMON_FLAGS += -DUSE_FIXED_POINT
else
INCDIRS += data/float
endif

LDLIBS = -lm

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
