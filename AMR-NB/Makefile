TARGET := demo_amrnb

# The default bitrate is 12200
# available bitrate: 4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200
# BITRATE=0 means not regenerate test data
BITRATE ?= 0
ifneq ($(BITRATE), 0)
$(info Using data generated with bitrate $(BITRATE))
$(shell ./data/bin/gendata.sh $(BITRATE) >/dev/null 2>&1)
else
$(info Using pre-generated data...)
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . enc/src dec/src data common/src

INCDIRS = . enc/src dec/src enc/include dec/include data common/include oscl

STDCLIB ?= newlib_small

COMMON_FLAGS := -O3

CXXFLAGS += -std=c++11

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
