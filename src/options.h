#ifndef MUBOT_OPTIONS_H
#define MUBOT_OPTIONS_H

#include <string.h>

typedef struct {
  int midi_chan;           // 0 == all channels
  char config_file[256];   // default is "keybot.conf"
  int lock_controls;       // lock params and ignore MIDI controller messages
  int store;               // store parameters on exit
  int restore;             // restore parameters on startup
  int base_note;           // lowest note for keybot (MIDI code)
} MubotOptions;

struct Device {
    char file[256];
    struct Device *next;
};

extern MubotOptions mubot_options;
extern struct Device *devs;

void default_options(void);
void add_device(const char*);

#endif
