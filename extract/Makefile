DEBUG = 
CFLAGS = $(if $(DEBUG),-g,-Os) -std=c89 -Wall

PROGRAMS = avipart avidump

.PHONY: all debug clean

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

avipart: avipart.o avi.o
	$(CC) -o $@ $^

avidump: avidump.o avi.o
	$(CC) -o $@ $^

%.o: %.c avi.h
	$(CC) $(CFLAGS) -o $@ -c $<
