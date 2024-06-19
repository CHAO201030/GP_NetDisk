Srcs := $(wildcard *.c)
Objs := $(patsubst %.c, %.o, $(Srcs))
Out  := route_server

CC := gcc
CFLAGS = -Wall -g -lpthread -lssl -lcrypto -lcrypt -lmysqlclient -ll8w8jwt

$(Out) : $(Objs)
	$(CC) $^ -o $@ $(CFLAGS)
%.o: %.c 
	$(CC) -c $< $(CFLAGS)

.PHONY: clean rebuild
clean:
	$(RM) $(Objs) $(Out)
rebuild: clean $(Out)
