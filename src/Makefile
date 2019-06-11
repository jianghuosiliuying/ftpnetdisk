SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=netdiskserver
CC:=gcc
CFLAGS:=-g -Wall -pthread -I /usr/include/mysql/ -lmysqlclient
$(ELF):$(OBJS)
	gcc $^ -o $@ $(CFLAGS)
.PHONY:clean
clean:
	rm -rf $(ELF) $(OBJS)
