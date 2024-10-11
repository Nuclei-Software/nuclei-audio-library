TARGET = lc3plus_demo

NUCLEI_SDK_ROOT ?= ../../../..

CORE ?= n300

ARCH_EXT ?=

FIXED_POINT ?= 1

SRCDIRS = . data

INCDIRS = . data

COMMON_FLAGS := -O2

ifeq ($(FIXED_POINT), 1)
SRCDIRS += fixed_point fixed_point/basic_op
INCDIRS += fixed_point fixed_point/basic_op data/fixed
COMMON_FLAGS += -DDISABLE_HR_MODE -DWMOPS=0 -DDONT_COUNT_MEM -DFIXED_POINT
else
SRCDIRS += floating_point
INCDIRS += floating_point floating_point/fft data/float
endif

LDLIBS = -lm

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
