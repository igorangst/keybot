#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include "arduino-serial-lib.h"

int open_serial(char* dev, int brate);
void close_serial(int serial);

#endif
