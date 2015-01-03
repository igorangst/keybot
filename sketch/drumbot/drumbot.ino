/*
 Control for DrumBot. The motor control code is inspired by
 the code for a robotic drum created by Randy Sarafan, see
 http://www.instructables.com/id/Arduino-Controlled-Robotic-Drum/ 
 
 The server code for the ALSA-based Linux application can be found on
 https://github.com/igorangst/keybot
 
 */

// messages
#define WHORU   0x26
#define IAM     0x27
#define DRUMBOT 0x02
#define NOTE_ON 0x01

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

int event = 0;
int strikelength = 35;
int incomingByte = 0;


void setup() {
  
  //establish motor direction toggle pins
  pinMode(DIR_A, OUTPUT); 
  pinMode(DIR_B, OUTPUT); 
  
  //establish motor brake pins
  pinMode(BRK_A, OUTPUT); 
  pinMode(BRK_B, OUTPUT); 

  digitalWrite(BRK_A, LOW); // ENABLE CH A
  digitalWrite(BRK_B, LOW); // ENABLE CH B  
  
  Serial.begin(9600);
}


void beatA() {
   //start down
   digitalWrite(BRK_A, LOW); 
   digitalWrite(DIR_A, DIR_DOWN);   
   analogWrite(MOV_A, 255);   
       
   //strike time
   delay(strikelength);
       
   //stop
   digitalWrite(BRK_A, HIGH); 
   delay(10);
       
   //go back
   digitalWrite(BRK_A, LOW); 
   digitalWrite(DIR_A, DIR_UP);   
   analogWrite(MOV_A, 255);   
       
   //recoil time
   delay(strikelength);    
       
   //stop
   digitalWrite(BRK_A, HIGH); 
}

void beatB() {
   //start down
   digitalWrite(BRK_B, LOW); 
   digitalWrite(DIR_B, DIR_DOWN);   
   analogWrite(MOV_B, 255);  
       
   //strike time
   delay(strikelength);
       
   //stop
   digitalWrite(BRK_B, HIGH); 
   delay(10);
       
   //go back
   digitalWrite(BRK_B, LOW); 
   digitalWrite(DIR_B, DIR_UP); 
   analogWrite(MOV_B, 255);   
       
   //recoil time
   delay(strikelength);    
       
   //stop
   digitalWrite(BRK_B, HIGH); 
}


void loop() { 
 static int chan = 0;
  
 if(event == 1){
   event = 0;
   
   if (incomingByte == NOTE_ON) {
     if (chan == 0) {
       beatA();  
       chan = 1;
     } else {
       beatB();
       chan = 0;
     }  
   }
   
   if (incomingByte == WHORU) {
      Serial.write(DRUMBOT);
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


