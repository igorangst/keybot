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

extern int         nclients;   // number of active clients
extern MubotClient clients[8]; // global array of clients

void       detect_clients();
void       setup_clients();
void       terminate_clients();

#endif
