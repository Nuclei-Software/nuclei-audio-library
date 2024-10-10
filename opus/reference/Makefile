# NOTE: make sure the path of opus.pc is in PKG_CONFIG_PATH
CFLAGS += $(shell pkg-config --cflags --static opus)
LDLIBS += $(shell pkg-config --libs --static opus)

CC = gcc

.PHONY: all clean

all: opus_demo

clean: 
	rm -f opus_demo

opus_demo: opus_demo.c
	$(CC) -o $@ $< $(CFLAGS) $(LDLIBS) -g
