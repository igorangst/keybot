#ifndef DRUMBOT_H
#define DRUMBOT_H

#include "message.h"
#include "midi.h"
#include "options.h"

int drumbot_event(snd_seq_event_t *ev, int serial);
 
#endif
