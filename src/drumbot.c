#include "drumbot.h"
#include "arduino-serial-lib.h"

int drumbot_event(snd_seq_event_t *ev, int serial){
  int stop = 0;

  switch (ev->type){
  case SND_SEQ_EVENT_NOTEON:
    printf("BAM!\n");
    serialport_writebyte(serial, NOTE_ON);
    usleep(1000);
    unsigned char velocity = ev->data.note.velocity;
    serialport_writebyte(serial, velocity);
  }
  snd_seq_free_event(ev);

  return stop;
}
