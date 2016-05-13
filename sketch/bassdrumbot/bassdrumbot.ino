/*
 Control for BassDrumBot. 
 */

// messages
#define WHORU   0x26
#define IAM     0x27
#define BDBOT   0x02
#define NOTE_ON 0x01

// motor directions
#define DIR_UP   LOW
#define DIR_DOWN HIGH
#define DIR_DAMP 128

// motor pins
#define MOV 10

const int strikelength = 55;
const int releaselength = 25;
const int damplength = 20;


int event = 0;
int incomingByte = 0;


void setup() {
  
  //establish motor direction toggle pins
  pinMode(MOV, OUTPUT); 
  digitalWrite(MOV, DIR_UP); 
  
  Serial.begin(9600);
}


void beat() {
   // strike
   digitalWrite(MOV, DIR_DOWN);   
   delay(strikelength);
   
   // stop
 //  digitalWrite(MOV, DIR_UP);
 //  delay(releaselength);
 
   // damping
 //  analogWrite(MOV, DIR_DAMP);
 //  delay(damplength);

   // ready for next strike
   digitalWrite(MOV, DIR_UP);   
}



void loop() { 
 static int chan = 0;
  
 if(event == 1){
   event = 0;
   
   if (incomingByte == NOTE_ON) {
      beat();
   }
   
   if (incomingByte == WHORU) {
      Serial.write(BDBOT);
      Serial.write(IAM);
   }
 }
}


void serialEvent(){
  if (!event && Serial.available()){
    event = 1;
    incomingByte = Serial.read();     
  }
}


