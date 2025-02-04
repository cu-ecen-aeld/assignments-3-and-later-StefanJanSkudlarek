ofiles := $(wildcard *.o)
wfiles := $(wildcard writer)

CROSS_COMPILE =
COMP = aarch64-none-linux-gnu-

writer: writer.o
	echo "build executable from object"
ifeq ($(CROSS_COMPILE),)
	echo "no cross compiler option entered via make CROSS_COMPILE=option, native compilation."
	gcc -o writer writer.o
else ifeq ($(CROSS_COMPILE), $(COMP))
	echo "valid cross compiler option ${COMP} entered via make CROSS_COMPILE=option, cross compilation."
	aarch64-none-linux-gnu-gcc -o writer writer.o
else
	echo "invalid cross compiler option ${CROSS_COMPILE} entered via make CROSS_COMPILE=option, no cross compilation."
endif


writer.o: writer.c
	echo "build object from source"
ifeq ($(CROSS_COMPILE),)
	echo "no cross compiler option entered via make CROSS_COMPILE=option, native compilation."
	gcc -c writer.c	
else ifeq ($(CROSS_COMPILE), $(COMP))
	echo "valid cross compiler option ${COMP} entered via make CROSS_COMPILE=option, cross compilation."
	aarch64-none-linux-gnu-gcc -c writer.c	
else
	echo "invalid cross compiler option ${CROSS_COMPILE} entered via make CROSS_COMPILE=option, no cross compilation."
endif
	 	
clean:
ifneq ($(ofiles), )
	rm $(ofiles)
endif
ifneq ($(wfiles), )
	rm $(wfiles)
endif 

