SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=client
CC:=gcc
CFLAGS:=-g -Wall -lcrypt -lcrypto
$(ELF):$(OBJS)
	gcc $^ -o $@ $(CFLAGS)
.PHONY:clean
clean:
	rm -rf $(ELF) $(OBJS)
