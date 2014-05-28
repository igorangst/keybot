/*****************************************************************
 * 
 *
 * The code in this file is inspired by Matthias Nagorni's example
 * seqdemo.c, which can be found on his website:
 * http://turing.suse.de/~mana/
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <arduino-serial-lib.h>
#include <message.h>


snd_seq_t *open_seq();
int midi_action(snd_seq_t *seq_handle);

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

int send_byte(int serial, char byte){
  return 0;
}

char get_key(unsigned int note){
  char key  = 0; 
  switch (note){
  case 36: key = 1; break;
  case 38: key = 2; break;
  case 40: key = 3; break;
  case 41: key = 4; break;
  case 43: key = 5; break;
  case 45: key = 6; break;
  case 47: key = 7; break;
  case 48: key = 8; break;
  }
  return key;
}

int midi_action(snd_seq_t *seq_handle) {

  snd_seq_event_t *ev;
  int stop = 0;

  do {
    snd_seq_event_input(seq_handle, &ev);
    switch (ev->type) {
    case SND_SEQ_EVENT_CONTROLLER:{ 
      fprintf(stderr, "Control event on Channel %2d: C%1d = %3d       \r",
	      ev->data.control.channel, 
	      ev->data.control.param, 
	      ev->data.control.value);
      unsigned int param = ev->data.control.param;
      int value = ev->data.control.value;
      
      if (param == 1){
	
	// lower rest position
	send_byte(1, (char)SET_LO);
	send_byte(1, (char)value);
	
      } else if (param == 2){
	
	// upper rest position
	send_byte(1, (char)SET_HI);
	send_byte(1, (char)value);
      }
      
      break;
    }
    case SND_SEQ_EVENT_PITCHBEND:{
      fprintf(stderr, "Pitchbender event on Channel %2d: %5d   \r", 
	      ev->data.control.channel, ev->data.control.value);
      stop = 1;
      break;
    }
    case SND_SEQ_EVENT_NOTEON:{
      fprintf(stderr, "Note On event on Channel %2d: %5d       \r",
	      ev->data.control.channel, ev->data.note.note);
      unsigned int note = ev->data.note.note;
      char key = get_key(note);
      if (key){
	send_byte(1, (char)NOTE_ON);
	send_byte(1, key);
      }
      
      //	serialport_writebyte(serial, '1');
      break;
    }        
    case SND_SEQ_EVENT_NOTEOFF: {
      fprintf(stderr, "Note Off event on Channel %2d: %5d      \r",         
	      ev->data.control.channel, ev->data.note.note);           
      unsigned int note = ev->data.note.note;
      char key = get_key(note);
      if (key){
	send_byte(1, (char)NOTE_OFF);
	send_byte(1, key);
      }
      //	serialport_writebyte(serial, '0');
      break;
    }}
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);

  if (stop){
    send_byte(1, (char)PANIC);
  }
  return stop;
}


void usage(){
  printf ("Usage: keybot [OPTION]\n");
  printf (" -d dev\tdevice for serial connection to arduino (/dev/ttyACM0)\n");
  printf (" -b int\tbaud rate for serial connection (9600)\n");
  printf (" -h    \tprint help message and exit\n");  
}


int main(int argc, char *argv[]) {

  // device for serial connection
  char dev[256] = "/dev/ttyACM0";

  // baud rate for serial connection
  int brate = 9600;

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
  int npfd;
  struct pollfd *pfd;
    
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

  // setup serial 
  /* int fd = -1; */
  /* fd = serialport_init(dev, brate); */
  /* if (fd == -1){ */
  /*   printf("ERROR: could not open device '%s'\n", dev); */
  /*   exit(1); */
  /* } */
  /* serialport_flush(fd); */

  while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
      if (midi_action(seq_handle)) break;
    }  
  }

  printf("exit\n");

  //  serialport_close(fd);
}
