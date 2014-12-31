#include "midi.h"


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
