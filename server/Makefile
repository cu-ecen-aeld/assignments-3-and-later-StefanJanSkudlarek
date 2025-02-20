ofiles := $(wildcard *.o)
wfiles := $(wildcard aesdsocket)

all: aesdsocket

default: aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) $(CCFLAGS) -c aesdsocket.c

aesdsocket: aesdsocket.o
	$(CC) $(LDFLAGS) aesdsocket.o -o aesdsocket
	
.PHONY: clean
	 	
clean:
ifneq ($(ofiles), )
	rm $(ofiles)
endif
ifneq ($(wfiles), )
	rm $(wfiles)
endif 

