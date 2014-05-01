CC=gcc
LIBS=-lasound lib/arduino-serial-lib.o
INCL=-I. -I../arduino-serial 

EXEC=bin/keybot
SRC=keybot.c

default all: keybot

keybot: dirs $(SRC)
	$(CC) -o $(EXEC) $(INCL) $(SRC) $(LIBS)

dirs:
	mkdir -p bin

clean:
	rm -f $(EXEC) *~
