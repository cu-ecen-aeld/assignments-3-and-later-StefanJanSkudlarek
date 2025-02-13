ofiles := $(wildcard *.o)
wfiles := $(wildcard writer)

all: writer

writer.o: writer.c
	$(CC) $(CCFLAGS) -c writer.c

writer: writer.o
	$(CC) $(LDFLAGS) writer.o -o writer
	
.PHONY: clean
	 	
clean:
ifneq ($(ofiles), )
	rm $(ofiles)
endif
ifneq ($(wfiles), )
	rm $(wfiles)
endif 

