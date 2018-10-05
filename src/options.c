#include "options.h"
#include <stdlib.h>

MubotOptions mubot_options;
struct Device *devs;

void default_options() {
  strcpy(mubot_options.config_file, "keybot.conf");
  mubot_options.midi_chan = 0;
  mubot_options.lock_controls = 0;
  mubot_options.store = 0;
  mubot_options.restore = 0;
  mubot_options.base_note = 48;
}

void add_device(const char *file){
    struct Device *dev = malloc(sizeof(struct Device));
    strcpy(dev->file, file);
    dev->next = devs;
    devs = dev;
}
