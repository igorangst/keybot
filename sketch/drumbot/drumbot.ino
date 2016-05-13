/*
 Control for DrumBot. The motor control code is inspired by
 the code for a robotic drum created by Randy Sarafan, see
 http://www.instructables.com/id/Arduino-Controlled-Robotic-Drum/ 
 
 The server code for the ALSA-based Linux application can be found on
 https://github.com/igorangst/keybot
 
 */

#include "LPD8806.h"
#include "SPI.h" // Comment out this line if using Trinket or Gemma
#include "Timer.h"


// Number of RGB LEDs in strand:
int nLEDs = 25;

// Chose 2 pins for output; can be any valid output pins:
int dataPin  = 4;
int clockPin = 5;

// First parameter is the number of LEDs in the strand.  The LED strips
// are 32 LEDs per meter but you can extend or cut the strip.  Next two
// parameters are SPI data and clock pins:
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);


// messages
#define WHORU    0x26
#define IAM      0x27
#define SNAREBOT 0x03
#define NOTE_ON  0x01

// motor directions
#define DIR_UP   LOW
#define DIR_DOWN HIGH

// motor pins
#define DIR_A 12
#define BRK_A 9
#define MOV_A 3

#define DIR_B 13
#define BRK_B 8
#define MOV_B 11

// message over serial line
int event = 0;    // message completed
int pending = 0;  // incomplete message
int message = 0;  // type of event
int velocity = 0; // velocity of next beat

// strike length in ms
const int strikelength = 45;

// internal state of drum sticks
int activeA = 0;
int activeB = 0;

Timer t;

unsigned char color[25][3];  // buffer for strip pixels in RBG format
int colorIndex = 0;          // index (see Wheel() function) of last chosen color
unsigned char lastColor[3];  // RGB of last chosen color
unsigned char fader[3]; 
int fadeMode = 0;
int nFadeModes = 5;
int fadeCount = 0;

void setup() {
  
  //establish motor direction toggle pins
  pinMode(DIR_A, OUTPUT); 
  pinMode(DIR_B, OUTPUT); 
  
  //establish motor brake pins
  pinMode(BRK_A, OUTPUT); 
  pinMode(BRK_B, OUTPUT); 

  digitalWrite(BRK_A, LOW); // ENABLE CH A
  digitalWrite(BRK_B, LOW); // ENABLE CH B  
  
  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();

  t.every(20, updateStrip);
  t.every(20, fadeColors);
  
  Serial.begin(9600);
}

// compute the actual strike length depending on the velocity
int actualStrikeLength() {
  return map(velocity, 0, 127, 2*strikelength, strikelength);
}

// compute PWM parameter for motor depending on the velocity
int strikeForce() {
  return 128 + velocity;  
}

// The movement of the sticks follows a sequence of four phases, 
// which are activated using the timer in order to avoid delays.
// ------------------------------------------------------------
// beat    : stick moves down
// rest    : stick rests for 10 ms while the motor is braking
// release : stick moves up
// finish  : the motor is shut down again

void beatA() {
   //start down
   digitalWrite(BRK_A, LOW); 
   digitalWrite(DIR_A, DIR_DOWN);   
   analogWrite(MOV_A, strikeForce() );  
       
   //strike time
   t.after(actualStrikeLength(), restA);
}

void restA() {
   //stop
   digitalWrite(BRK_A, HIGH); 
   t.after(10, releaseA);
}
 
void releaseA() {      
   //go back
   digitalWrite(BRK_A, LOW); 
   digitalWrite(DIR_A, DIR_UP); 
   analogWrite(MOV_A, 255);   
       
   //recoil time
   t.after(strikelength, finishA);
}

void finishA() {
   //stop
   digitalWrite(BRK_A, HIGH);
   activeA = 0; 
}

// Same for second drum stick

void beatB() {
   //start down
   digitalWrite(BRK_B, LOW); 
   digitalWrite(DIR_B, DIR_DOWN);   
   analogWrite(MOV_B, strikeForce() );  
       
   //strike time
   t.after(actualStrikeLength(), restB);
}

void restB() {
   //stop
   digitalWrite(BRK_B, HIGH); 
   t.after(10, releaseB);
}
 
void releaseB() {      
   //go back
   digitalWrite(BRK_B, LOW); 
   digitalWrite(DIR_B, DIR_UP); 
   analogWrite(MOV_B, 255);   
       
   //recoil time
   t.after(strikelength, finishB);
}

void finishB() {
   //stop
   digitalWrite(BRK_B, HIGH);
   activeB = 0; 
}

#define RGB_G(c) ((c >> 16) & 0x7f) 
#define RGB_R(c) ((c >>  8) & 0x7f) 
#define RGB_B(c) ((c)       & 0x7f)

void lightOn(){
  fadeMode = (fadeMode + 1) % nFadeModes;
  colorIndex = (colorIndex + 25) % 384;
  uint32_t c = Wheel(colorIndex);
  unsigned char r = RGB_R(c); 
  unsigned char g = RGB_G(c);
  unsigned char b = RGB_B(c);

  if (fadeMode < 3) {
    int i;
    for (i=0; i<nLEDs; ++i){
      setColor(i, r, g, b);
    }
    lastColor[0] = r;
    lastColor[1] = g;
    lastColor[2] = b;
  
    int steps = 32;
    for (i=0; i<3; ++i){
      fader[i] = (lastColor[i] / steps) + 1;
    }
  } else {
    
    // spike wheel
    int spike = nLEDs / 4;
    int i;
    for (i=0; i<nLEDs; ++i){
      int j = i / spike;
      if (j%2 == 0){
        color[i][0] = (r + j*16) % 128;
        color[i][1] = (g + j*16) % 128;
        color[i][2] = (b + j*16) % 128;
      } else {
        setColor(i, 0, 0, 0);
      }  
    }
  }
  fadeCount = 0;
}

void setColor(int i, unsigned char r, unsigned char g, unsigned char b){
  color[i][0] = r;
  color[i][1] = g;
  color[i][2] = b;  
}

void wheelRight(){
  unsigned char t[3];
  t[0] = color[0][0];
  t[1] = color[0][1];
  t[2] = color[0][2];
  int i, c;
  for (i = 1; i < nLEDs; ++i){
    for (c = 0; c < 3; ++c){
      color[i-1][c] = color[i][c]; 
    }   
  }
  setColor(nLEDs-1, t[0], t[1], t[2]);
}

void wheelLeft(){
  unsigned char t[3];
  t[0] = color[nLEDs-1][0];
  t[1] = color[nLEDs-1][1];
  t[2] = color[nLEDs-1][2];
  int i, c;
  for (i = nLEDs-1; i > 0; --i){
    for (c = 0; c < 3; ++c){
      color[i][c] = color[i-1][c]; 
    }   
  }
  setColor(0, t[0], t[1], t[2]);
}

void fadeColors() {
  fadeCount++;
  int i;
  
  if (fadeMode == 0){
    for (i = 0; i < nLEDs; ++i){
      color[i][0] = max(0, color[i][0] - fader[0]); 
      color[i][1] = max(0, color[i][1] - fader[1]);
      color[i][2] = max(0, color[i][2] - fader[2]);
    }
  } else if (fadeMode == 1 || fadeMode == 3){
    wheelRight();
    if (fadeCount > nLEDs){
      setColor(nLEDs-1, 0, 0, 0); 
    }
  } else if (fadeMode == 2 || fadeMode == 4){
    wheelLeft();
    if (fadeCount > nLEDs) {
      setColor(0, 0, 0, 0);
    }  
  }
}


void updateStrip() {
  int i;

  for (i = 0; i < nLEDs; ++i){
    strip.setPixelColor(i, strip.Color(color[i][0], color[i][1], color[i][2]));
  }  
  strip.show();
}


void loop() { 
 static int chan = 0;
 t.update();
  
 if(event == 1){
   event = 0;
   
   if (message == NOTE_ON) {
     if (chan == 0 && !activeA ||
         chan == 1 && activeB && !activeA){
       activeA = 1;
       chan = 1;
       beatA();  
     } else if (!activeB) {
       activeB = 1;
       chan = 0;
       beatB();
     } else {
       // FIXME: lost beat 
     } 
     lightOn();
   }
   
   if (message == WHORU) {
      Serial.write(SNAREBOT);
      Serial.write(IAM);
   }
 }
}


void serialEvent(){
  if (!event && Serial.available()){
    char inByte = Serial.read();
    if (pending) {
      event = 1;
      pending = 0;
      velocity = inByte;  
    } else {
      message = inByte;
      if (message == NOTE_ON){
        pending = 1;
      } else {
        event = 1;
      }      
    }
  }
}


//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}


