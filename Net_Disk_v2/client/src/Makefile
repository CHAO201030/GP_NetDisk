Srcs := $(wildcard *.c)
Objs := $(patsubst %.c, %.o, $(Srcs))
Out  := client

CC := gcc
CFLAGS = -Wall -g -lpthread -lssl -lcrypto -lcrypt

$(Out) : $(Objs)
	$(CC) $^ -o $@ $(CFLAGS)
%.o: %.c algs.h
	$(CC) -c $< $(CFLAGS)

.PHONY: clean rebuild
clean:
	$(RM) $(Objs) $(Out)
rebuild: clean $(Out)
