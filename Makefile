CC=gcc
#CFLAGS=-O1 -Werror -Wall
CFLAGS=-g -Werror -Wall
LIBS=-lasound arduino-serial/arduino-serial-lib.o
INCL=-Iarduino-serial -Isrc

EXEC=bin/mubot
MAIN=src/main.c
SRCS=src/keybot.c src/drumbot.c src/fishbot.c src/dummybot.c src/midi.c src/options.c src/client.c src/serial.c
OBJS=$(subst .c,.o,$(SRCS))
CONFIG=~/.keybot/config

default all: mubot

mubot: dirs arduino-serial/checkout $(SRC) exec

exec: $(MAIN) $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(INCL) $(MAIN) $(OBJS) $(LIBS)

dirs:
	mkdir -p bin

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $< $(INCL)

arduino-serial/checkout:
	git submodule init
	git submodule update
	touch arduino-serial/checkout

clean:
	rm -f $(EXEC) $(OBJS) *~ src/*~

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
