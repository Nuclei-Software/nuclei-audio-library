# NOTE: make sure the path of speexdsp.pc is in PKG_CONFIG_PATH
CFLAGS += $(shell pkg-config --cflags --static speexdsp)
LDLIBS += $(shell pkg-config --libs --static speexdsp)

CC = gcc

ifeq ($(FIXED_POINT), 1)
CFLAGS += -DFIXED_POINT
endif

.PHONY: all clean

all: speexdsp_demo

clean: 
	rm -f speexdsp_demo

speexdsp_demo: speexdsp_demo.c
	$(CC) -o $@ $< $(CFLAGS) -g -O2 -static $(LDLIBS) 
