#include "midi.h"
#include "client.h"

snd_seq_t     *seq_handle = 0;
struct pollfd *pfd = 0;
int            npfd = 0;


/******************************************************************
 * Function: snd_seq_t *open_seq()
 * Open connection to ALSA sequncer, used for all MIDI handling.
 * @return: handler for ALSA sequencer connection
 ******************************************************************/
snd_seq_t *open_seq() {
  snd_seq_t *seq_handle;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "Arduino Keybot");
  return(seq_handle);
}

int open_port(const char *name){
  int portid;

  if ((portid = snd_seq_create_simple_port(seq_handle, name,
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    return -1;
  }
  return portid;
}

void init_alsa(){
  seq_handle = open_seq(); 
  snd_seq_drop_input(seq_handle);
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  printf("Connected to ALSA sequencer.\n");
}


/******************************************************************
 * Function: int drop_event(snd_seq_event_t *ev){
 * Check if an event is on a channel we are listening on
 * @param ev: event to check
 * @return  : returns 1 if the event should be dropped, 0 otherwise
 ******************************************************************/
int drop_event(snd_seq_event_t *ev){

  // all channels on?
  if (mubot_options.midi_chan == 0){
    return 0;
  }
  
  int chan = ev->data.control.channel;
  return (chan + 1 != mubot_options.midi_chan);
}


/******************************************************************
 * Function: int midi_action(snd_seq_t *seq_handle, int serial)
 * Main function to handle MIDI events. Sends control messages to
 * the arduino client
 * @param seq_handle: handler for ALSA connection 
 * @param serial    : handler for serial connection to arduino
 * @return          : returns 1 if an error or stop occurred
 ******************************************************************/
int midi_action(snd_seq_t *seq_handle) {

  snd_seq_event_t *ev;
  int stop = 0;

  // loop while new MIDI events are in the queue
  do {
    snd_seq_event_input(seq_handle, &ev);

    // filter events on other channels
    if (drop_event(ev)) {
      snd_seq_free_event(ev);
      printf("drop\n");
      continue;
    }

    // lookup target client and redirect event
    char port = ev->dest.port;
    int i;
    for (i=0; i<nclients; ++i){
      if (clients[i].type != NOBODY && clients[i].port == port){
	int serial = clients[i].serial;
	ClientType t = clients[i].type;
	if (t == KEYBOT){
	  stop =  keybot_event(ev, serial);
	  break;
	}
      }
    }
  } while (!stop && snd_seq_event_input_pending(seq_handle, 0) > 0);
  
  return stop; 
}
