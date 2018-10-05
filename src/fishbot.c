#include "fishbot.h"
#include "arduino-serial-lib.h"

/******************************************************************
 * Function: char get_chime(int note)
 * Simple conversion from MIDI notes to chime indices. Per default,
 * the chimes cover the white keys of one octave from C to C.
 * @return: chime index between -1 (out of range) and 7
 ******************************************************************/
char get_chime(int note){
  switch (note){
  case 60: return 0;
  case 62: return 1;
  case 64: return 2;
  case 65: return 3;
  case 67: return 4;
  case 69: return 5;
  case 71: return 6;
  case 72: return 7;
  default: return -1;
  }
}

int fishbot_event(snd_seq_event_t *ev, int serial){
  char chime;
  int stop = 0;

    // process event
  switch (ev->type) {
  case SND_SEQ_EVENT_CONTROLLER:
  case SND_SEQ_EVENT_PITCHBEND:
  case SND_SEQ_EVENT_NOTEOFF: 

    // Ignore 
    break;
  case SND_SEQ_EVENT_NOTEON:

    // play a note (if it is in range)
    fprintf(stderr, "[ NOTE ON   (%5d) on channel %2d ]      \r",
	    ev->data.note.note, ev->data.control.channel);
    chime = get_chime(ev->data.note.note);
     if (chime != -1){
      serialport_writebyte(serial, NOTE_ON);
      usleep(1000);
      serialport_writebyte(serial, chime);      
    } 
    break;
  }
  snd_seq_free_event(ev);

  if (stop) {
    
    // bring client to safe position
    serialport_writebyte(serial, PANIC);
  }

  return stop;
}

