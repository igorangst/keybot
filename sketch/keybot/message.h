#ifndef MESSAGE_H
#define MESSAGE_H

enum KeybotMessage { 
  NOTE_ON  = 0x01 ,  // note on event, followed by key (0-7)
  NOTE_OFF = 0x02 ,  // note off event, followed by key (0-7)
  PANIC    = 0xff ,  // all notes off
  ALIVE    = 0xaa ,  // heart beat
  SET_LO   = 0x11 ,  // set lower rest position (0 = 180 deg, 127 = 0 deg)
  SET_HI   = 0x12 ,  // set upper rest position (0 = 180 deg, 127 = 0 deg)
};

#endif
