CC=gcc
LIBS=-lasound arduino-serial/arduino-serial-lib.o
INCL=-Iarduino-serial -Isrc

EXEC=bin/keybot
SRC=src/keybot.c
TEST=src/test.c
CONFIG=~/.keybot/config

default all: keybot

keybot: dirs arduino-serial/checkout $(SRC)
	$(CC) -o $(EXEC) $(INCL) $(SRC) $(LIBS)

test: dirs $(TEST)
	$(CC) -o bin/test $(INCL) $(TEST) $(LIBS)

dirs:
	mkdir -p bin

arduino-serial/checkout:
	git submodule init
	git submodule update
	touch arduino-serial/checkout

clean:
	rm -f $(EXEC) *~ src/*~

config:
	if [ ! -d ~/.keybot ] ; \
	then \
		mkdir ~/.keybot; \
		echo "LO_STOP = 0" > $(CONFIG); \
		echo "HI_STOP = 32" >> $(CONFIG); \
		echo "SET_LO_CC = *:1" >> $(CONFIG); \
		echo "SET_HI_CC = *:2" >> $(CONFIG); \
		echo "SET_PANIC_CC = *:3" >> $(CONFIG); \
	fi;
