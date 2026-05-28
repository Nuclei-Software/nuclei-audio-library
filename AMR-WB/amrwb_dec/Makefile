TARGET := amrwb_dec

# The default bitrate is 6600
# available bitrate: 6600, 8850, 12650, 14250, 15850, 18250, 19850, 23050, 23850
# BITRATE=0 means not regenerate test data
BITRATE ?= 0
ifneq ($(BITRATE), 0)
$(info Using data generated with bitrate $(BITRATE))
$(shell ./../bin/gendata.sh $(BITRATE) >/dev/null 2>&1)
else
$(info Using pre-generated data...)
endif

NUCLEI_SDK_ROOT ?= ../../../..

SRCDIRS = . src

INCDIRS = . inc src ../data

STDCLIB ?= newlib_small

COMMON_FLAGS := -O3

CXXFLAGS += -std=c++11

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
