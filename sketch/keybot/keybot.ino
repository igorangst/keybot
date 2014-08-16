#include <Servo.h> 
#include "message.h"
 

typedef struct {
  int   lo_ang;
  int   hi_ang;
  bool  pressed;
  Servo servo;
} Finger;

typedef struct {
  char event;
  char param;
  bool valid;
} Message;


Finger finger[8];    // array of fingers
int    last_finger;  // last finger moved for calibration
Message msg;         // current message to be processed
 
 
void setup() 
{ 
  // initialize serial connection to host
  Serial.begin(9600);
  
  // initialize fingers
  for (int i=0; i<8; ++i){
    if (i%2){
      finger[i].lo_ang = 45;
      finger[i].hi_ang = 90;
    } else {      
      finger[i].lo_ang = 90;
      finger[i].hi_ang = 45;
    }
    finger[i].pressed = false;
    finger[i].servo.attach(i);
  }
  
  // invalidate current message
  msg.valid = false;
} 
 
void panic()
{
  
  // all notes off
  for (int i=0; i<8; ++i){
    finger[i].pressed = false;
    finger[i].servo.write(finger[i].hi_ang);
  }
  
  // invalidate message
  msg.valid = false;
}

// for serial FSM: next byte is a new message
bool new_msg = true;

void serialEvent() {

  // do not overwrite valid message
  if (!msg.valid) {
    
    // get next byte from serial line
    if (Serial.available()){
      char inByte = (char)Serial.read();
      if (new_msg){
        
        // byte is event type
        msg.event = inByte;
        if (inByte == PANIC || inByte == ALIVE){
           msg.valid = true; 
        } else {
           new_msg = false; 
        }
      } else {
       
        // byte is param
        msg.param = inByte;
        msg.valid = true;
        new_msg = true;
      } 
    }
  }  
}
 
void loop() 
{ 
  if (msg.valid){   
    if (msg.event == PANIC){
       panic(); 
       msg.valid = false;
    } else if (msg.event == ALIVE){
      
      // FIXME: reset timer here for heart beat function
      msg.valid = false;      
    } else if (msg.event == NOTE_ON){
      int k = msg.param;
      finger[k].pressed = true;
      finger[k].servo.write(finger[k].lo_ang);
      last_finger = k;
      msg.valid = false;
    } else if (msg.event == NOTE_OFF){
      int k = msg.param;
      finger[k].pressed = false;
      finger[k].servo.write(finger[k].hi_ang);
      last_finger = k;
      msg.valid = false;
    } else if (msg.event == SET_LO){
      int ang = msg.param; 
      finger[last_finger].lo_ang = ang;
      if (finger[last_finger].pressed){
        finger[last_finger].servo.write(ang);
      }
      msg.valid = false;
    } else if (msg.event == SET_HI){
      int ang = msg.param; 
      finger[last_finger].hi_ang = ang;
      if (!finger[last_finger].pressed){
        finger[last_finger].servo.write(ang);
      }
      msg.valid = false;
    }
  }
} 
