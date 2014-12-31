#include "options.h"

MubotOptions mubot_options;

void default_options() {
  strcpy(mubot_options.config_file, "keybot.conf");
  mubot_options.midi_chan = 0;
  mubot_options.lock_controls = 0;
  mubot_options.store = 0;
  mubot_options.restore = 0;
}
