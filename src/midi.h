#ifndef MIDI_H
#define MIDI_H

#include <alsa/asoundlib.h>
#include "options.h"

int        midi_action(snd_seq_t* seq_handle);
snd_seq_t *init_alsa();
int        open_port(snd_seq_t *seq_handle, const char *name);
int        drop_event(snd_seq_event_t *ev);

#endif
