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
#include <arduino-serial-lib.h>
#include <options.h>
#include <message.h>
#include <client.h>
#include <midi.h>
#include <keybot.h>

// ======================================================={ global options
char dev[256];          // serial device, default is "/dev/ttyACM0"
int  brate = 9600;      // baud rate for serial 
// ========================================================}


// will be set when ctrl-c is intercepted
int sig_exit = 0;

// signal handler for ctrl-c
void intHandler(int dummy) {
  sig_exit = 1;
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

  snd_seq_event_t *ev;
  int stop = 0;

  // loop while new MIDI events are in the queue
  do {
    snd_seq_event_input(seq_handle, &ev);

    // filter events on other channels
    if (drop_event(ev)) {
      snd_seq_free_event(ev);
      continue;
    }

    // lookup target client
    char client = ev->dest.client;
    char port = ev->dest.port;

    printf("Dest: (%i,%i)\n", client, port);
    stop =  keybot_event(ev, serial);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0 
	   && !stop 
	   && !sig_exit);
  
  return stop; 
}


/******************************************************************
 * Function: usage()
 * Print usage message on stdout.
 ******************************************************************/
void usage(){
  printf ("Usage: keybot [OPTION]\n");
  printf (" -d dev  \tdevice for serial connection to arduino (default /dev/ttyACM*)\n");
  printf (" -b int  \tbaud rate for serial connection (default 57600)\n");
  printf (" -c chan \tMIDI channel to listen on (default all)\n");
  printf (" -r      \trestore params from config file on startup (default off)\n");
  printf (" -s      \tstore params to config file on exit (default off)\n");
  printf (" -f file \tuse config file for params (default keybot.conf)\n");
  printf (" -l      \tlock params, ignore MIDI controller messages (default unlocked)\n");
  printf (" -h      \tprint help message and exit\n");  
}


/******************************************************************
 * Function: parse_args(int argc, char* argv[])
 * Parse command line args, exit on error
 ******************************************************************/
void parse_args(int argc, char* argv[]) {
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
      int c = atoi(argv[i]);
      if (c < 0 || c > 16){
	printf("ERROR: illegal MIDI channel number: %i\n", c);
	exit(1);
      }
      mubot_options.midi_chan = c;
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-r")){
      mubot_options.restore = 1;
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-s")){
      mubot_options.store = 1;
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-f")){
      ++i;
      if (i>=argc){
	printf ("ERROR: missing argument for -f option\n");
	usage();
	exit(1);
      }
      strcpy(mubot_options.config_file, argv[i]);
      ++i;
      continue;
    } else if (!strcmp(argv[i], "-l")){
      mubot_options.lock_controls = 1;
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
}


/******************************************************************
 * Function: main()
 * What do you think it does?
 ******************************************************************/
int main(int argc, char *argv[]) {

  // set default values and parse command line arguments
  strcpy(dev, "/dev/ttyACM0");
  default_options();
  parse_args(argc, argv);

  // setup ALSA
  snd_seq_t *seq_handle;
  seq_handle = open_seq(); 
  snd_seq_drop_input(seq_handle);
  int npfd;
  struct pollfd *pfd;
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  printf("Connected to ALSA sequencer.\n");

  // connect to client
  int serial = open_serial(dev, brate);
  if (serial == -1){
    printf("ERROR: Could not connect to client via '%s'\n", dev);
    exit(-1);
  }
  ClientType t = probe(serial);
  if (t != KEYBOT){
    printf("ERROR: Incompatible client on device '%s'\n", dev);
    exit(-1);
  }

  if (mubot_options.restore){
    restore_params(serial);
  }

  // set signal handler for ctrl-c
  signal(SIGINT, intHandler);

  // main loop processing MIDI events
  printf("Let us hear some muzak!\n");
  while (!sig_exit) {
    if (poll(pfd, npfd, 100000) > 0) {
      if (midi_action(seq_handle, serial)) break;
    }  
  }

  if (mubot_options.store){
    dump_params(serial);
  }

  // clean up
  close_serial(serial);
  snd_seq_close(seq_handle);
  printf("\rGood bye!                                       \n");
  exit(0);
}
