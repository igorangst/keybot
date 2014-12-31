#ifndef MUBOT_OPTIONS_H
#define MUBOT_OPTIONS_H

#include <string.h>

typedef struct{
  int midi_chan;           // 0 == all channels
  char config_file[256];   // default is "keybot.conf"
  int lock_controls;       // lock params and ignore MIDI controller messages
  int store;               // store parameters on exit
  int restore;             // restore parameters on startup
} MubotOptions;

extern MubotOptions mubot_options;

void default_options(){
  strcpy(mubot_options.config_file, "keybot.conf");
  mubot_options.midi_chan = 0;
  mubot_options.lock_controls = 0;
  mubot_options.store = 0;
  mubot_options.restore = 0;
}

#endif
