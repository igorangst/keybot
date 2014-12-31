#ifndef MIDI_H
#define MIDI_H

#include <alsa/asoundlib.h>
#include "options.h"

snd_seq_t *open_seq();
int        drop_event(snd_seq_event_t *ev);

#endif
