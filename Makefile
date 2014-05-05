CC=gcc
LIBS=-lasound arduino-serial/arduino-serial-lib.o
INCL=-Iarduino-serial -Isrc

EXEC=bin/keybot
SRC=src/keybot.c

default all: keybot

keybot: dirs arduino-serial/checkout $(SRC)
	$(CC) -o $(EXEC) $(INCL) $(SRC) $(LIBS)

dirs:
	mkdir -p bin

arduino-serial/checkout:
	git submodule init
	git submodule update
	touch arduino-serial/checkout

clean:
	rm -f $(EXEC) *~ src/*~

