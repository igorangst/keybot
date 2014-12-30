#ifndef MESSAGE_H
#define MESSAGE_H

enum KeybotMessage { 
  NOTE_ON  = 0x01 ,  // note on event, followed by key (0-127)
  NOTE_OFF = 0x02 ,  // note off event, followed by key (0-127)
  PANIC    = 0xff ,  // all notes off
  ALIVE    = 0xaa ,  // heart beat
  SET_LO   = 0x11 ,  // set lower rest position (0 = 180 deg, 127 = 0 deg)
  SET_HI   = 0x12 ,  // set upper rest position (0 = 180 deg, 127 = 0 deg)
  DUMP     = 0x55 ,  // dump all parameters in (lo,hi) pairs from 0 to 7
  DUMP_EOF = 0x80 ,  // byte sent to mark end of dump
  WHORU    = 0x26 ,  // command to identify arduino client side 
  IAM      = 0x27    // answer from client side, followed by client ID (see below)
};

typedef enum {
  KEYBOT   = 0x01 , 
  DRUMBOT  = 0x02 ,
  NOBODY   = 0xff
} ClientType;

#endif
