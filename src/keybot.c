/*****************************************************************
 * FILE  : keybot.c
 * AUTHOR: Igor Angst (http://github.com/igorangst)
 * 
 * DESCR : Server for Arduino KeyBot. The ALSA communication code in
 *         this file is inspired by Matthias Nagorni's example
 *         seqdemo.c, which can be found on his website:
 *         http://turing.suse.de/~mana/
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <arduino-serial-lib.h>
#include <message.h>


// global settings
int  midi_chan = 0;     // 0 == all channels
char config_file[256];

// forward declarations
snd_seq_t *open_seq();
int midi_action(snd_seq_t *seq_handle, int serial);

// will be set when ctrl-c is intercepted
int sig_exit = 0;

// signal handler for ctrl-c
void intHandler(int dummy) {
  sig_exit = 1;
}

/******************************************************************
 * Function: snd_seq_t *open_seq()
 * Open connection to ALSA sequncer, used for all MIDI handling.
 * @return: handler for ALSA sequencer connection
 ******************************************************************/
snd_seq_t *open_seq() {
  snd_seq_t *seq_handle;
  int portid;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "Arduino Keybot");
  if ((portid = snd_seq_create_simple_port(seq_handle, "Arduino Keybot IN1",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}


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
  myFile = fopen(config_file, "r");

  //read file into array
  int numberArray[16];
  int i;

  if (myFile == NULL) {
    printf("Error Reading File\n");
    exit (0);
  }
  for (i = 0; i < 16; i++) {
    fscanf(myFile, "%d,", &numberArray[i] );
  }
  printf ("======================================\n");
  printf ("RESTORE PARAMS FROM CONFIGURATION FILE\n");
  printf ("======================================\n");
  for (i = 0; i < 8; i++) {
    char lo = (char)numberArray[2*i];
    char hi = (char)numberArray[2*i+1];
    printf ("LO[%i] = %i \t HI[%i] = %i\n", i, lo, i, hi);

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
  printf ("======================================\n");
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

  serialport_writebyte(serial, DUMP);
  usleep(1000);
  serialport_read_until(serial, params, DUMP_EOF, 256, 5000);
  
  printf ("======================================\n");
  printf ("STORE PARAMETERS FROM ARDUINO CLIENT   \n");
  printf ("======================================\n");

  FILE *myFile;
  myFile = fopen(config_file, "w");

  unsigned i=0;
  for (i=0; i<8; ++i){
    int lo = params[2*i];
    int hi = params[2*i+1];
    printf("LO[%i] = %i / HI[%i] = %i\n", i, lo, i, hi);
    fprintf(myFile, "%i %i ", lo, hi); 
  }
  printf ("======================================\n");

  fclose(myFile);
  // flush serial
  // serialport_read_until(serial, params, DUMP_EOF, 256, 1000);  
}


/******************************************************************
 * Function: int drop_event(snd_seq_event_t *ev){
 * Check if an event is on a channel we are listening on
 * @param ev: event to check
 * @return  : returns 1 if the event should be dropped, 0 otherwise
 ******************************************************************/
int drop_event(snd_seq_event_t *ev){

  // all channels on?
  if (midi_chan == 0){
    return 0;
  }
  
  int chan = ev->data.control.channel;
  return (chan + 1 != midi_chan);
}


/******************************************************************
 * Function: int midi_action(snd_seq_t *seq_handle, int serial)
 * Main function to handle MIDI events. Sends control messages to
 * the arduino client
 * @param seq_handle: handler for ALSA connection 
 * @param serial    : handler for serial connection to arduino
 * @return          : returns 1 if an error or stop occurred
 ******************************************************************/
int midi_action(snd_seq_t *seq_handle, int serial) {

  // last event was note on
  static int note_on = 0; 

  snd_seq_event_t *ev;
  int stop = 0;
  char finger;

  // loop while new MIDI events are in the queue
  do {
    snd_seq_event_input(seq_handle, &ev);

    // filter events on other channels
    if (drop_event(ev)) {
      snd_seq_free_event(ev);
      continue;
    }

    // process event
    switch (ev->type) {

      // controller events are used to set the rest positions
      case SND_SEQ_EVENT_CONTROLLER: 
        fprintf(stderr, "Control event on Channel %2d: %5d       \r",
                ev->data.control.channel, ev->data.control.value);
	char v = ev->data.control.value;
	serialport_writebyte(serial, note_on ? SET_LO : SET_HI);
	usleep(1000);
	serialport_writebyte(serial, v);
        break;
      case SND_SEQ_EVENT_PITCHBEND:

	// for now, pitch bend will stop the server process
        fprintf(stderr, "Pitchbender event on Channel %2d: %5d   \r", 
                ev->data.control.channel, ev->data.control.value);
	stop = 1;
        break;
      case SND_SEQ_EVENT_NOTEON:

	// play a note (if it is in range)
        fprintf(stderr, "Note On event on Channel %2d: %5d       \r",
                ev->data.control.channel, ev->data.note.note);
	finger = get_finger(ev->data.note.note);
	fprintf(stderr, "Finger %i     \r", finger);
	if (finger != -1){
	  serialport_writebyte(serial, NOTE_ON);
	  usleep(1000);
	  serialport_writebyte(serial, finger);
	  note_on = 1;	
	} 
        break;        
      case SND_SEQ_EVENT_NOTEOFF: 

	// release note (if it is in range)
        fprintf(stderr, "Note Off event on Channel %2d: %5d      \r",         
                ev->data.control.channel, ev->data.note.note);
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
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);

  if (stop) {
    
    // bring client to safe position
    serialport_writebyte(serial, PANIC);
    dump_params(serial);
  }
  return stop;
}


/******************************************************************
 * Function: usage()
 * Print usage message on stdout.
 ******************************************************************/
void usage(){
  printf ("Usage: keybot [OPTION]\n");
  printf (" -d dev\tdevice for serial connection to arduino (/dev/ttyACM0)\n");
  printf (" -b int\tbaud rate for serial connection (9600)\n");
  printf (" -c chan\tMIDI channel to listen on (default all)\n");
  printf (" -r    \trestore params from config file on startup\n");
  printf (" -s    \tstore params on exit\n");
  printf (" -f file\tuse config file for params\n");
  printf (" -h    \tprint help message and exit\n");  
}


/******************************************************************
 * Function: main()
 * What do you think it does?
 ******************************************************************/
int main(int argc, char *argv[]) {

  // device for serial connection
  char dev[256] = "/dev/ttyACM0";

  // baud rate for serial connection
  int brate = 9600;

  // restore/stpre parameter on startup/exit
  int restore = 0;
  int store = 0;
  strcpy(config_file, "keybot.conf");

  // process command line arguments
  int i;
  for (i=1; i<argc; ){
    if (!strcmp(argv[i], "-d")){
      ++i;
      if (i>=argc){
	printf ("ERROR: missing argument for -d option\n");
	usage();
	exit(1);
      }
      strcpy(dev, argv[i]);
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-b")){
      ++i;
      if (i>=argc){
	printf ("ERROR: missing argument for -b option\n");
	usage();
	exit(1);
      }
      brate = atoi(argv[i]);
      if (brate <= 0){
	printf("ERROR: expecting positive baud rate\n");
	exit(1);
      }
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-c")){
      ++i;
      if (i>=argc){
	printf ("ERROR: missing argument for -c option\n");
	usage();
	exit(1);
      }
      midi_chan = atoi(argv[i]);
      if (midi_chan < 0 || midi_chan > 16){
	printf("ERROR: illegal MIDI channel number: %i\n", midi_chan);
	exit(1);
      }
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-r")){
      restore = 1;
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-s")){
      store = 1;
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-f")){
      ++i;
      if (i>=argc){
	printf ("ERROR: missing argument for -f option\n");
	usage();
	exit(1);
      }
      strcpy(config_file, argv[i]);
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-h")){
      usage();
      exit(0);
    }
    printf ("ERROR: unknown option or command line argument '%s'\n", argv[i]);
    usage();
    exit(1);
  }

  // setup ALSA
  snd_seq_t *seq_handle;
  seq_handle = open_seq(); // FIXME: flush MIDI events 
  int npfd;
  struct pollfd *pfd;
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

  // setup serial 
  int fd = -1;
  fd = serialport_init(dev, brate);
  if (fd == -1){
    printf("ERROR: could not open device '%s'\n", dev);
    exit(1);
  }
  usleep(1000);
  serialport_flush(fd);

  if (restore){
    restore_params(fd);
  }

  // set signal handler for ctrl-c
  signal(SIGINT, intHandler);

  // main loop processing MIDI events
  while (!sig_exit) {
    if (poll(pfd, npfd, 100000) > 0) {
      if (midi_action(seq_handle, fd)) break;
    }  
  }

  if (store){
    dump_params(fd);
  }

  // clean up
  serialport_close(fd);
}
