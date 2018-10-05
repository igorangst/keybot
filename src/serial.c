#include "serial.h"
#include "unistd.h"

int open_serial(char* dev, int brate){
  int serial = -1;
  printf("Connecting to serial port '%s'...\n", dev);
  serial = serialport_init(dev, brate);
  if (serial == -1){
      printf("FAILED.\n");
      return -1;
  }
  usleep(1000); // FIXME
  serialport_flush(serial);
  printf("CONNECTED.\n");
  return serial;
}

void close_serial(int serial){
  serialport_close(serial);
}
