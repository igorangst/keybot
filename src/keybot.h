#ifndef KEYBOT_H
#define KEYBOT_H

#include "message.h"
#include "midi.h"
#include "options.h"

int  keybot_event(snd_seq_event_t *ev, int serial);
void keybot_setup(int serial);
void keybot_terminate(int serial);

#endif
