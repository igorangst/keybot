#ifndef FISHBOT_H
#define FISHBOT_H

#include "message.h"
#include "midi.h"
#include "options.h"

int fishbot_event(snd_seq_event_t *ev, int serial);
 
#endif
