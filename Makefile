CC=gcc
LIBS=-lasound arduino-serial/arduino-serial-lib.o
INCL=-Iarduino-serial -Isrc

EXEC=bin/keybot
SRC=src/keybot.c

default all: keybot

keybot: dirs $(SRC)
	$(CC) -o $(EXEC) $(INCL) $(SRC) $(LIBS)

dirs:
	mkdir -p bin

clean:
	rm -f $(EXEC) *~ src/*~
