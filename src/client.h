#ifndef CLIENT_H
#define CLIENT_H

#include "message.h"
#include "serial.h"

#define MUBOT_MAX_CLIENTS 8

typedef struct 
{
  int port;        // ALSA port id associated with this client
  int serial;      // serial port id associated with this client
  ClientType type; // keybot, drumbot, ...
} MubotClient;

ClientType probe(int serial);
void       init_clients();
void       detect_clients();

extern int         nclients;
extern MubotClient clients[8];

#endif
