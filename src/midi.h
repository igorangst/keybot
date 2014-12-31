#ifndef MIDI_H
#define MIDI_H

#include <alsa/asoundlib.h>
#include "options.h"

extern snd_seq_t     *seq_handle;
extern struct pollfd *pfd;
extern int            npfd;

int        midi_action(snd_seq_t* seq_handle);
void       init_alsa();
snd_seq_t *open_seq();
int        open_port(const char *name);
int        drop_event(snd_seq_event_t *ev);

#endif
