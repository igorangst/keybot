#ifndef CLIENT_H
#define CLIENT_H

#include "message.h"
#include "serial.h"

typedef struct 
{
  int port;        // ALSA port id associated with this client
  int serial;      // serial port id associated with this client
  ClientType type; // keybot, drumbot, ...
} MubotClient;


ClientType probe(int serial) {
  ClientType t = NOBODY;
  printf("Probing client... ");
  serialport_writebyte(serial, WHORU);
  usleep(1000);
  char answer[32];
  int n = serialport_read_until(serial, answer, IAM, 2, 500);

  if (n == -1 || answer[1] != IAM){
    printf("No known client connected\n");
    t = NOBODY;
  } else if (answer[0] == KEYBOT){
    printf("Detected KEYBOT\n");
    t = KEYBOT;
  } else if (answer[0] == DRUMBOT){
    printf("Detected DRUMBOT\n");
    t = DRUMBOT;
  } else {
    printf("Unknown client ID: %i\n", answer[0]);
  }

  return t;
}


ClientType probe_client(char* dev, unsigned brate) {
  int serial = open_serial(dev, brate);
  if (serial == -1){
    return NOBODY;
  }

  ClientType t = NOBODY;
  serialport_writebyte(serial, WHORU);
  usleep(1000);
  char answer[32];
  int n = serialport_read_until(serial, answer, DUMP_EOF, 3, 5000);

  if (n < 2 || answer[0] != IAM){
    printf("No known client connected to device '%s' (%i,%c)\n", dev, n, answer[0]);
    t = NOBODY;
  } else if (answer[1] == KEYBOT){
    printf("Detected KEYBOT connected to device '%s'\n", dev);
    t = KEYBOT;
  } else if (answer[1] == DRUMBOT){
    printf("Detected DRUMBOT connected to device '%s'\n", dev);
    t = DRUMBOT;
  }

  close_serial(serial);
  return t;
}

#endif
