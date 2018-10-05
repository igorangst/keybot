#ifndef CLIENT_H
#define CLIENT_H

#include "message.h"
#include "serial.h"
#include "arduino-serial-lib.h"

// Client implementations
#include "drumbot.h"
#include "keybot.h"
#include "fishbot.h"
#include "dummybot.h"

#define MUBOT_MAX_CLIENTS 8

typedef struct 
{
  int port;        // ALSA port id associated with this client
  int serial;      // serial port id associated with this client
  ClientType type; // keybot, drumbot, ...
} MubotClient;

extern int         nclients;   // number of active clients
extern MubotClient clients[8]; // global array of clients

void       detect_clients();         // detect clients and populate client array
void       create_ports(snd_seq_t*); // create ALSA ports for detected clients
void       setup_clients();          // perform client specific setup routines
void       terminate_clients();      // stop all clients

#endif
