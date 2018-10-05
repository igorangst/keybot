
#include "Timer.h"
#include "message.h"

typedef enum State {
   PLAY,
   STOP,
   INTRO,
   SLAVE
};

typedef enum Pitch {
  PAUSE   = 0,
  NOTE_c  = 2, // enum values correspond to output pins 
  NOTE_d  = 3,
  NOTE_e  = 4, 
  NOTE_f  = 5, 
  NOTE_g  = 6, 
  NOTE_a  = 7, 
  NOTE_h  = 8, 
  NOTE_C  = 9,

  SET_BPM,      // change tempo  
  END_OF_SONG,  // spacer between songs
  DA_CAPO,      // start over from position 0
  GOTO,         // jump to specific position
  REPEAT,       // start of loop, followed by number of repetitions
  END_REPEAT    // end of loop
};

typedef struct Note_t {
  Pitch pitch;
  int   dur; 
} Note;

typedef struct Song_t {
  float bpm;
  Note  notes[64];
} Song;


const int button = 10; // pin of "next song" button
const int led    = 13; // status led

// shortcuts
#define c8 {NOTE_c, 8}
#define d8 {NOTE_d, 8}
#define e8 {NOTE_e, 8}
#define f8 {NOTE_f, 8}
#define g8 {NOTE_g, 8}
#define a8 {NOTE_a, 8}
#define h8 {NOTE_h, 8}
#define C8 {NOTE_C, 8}
#define _8 {PAUSE,  8}

#define c4 {NOTE_c, 4}
#define d4 {NOTE_d, 4}
#define e4 {NOTE_e, 4}
#define f4 {NOTE_f, 4}
#define g4 {NOTE_g, 4}
#define a4 {NOTE_a, 4}
#define h4 {NOTE_h, 4}
#define C4 {NOTE_C, 4}
#define _4 {PAUSE,  4}

#define c2 {NOTE_c, 2}
#define d2 {NOTE_d, 2}
#define e2 {NOTE_e, 2}
#define f2 {NOTE_f, 2}
#define g2 {NOTE_g, 2}
#define a2 {NOTE_a, 2}
#define h2 {NOTE_h, 2}
#define C2 {NOTE_C, 2}
#define _2 {PAUSE,  2}

#define c1 {NOTE_c, 1}
#define d1 {NOTE_d, 1}
#define e1 {NOTE_e, 1}
#define f1 {NOTE_f, 1}
#define g1 {NOTE_g, 1}
#define a1 {NOTE_a, 1}
#define h1 {NOTE_h, 1}
#define C1 {NOTE_C, 1}
#define _1 {PAUSE,  1}

#define c_ {NOTE_c, 0}
#define d_ {NOTE_d, 0}
#define e_ {NOTE_e, 0}
#define f_ {NOTE_f, 0}
#define g_ {NOTE_g, 0}
#define a_ {NOTE_a, 0}
#define h_ {NOTE_h, 0}
#define C_ {NOTE_C, 0}

Note song[] = {
  
     // INTRO
     {SET_BPM, 120},
     {NOTE_c, 16}, {NOTE_d, 16}, {NOTE_e, 16}, {NOTE_f, 16},
     {NOTE_g, 16}, {NOTE_a, 16}, {NOTE_h, 16}, {NOTE_C, 16},
     {END_OF_SONG},   
        
     // Fuchs, du hast die Gans gestohlen
     {SET_BPM, 120},
     c8, d8, e8, f8, g8, g8, g8, g8, 
     a8, f8, C8, a8, g2,
     a8, f8, C8, a8, g2,
     g8, f8, f8, f8, f8, e8, e8, e8,
     e8, d8, e8, d8, c8, e8, g4, 
     g8, f8, f8, f8, f8, e8, e8, e8,
     e8, d8, e8, d8, c2,
     {END_OF_SONG}, 
         
     // DUCK SONG
     {SET_BPM, 180},
     c4, d4, e4, f4, g2, g2, 
     a4, a4, a4, a4, g1,
     a4, a4, a4, a4, g1,
     f4, f4, f4, f4, e2, e2, 
     g4, g4, g4, g4, c1,   
     {END_OF_SONG},
     
     // Twinkle twinkle little star
     {SET_BPM, 140},
     c4, c4, g4, g4, a4, a4, g2, f4, f4, e4, e4, d4, d4, c2,
     g4, g4, f4, f4, e4, e4, d2, g4, g4, f4, f4, e4, e4, d2,
     c4, c4, g4, g4, a4, a4, g2, f4, f4, e4, e4, d4, d4, c2,
     {END_OF_SONG},
  
     // mon chapeau
     {SET_BPM, 210},
     g2, g4, C2, g4, g4, f4, e4, f4, d2,
     _2, d8, e8, f2, g4, a2, g4, e2, _4,
     _2, g4, C2, g4, g4, f4, e4, f4, d2,
     _2, d8, e8, f2, g4, a2, g4, c2, _4, 
     {END_OF_SONG},
     
     // Alle Vögel sind schon da
     {SET_BPM, 130},
     c4, _8, e8, g4, C4, a4, C8, a8, g2, f4, _8, g8, e4, c4, d2, c2,
     {REPEAT, 2}, 
     g4, g4, f4, f4, e4, g8, e8, d2, 
     {END_REPEAT},
     c4, _8, e8, g4, C4, a4, C8, a8, g2, f4, _8, g8, e4, c4, d2, c2,
     {END_OF_SONG},
     
     // Guten Abend, gut Nacht
     {SET_BPM, 240},
     e4, e4, c_, g2, _4, e4, e2, c_, g2, _4, 
     e4, g4, c_, C2, h2, _4, a4, d_, f_, a2, g2, d4, e4, 
     d_, f2, d2, d4, e4, d_, f2, _2, d4, f4, d_, g_, h4, a4, g2, h2, c_, e_, C2, _2,
     c4, c4, f_, a_, C1, a4, f4, c_, g1, e4, c4, d_, f2, g2, a2, c_, g1,
     c4, c4, f_, a_, C1, a4, f4, c_, g1, e4, c4, d_, f2, e2, d2, c_, e_, g_, C1, 
     {END_OF_SONG},
     
     // Arpeggio
     {SET_BPM, 120},
     {REPEAT, 2},
     c8, e8, g8, e8, c8, e8, g8, e8,
     c8, f8, a8, f8, c8, f8, a8, f8, 
     {END_REPEAT},
     d8, g8, h8, g8, f8, g8, h8, d8, 
     c_, e_, g_, C1,
     {END_OF_SONG},
     
     {DA_CAPO}
};

const int strike_length = 20; // solenoid on time in ms
float     bpm  = 120.0;       // current beats per minute
int       loop_count = 0;     // number of repetitions to go
int       loop_start = 0;     // beginning of loop (first note)
int       pos = 0;            // current note position
State     state = INTRO;      // global program state

typedef struct {
  char event;
  char param;
  bool valid;
} Message;

bool    new_msg = true;  // for serial FSM: next byte is a new message
Message msg;             // current message to be processed

bool    pending = false;  // incomplete message pending 

Timer t;

// calculate note duration according to note value and current bpm
int duration(int val){
  if (val == 0){
    return 0;    
  } else {
    float beat = 4.0 * 1000.0 * (60.0 / bpm);
    return (int)(beat / (float)val);
  }
}

void strike(int pin){
  digitalWrite(pin, HIGH);
  digitalWrite(led, HIGH);
}

void unstrike(){
  digitalWrite(led, LOW);
  for (int i=NOTE_c; i<=NOTE_C; ++i){
    digitalWrite(i, LOW);
  } 
}

int event = -1;

void playNote(){
  if (state == STOP){
    return;  
  }
  
  // get next note
  Note n = song[pos];
  while (n.pitch != END_OF_SONG){
    if (n.pitch == PAUSE || (n.pitch >= NOTE_c && n.pitch <= NOTE_C)){
      
      // play note
      if (n.pitch != PAUSE){
        strike(n.pitch);
      }
      Serial.println(n.pitch);
      Serial.println(n.dur);
      ++pos;
      if (n.dur > 0){
        
        // exit loop 
        break;  
      }
    } else if (n.pitch == GOTO){
      pos = n.dur; 
    } else if (n.pitch == SET_BPM){
      bpm = (float)n.dur;
      ++pos;    
    } else if (n.pitch == REPEAT){
      loop_count = n.dur;
      loop_start = pos + 1;
      ++pos;    
    } else if (n.pitch == END_REPEAT){
      --loop_count;
      if (loop_count == 0){
        ++pos;
      } else {
        pos = loop_start;
      }
    } else if (n.pitch == DA_CAPO){
      pos = 0; 
    } else {
      ++pos; 
    }
    
    // get next note
    n = song[pos];
  }
  if (n.pitch == END_OF_SONG){
    state = STOP;
    ++pos;
    unstrike();
    return;
  } 
    
  int d = duration(n.dur);
  event = t.after(d, playNote);
  delay(strike_length);
  unstrike();

  Serial.println(millis());
  Serial.println("---------");
}

void setup(){
  pinMode(button, INPUT_PULLUP);
  
  pinMode(NOTE_c, OUTPUT);
  pinMode(NOTE_d, OUTPUT);
  pinMode(NOTE_e, OUTPUT);
  pinMode(NOTE_f, OUTPUT);
  pinMode(NOTE_g, OUTPUT);
  pinMode(NOTE_a, OUTPUT);
  pinMode(NOTE_h, OUTPUT);
  pinMode(NOTE_C, OUTPUT);
    
  pinMode(led, OUTPUT);
  Serial.begin(9600);
  
  delay(100);
  if (Serial){
    state = SLAVE;
  }
}

void loop(){
  t.update();
  
  if (state == INTRO){
    state = PLAY;
    playNote();
  } else if (state == STOP){
    if (Serial){
      state = SLAVE;  
    } else {
      if (!digitalRead(button)){
        state = PLAY;
        playNote();
      }
    }
  } else if (state == SLAVE){
    processMessage();
  }
}

void panic()
{
  unstrike();
  
  // invalidate message
  msg.valid = false;
  new_msg = true;
  pending = false;
}


void processMessage(){
    if (msg.valid){   
    if (msg.event == PANIC){
       panic(); 
       msg.valid = false;
    } else if (msg.event == ALIVE){

      // FIXME: reset timer here for heart beat function
      msg.valid = false;      
    } else if (msg.event == NOTE_ON){
      int k = msg.param;
      
      strike(k+2);
      delay(strike_length);
      unstrike(); // FIXME: concurrent version?
      
      msg.valid = false;
      // Serial.println("Note on");
    } else if (msg.event == NOTE_OFF){
 
      // FIXME: do something?
 
      msg.valid = false;
      // Serial.println("Note off");
    }  else if (msg.event == WHORU){
       Serial.write(FISHBOT); 
       Serial.write(IAM);
       msg.valid = false; 
    } else {
       
       // ignore message
       msg.valid = false; 
    }
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

