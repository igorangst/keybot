#ifndef SERIAL_H
#define SERIAL_H

int open_serial(char* dev, int brate){
  int serial = -1;
  serial = serialport_init(dev, brate);
  if (serial == -1){
    return -1;
  }
  usleep(1000); // FIXME
  serialport_flush(serial);
  printf("Connected to serial port '%s'\n", dev);
  return serial;
}

void close_serial(int serial){
  serialport_close(serial);
}

#endif
