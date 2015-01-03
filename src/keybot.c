#include "keybot.h"

/******************************************************************
 * Function: char get_finger(int note)
 * Simple conversion from MIDI notes to finger indices. Per default,
 * the fingers cover the white keys of one octave from C to C.
 * @return: finger index between -1 (out of range) and 7
 ******************************************************************/
char get_finger(int note){
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

/******************************************************************
 * Function: void restore_params(int serial)
 * Reads parameters from config file and sets the keybot client
 * accordingly. Parameters are lo and hi rest positions.
 * @param serial: handler for serial connection to arduino
 ******************************************************************/
void restore_params(int serial){
  FILE *myFile;
  
  myFile = fopen(mubot_options.config_file, "r");

  //read file into array
  int numberArray[16];
  int i;

  if (myFile == NULL) {
    printf("Error reading file '%s'\n", mubot_options.config_file);
    exit (1);
  }
  for (i = 0; i < 16; i++) {
    fscanf(myFile, "%d,", &numberArray[i] );
  }
  printf ("\rRestoring parameters from config file '%s'\n", mubot_options.config_file);
  for (i = 0; i < 8; i++) {
    char lo = (char)numberArray[2*i];
    char hi = (char)numberArray[2*i+1];
    printf ("Set finger %i to lo=%i, hi=%i\n", i+1, lo, hi);

    serialport_writebyte(serial, NOTE_OFF);
    usleep(1000);
    serialport_writebyte(serial, (char)i);
    usleep(1000);
    serialport_writebyte(serial, SET_LO);
    usleep(1000);
    serialport_writebyte(serial, lo);
    usleep(1000);
    serialport_writebyte(serial, SET_HI);
    usleep(1000);
    serialport_writebyte(serial, hi);
  }
  printf ("Done.\n");
}


/******************************************************************
 * Function: void dump_params(int serial)
 * Ask the arduino client to transfer parameters over serial line.
 * Parameters are lo and hi rest positions. The transfer is ended
 * by a DUMP_EOF symbol.
 * @param serial: handler for serial connection to arduino
 ******************************************************************/
void dump_params(int serial){
  char params[256];  

  // FIXME
  char config_file[32] = "keybot.conf";

  serialport_writebyte(serial, DUMP);
  usleep(1000);
  int n = serialport_read_until(serial, params, DUMP_EOF, 256, 1000);
  if (n == -1) {
    printf("\rCould not get parameters from client. Read transaction timed out.\n");
    return;
  }

  FILE *myFile;
  myFile = fopen(config_file, "w");
  if (myFile == NULL) {
    printf("Error writing file '%s'\n", config_file);
    exit (1);
  }
  printf ("\rSaving parameters to config file '%s'\n", config_file);

  unsigned i=0;
  for (i=0; i<8; ++i){
    int lo = params[2*i];
    int hi = params[2*i+1];
    printf ("Set finger %i to lo=%i, hi=%i\n", i+1, lo, hi);
    fprintf(myFile, "%i %i ", lo, hi); 
  }
  printf ("Done.\n");

  fclose(myFile);
  // flush serial
  // serialport_read_until(serial, params, DUMP_EOF, 256, 1000);  
}


int keybot_event(snd_seq_event_t *ev, int serial){
  char finger;
  int stop = 0;

  // last event was note on
  static int note_on = 0; 

  // process event
  switch (ev->type) {
  case SND_SEQ_EVENT_CONTROLLER:
    
    // controller events are used to set the rest positions 
    fprintf(stderr, "[ CONTROL   (%5d) on channel %2d ]      \r",
	    ev->data.control.value, ev->data.control.channel);
    char v = ev->data.control.value;
    
    if (!mubot_options.lock_controls){
      serialport_writebyte(serial, note_on ? SET_LO : SET_HI);
      usleep(1000);
      serialport_writebyte(serial, v);
    }
    break;
  case SND_SEQ_EVENT_PITCHBEND:

    // Ignore for now
    break;
  case SND_SEQ_EVENT_NOTEON:

    // play a note (if it is in range)
    fprintf(stderr, "[ NOTE ON   (%5d) on channel %2d ]      \r",
	    ev->data.note.note, ev->data.control.channel);
    finger = get_finger(ev->data.note.note);
     if (finger != -1){
      serialport_writebyte(serial, NOTE_ON);
      usleep(1000);
      serialport_writebyte(serial, finger);
      note_on = 1;	
    } 
    break;        
  case SND_SEQ_EVENT_NOTEOFF: 

    // release note (if it is in range)
    fprintf(stderr, "[ NOTE OFF  (%5d) on channel %2d ]      \r",
	    ev->data.note.note, ev->data.control.channel);
    finger = get_finger(ev->data.note.note);
    if (finger != -1){
      serialport_writebyte(serial, NOTE_OFF);
      usleep(1000);
      serialport_writebyte(serial, finger);
      note_on = 0;
    } 
    break;
  }
  snd_seq_free_event(ev);

  if (stop) {
    
    // bring client to safe position
    serialport_writebyte(serial, PANIC);
    dump_params(serial);
  }

  return stop;
}

