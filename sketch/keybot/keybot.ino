#include <Servo.h> 
#include "message.h"
 


typedef struct {
  int   lo_ang;   // lower rest angle
  int   hi_ang;   // higher rest angle
  int   lo_param; // for reading out to server
  int   hi_param; // for reading out to server
  bool  pressed; 
  Servo servo;
} Finger;

typedef struct {
  char event;
  char param;
  bool valid;
} Message;


Finger  finger[8];       // array of fingers
int     last_finger;     // last finger moved for calibration

bool    new_msg = true;  // for serial FSM: next byte is a new message
Message msg;             // current message to be processed

bool          pending = false;  // incomplete message pending 
unsigned long pending_since;    // milliseconds since message pending

int redPin   = 2;
int greenPin = 12;
int bluePin  = 11;
 
int color = 0; 
 
void setup() 
{ 
  
  // set LED pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);  
  pinMode(bluePin, OUTPUT); 
  
  // initialize serial connection to host
  Serial.begin(9600);
  Serial.flush();
  
  // initialize fingers
  for (int i=0; i<8; ++i){
    if (i%2){
      
      // odd fingers turn to 0 degrees
      finger[i].lo_ang = 70;
      finger[i].hi_ang = 90;
      
      finger[i].lo_param = map(finger[i].lo_ang, 90, 45, 127, 0);
      finger[i].hi_param = map(finger[i].hi_ang, 115, 60, 127, 0);
    } else {      
      
      // even fingers turn to 180 degrees
      finger[i].lo_ang = 110;
      finger[i].hi_ang = 90;
        
      finger[i].lo_param = map(finger[i].lo_ang, 90, 135, 127, 0);
      finger[i].hi_param = map(finger[i].hi_ang, 75, 110, 127, 0);
    }
    finger[i].pressed = false;
    finger[i].servo.attach(3+i);
  }
  
  // no key pressed yet
  last_finger = -1;
  
  // invalidate current message and reset servos
  panic();
  
  setColor(color, 0, 0);
  color = (color + 1) % 256;
} 


// avoid freezing by invalidating incomplete messages after time out
void drop_message()
{
  if (pending){
    
    unsigned long duration = millis() - pending_since;
    
    if (duration > 100) {
      
      // no completion for 100 ms -> drop message
      pending = false;
      new_msg = true;
    }
  } else if (!new_msg){
   
     // incomplete message -> remember
     pending = true; 
     pending_since = millis();
  }
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
  new_msg = true;
  pending = false;
}


void set_lo(int param){
 if (last_finger == -1) return;
 
 int ang = 90;
  
 // param is between 0 (= all down) and 127 (= all up at 90 degrees)
 if (last_finger%2){
   
   // odd fingers turn to 0 degrees
   ang = map(msg.param, 127, 0, 90, 45);
 } else {
   
   // even fingers turn to 180 degrees
   ang = map(msg.param, 127, 0, 90, 135);
 }
 
 finger[last_finger].lo_param = param;
 finger[last_finger].lo_ang = ang;
 if (finger[last_finger].pressed){
   finger[last_finger].servo.write(ang);
 }  
}


void set_hi(int param){
 if (last_finger == -1) return;
 
 int ang = 90;
  
 // param is between 0 (= all down) and 127 (= all up at 90 degrees)
 if (last_finger%2){
   
   // odd fingers turn to 0 degrees
   ang = map(msg.param, 127, 0, 115, 60);
 } else {
   
   // even fingers turn to 180 degrees
   ang = map(msg.param, 127, 0, 75, 110);
 }
 
 finger[last_finger].hi_param = param;
 finger[last_finger].hi_ang = ang;
 if (!finger[last_finger].pressed){
   finger[last_finger].servo.write(ang);
 }  
}
 


void serialEvent() {

  // do not overwrite valid message
  if (!msg.valid) {
    
    // get next byte from serial line
    if (Serial.available()){
      char inByte = (char)Serial.read();
      if (new_msg){
        
        // byte is event type
        msg.event = inByte;
        if (inByte == PANIC || inByte == ALIVE || inByte == DUMP || inByte == WHORU){
           msg.valid = true; 
        } else {
           new_msg = false; 
        }
      } else {
       
        // byte is param
        msg.param = inByte;
        msg.valid = true;
        new_msg = true;
        
        // message is complete
        pending = false;
      } 
    }
  }  
}


void setColor(int red, int green, int blue)
{
  analogWrite(redPin,   red);
  analogWrite(greenPin, green);
  analogWrite(bluePin,  blue);
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
      // Serial.println("Note on");
    } else if (msg.event == NOTE_OFF){
      int k = msg.param;
      finger[k].pressed = false;
      finger[k].servo.write(finger[k].hi_ang);
      last_finger = k;
      msg.valid = false;
      // Serial.println("Note off");
    } else if (msg.event == SET_LO){
      set_lo(msg.param);
      msg.valid = false;
      // Serial.println("Set low rest");
    } else if (msg.event == SET_HI){
      set_hi(msg.param); 
      msg.valid = false;
      // Serial.println("Set high rest");
    } else if (msg.event == DUMP){
       for (int i=0; i<8; ++i){
          Serial.write(finger[i].lo_param);
          Serial.write(finger[i].hi_param);
       } 
       Serial.write(DUMP_EOF);     
       msg.valid = false;
    }  else if (msg.event == WHORU){
       Serial.write(KEYBOT); 
       Serial.write(IAM);
       msg.valid = false; 
    }
  }
 
  
  
  // invalidate incomplete messages after 100 ms
  drop_message();
}
