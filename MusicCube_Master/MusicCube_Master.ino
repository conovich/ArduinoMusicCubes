#include <LinkedList.h>

#include <Adafruit_NeoPixel.h>
#include "pitches.h"
//MASTER

/**
 * Example code for using a microchip mrf24j40 module to send and receive
 * packets using plain 802.15.4
 * Requirements: 3 pins for spi, 3 pins for reset, chip select and interrupt
 * notifications
 * This example file is considered to be in the public domain
 * Originally written by Karl Palsson, karlp@tweak.net.au, March 2011
 */
#include <SPI.h>
#include <mrf24j.h>


//WIRELESS
const int pin_reset = 6;
const int pin_cs = 10; // default CS pin on ATmega8/168/328
const int pin_interrupt = 2; // default interrupt pin on ATmega8/168/328

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

long last_time;
long tx_interval = 1000;

uint8_t receivedNote;
uint8_t receivedState;


//STATE VARIABLES
byte firstPin = 7;
byte topState = 0;

byte secondPin = 5;
byte rightState = 0;

byte thirdPin = 4;
byte bottomState = 0;

byte fourthPin = A1;
byte leftState = 0;

//RGB VARIABLES
#define colorPin A0
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      4
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, colorPin, NEO_GRB + NEO_KHZ800);


//SOUND VARIABLES
char myNote = 'C';
#define soundPin1 9
#define soundPin2 3
LinkedList<char> *playList1; //the bottom most playlist
LinkedList<char> *playList2; //the next playlist, going upward

//BUTTON VARIABLES
byte buttonPin = 8;
int buttonState = 0;



void setup() {
  Serial.begin(9600);
  
  mrf.reset();
  mrf.init();
  
  mrf.set_pan(0xcafe);
  // This is _our_ address
  mrf.address16_write(0x4202); //0x4202

  // uncomment if you want to receive any packet on this channel
  mrf.set_promiscuous(true);
  
  // uncomment if you want to enable PA/LNA external control
  //mrf.set_palna(true);
  
  // uncomment if you want to buffer all PHY Payload
  //mrf.set_bufferPHY(true);

  attachInterrupt(0, interrupt_routine, CHANGE); // interrupt 0 equivalent to pin 2(INT0) on ATmega8/168/328
  last_time = millis();
  interrupts();
  
  playList1 = new LinkedList<char>();
  playList2 = new LinkedList<char>();
  pixels.begin();
  InitPins();
}

void InitPins(){
  pinMode(firstPin, INPUT); 
  pinMode(secondPin, INPUT); 
  pinMode(thirdPin, INPUT); 
  pinMode(fourthPin, INPUT); 
  
  pinMode(buttonPin, INPUT);
  if(digitalRead(buttonPin) == 1){
     buttonState = 1;
  }
  else{
     buttonState = 0; 
  }
  
  
  SetRGB(255,255,0);
  pinMode(colorPin, OUTPUT);
}

void interrupt_routine() {
    mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  PollMagneticPins();
  
  CheckButtonState();
  
  HandleWireless();
  
  PlayMusic();
}

//checks state of the state pins!
void PollMagneticPins(){
  if(digitalRead(firstPin) == 1){
    topState = 1;     
  }
  else{
    topState = 0; 
  }
  if(digitalRead(secondPin) == 1){
    rightState = 1;  
  }
  else{
    rightState = 0; 
  }
  if(digitalRead(thirdPin) == 1){
    bottomState = 1; 
  }
  else{
    bottomState = 0; 
  }
  if(digitalRead(fourthPin) == 1){
    leftState = 1;
  }
  else{
    leftState = 0; 
  }
  //Serial.println(topState + rightState*2 + bottomState*4 + leftState*8);
}

void CheckButtonState(){
  if(buttonState == 0 && digitalRead(buttonPin) == 1){
     buttonState = 1;
     ChangeNote();
     Serial.println("button state 1");
  }
  else if(buttonState == 1 && digitalRead(buttonPin) == 0){
     buttonState = 0;
     ChangeNote();
     Serial.println("button state 0");
  }
}

void ComputePlayList(){
 playList1->clear();
 playList1->add(myNote);
 playList2->clear();
 //playList2->add(myNote);
 
  //TOP SWITCH: 1
  //RIGHT SWITCH: 2
  //BOTTOM SWITCH: 3
  //LEFT SWITCH: 4
  
  if(topState == 1){
    if(receivedNote != 0){
      Serial.println("i should add a top note!");
      char noteToAdd = GetReceivedNoteAsChar();
      if (noteToAdd != 'Z'){
        playList2->add(noteToAdd);
        Serial.println(playList2->size());
        Serial.println (noteToAdd); 
      }
      else{
       Serial.println ("ARGHHH."); 
      }
    }
  }
  else if(rightState == 1 || leftState == 1){
    Serial.println("i should add a side note!");
      char noteToAdd = GetReceivedNoteAsChar();
      if (noteToAdd != 'Z'){
        playList1->add(noteToAdd);
        Serial.println (noteToAdd); 
      }
      else{
       Serial.println ("ARGHHH."); 
      }
  }
}

char GetReceivedNoteAsChar(){
  switch(receivedNote){
    case 65:
      return 'A';
    break;
    case 66:
      return 'B';
    break;
    case 67:
      return 'C';
    break;
    case 68:
      return 'D';
    break;
    case 69:
      return 'E';
    break;
    case 70:
      return 'F';
    break;
    case 71:
      return 'G';
    break;
    case 72:
      return 'H';
    break;
  }
  return 'Z';
}

void PlayMusic(){
  //Serial.println("should play music");
  
  //TODO: deal with connections here
  //int playListLength = 1;
  //char playList[1];
  ComputePlayList();
  
  for(int i = 0; i < playList1->size(); i++){
    if(playList2->size() > i){
      Serial.println("TWO NOTES OH HEYYY");
      PlayTwoNotes(playList1->get(i), playList2->get(i), soundPin1, soundPin2);
    }
    else{
      Serial.println("PLAY ONE NOTE. ONLY ONE.");
      Serial.println(myNote);
      PlayNote(playList1->get(i), soundPin1);
    }
  }
}

void PlayNote(char note, int pin){
  // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
  int noteDuration = 1000/2;
  
  if(note == 'C'){
    tone(pin, NOTE_C4, noteDuration);
  }
  else if(note == 'D'){
    tone(pin, NOTE_D4, noteDuration);
  }
  else if(note == 'E'){
    tone(pin, NOTE_E4, noteDuration);
  }
  else if(note == 'F'){
    tone(pin, NOTE_F4, noteDuration);
  }
  else if(note == 'G'){
    tone(pin, NOTE_G4, noteDuration);
  }
  else if(note == 'A'){
    tone(pin, NOTE_A4, noteDuration);
  }
  else if(note == 'B'){
    tone(pin, NOTE_B4, noteDuration);
  }
  else if(note == 'H'){ //ACTUALLY A HIGH C. YES I KNOW THIS IS WEIRD.
    tone(pin, NOTE_C5, noteDuration);
  }
  
  int pauseBetweenNotes = noteDuration*2; 
  delay(pauseBetweenNotes);
  noTone(pin);
}

#define realNoteDuration 500
#define switchingDuration 10
//int durationCount = 0;
//int switchCount = 0;
//char currentNote;

//AHHH ARDUINO TONE LIBRARY CANT DO THIS
void PlayTwoNotes(char note1, char note2, int pin1, int pin2){
  // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int durationCount = 0;
    int switchCount = 0;
    char currentNote = note1; 
    
    int noteDuration = 1000/2;
    int divider = 10;
    int switchDuration = noteDuration/divider;
    
  while(switchCount < divider){
        switchCount++;
        
        if(currentNote == note1){
         currentNote = note2; 
        }
        else if(currentNote == note2){
         currentNote = note1; 
        }
        
     if(currentNote == note1){
        if(note1 == 'C'){
          tone(pin1, NOTE_C4, switchDuration);
        }
        else if(note1 == 'D'){
          tone(pin1, NOTE_D4, switchDuration);
        }
        else if(note1 == 'E'){
          tone(pin1, NOTE_E4, switchDuration);
        }
        else if(note1 == 'F'){
          tone(pin1, NOTE_F4, switchDuration);
        }
        else if(note1 == 'G'){
          tone(pin1, NOTE_G4, switchDuration);
        }
        else if(note1 == 'A'){
          tone(pin1, NOTE_A4, switchDuration);
        }
        else if(note1 == 'B'){
          tone(pin1, NOTE_B4, switchDuration);
        }
        else if(note1 == 'H'){ //ACTUALLY A HIGH C. YES I KNOW THIS IS WEIRD.
          tone(pin1, NOTE_C5, switchDuration);
        }
      }
      
      else if(currentNote = note2){
        if(note2 == 'C'){
          tone(pin2, NOTE_C4, switchDuration);
        }
        else if(note2 == 'D'){
          tone(pin2, NOTE_D4, switchDuration);
        }
        else if(note2 == 'E'){
          tone(pin2, NOTE_E4, switchDuration);
        }
        else if(note2 == 'F'){
          tone(pin2, NOTE_F4, switchDuration);
        }
        else if(note2 == 'G'){
          tone(pin2, NOTE_G4, switchDuration);
        }
        else if(note2 == 'A'){
          tone(pin2, NOTE_A4, switchDuration);
        }
        else if(note2 == 'B'){
          tone(pin2, NOTE_B4, switchDuration);
        }
        else if(note2 == 'H'){ //ACTUALLY A HIGH C. YES I KNOW THIS IS WEIRD.
          tone(pin2, NOTE_C5, switchDuration);
        }
      }
      
      delay(switchDuration);
      noTone(pin1);
      noTone(pin2);
      

  }
  
  
  int pauseBetweenNotes = noteDuration; 
  delay(pauseBetweenNotes);
  noTone(pin1);
  noTone(pin2);
    
    
  /*  
  if(durationCount == 0){
   currentNote = note1; 
  }

  durationCount++;
  if(durationCount == realNoteDuration){
   //turn off both tones
      noTone(pin1);
      noTone(pin2);
  }
  
  switchCount++;
  if(switchCount = switchingDuration){
    switchCount = 0;
     if(currentNote == note1){
       currentNote = note2;
       noTone(pin1);
     }
    else{
      currentNote = note1;
      noTone(pin2);
    } 
  }
  
  if(currentNote == note1){
    if(note1 == 'C'){
      tone(pin1, NOTE_C4, noteDuration);
    }
    else if(note1 == 'D'){
      tone(pin1, NOTE_D4, noteDuration);
    }
    else if(note1 == 'E'){
      tone(pin1, NOTE_E4, noteDuration);
    }
    else if(note1 == 'F'){
      tone(pin1, NOTE_F4, noteDuration);
    }
    else if(note1 == 'G'){
      tone(pin1, NOTE_G4, noteDuration);
    }
    else if(note1 == 'A'){
      tone(pin1, NOTE_A4, noteDuration);
    }
    else if(note1 == 'B'){
      tone(pin1, NOTE_B4, noteDuration);
    }
    else if(note1 == 'H'){ //ACTUALLY A HIGH C. YES I KNOW THIS IS WEIRD.
      tone(pin1, NOTE_C5, noteDuration);
    }
  }
  
  else if(currentNote = note2){
    if(note2 == 'C'){
      tone(pin2, NOTE_C4, noteDuration);
    }
    else if(note2 == 'D'){
      tone(pin2, NOTE_D4, noteDuration);
    }
    else if(note2 == 'E'){
      tone(pin2, NOTE_E4, noteDuration);
    }
    else if(note2 == 'F'){
      tone(pin2, NOTE_F4, noteDuration);
    }
    else if(note2 == 'G'){
      tone(pin2, NOTE_G4, noteDuration);
    }
    else if(note2 == 'A'){
      tone(pin2, NOTE_A4, noteDuration);
    }
    else if(note2 == 'B'){
      tone(pin2, NOTE_B4, noteDuration);
    }
    else if(note2 == 'H'){ //ACTUALLY A HIGH C. YES I KNOW THIS IS WEIRD.
      tone(pin2, NOTE_C5, noteDuration);
    }
  }
  
  
  int pauseBetweenNotes = noteDuration*2; 
  delay(pauseBetweenNotes);
  noTone(pin1);
  noTone(pin2);*/
}

//COLOR
void SetRGB(int R, int G, int B){
   
   for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
     Serial.println("setting color");
    pixels.setPixelColor(i, pixels.Color(R,G,B));
    pixels.show(); // This sends the updated pixel color to the hardware.
    //delay(delayval); // Delay for a period of time (in milliseconds).
  }
}

//SOUND
void ChangeNote(){
  if(myNote == 'C'){
    myNote = 'D';
    SetRGB(0,255,0); //GREEN
  }
  else if(myNote == 'D'){
    myNote = 'E';
    SetRGB(0,246,238); //AQUA
  }
  else if(myNote == 'E'){
    myNote = 'F';
    SetRGB(0,0,255); //BLUE
  }
  else if(myNote == 'F'){
    myNote = 'G';
    SetRGB(127,0,255); //PURPLE
  }
  else if(myNote == 'G'){
    myNote = 'A';
    SetRGB(246,0,246); //PINK
  }
  else if(myNote == 'A'){
    myNote = 'B';
    SetRGB(246,115,0); //ORANGE
  }
  else if(myNote == 'B'){
    myNote = 'H';
    SetRGB(255,255,0); //YELLOW
  }
  else if(myNote == 'H'){ //actually corresponds to a higher C note
    myNote = 'C';
    SetRGB(255,0,0); //RED
  }

}

//WIRELESS
void HandleWireless(){
  mrf.check_flags(&handle_rx, &handle_tx);
    unsigned long current_time = millis();
    if (current_time - last_time > tx_interval) {
        last_time = current_time;
        //Serial.println("txxxing... to 0x6001");
       // mrf.send16(0x6001, "abcd");
    }
}

void handle_rx() {
    Serial.print("rec eived a packet ");Serial.print(mrf.get_rxinfo()->frame_length, DEC);Serial.println(" bytes long");
    
    if(mrf.get_bufferPHY()){
      Serial.println("Packet data (PHY Payload):");
      for (int i = 0; i < mrf.get_rxinfo()->frame_length; i++) {
        if(mrf.get_rxinfo()->frame_length == 1){
          Serial.print(mrf.get_rxbuf()[i]);
        }
      }
    }
    
    Serial.println("\r\nASCII data (relevant data):");
    for (int i = 0; i < mrf.rx_datalength(); i++) {
        Serial.write(mrf.get_rxinfo()->rx_data[i]);
        if(mrf.rx_datalength() == 1){
         receivedNote = mrf.get_rxinfo()->rx_data[i];
         Serial.println("incoming note ascii!");
         Serial.println(receivedNote);
        }
        /*else if(mrf.rx_datalength() == 4){
          receivedState = mrf.get_rxinfo()->rx_data[i];
          Serial.println("incoming state!");
        }*/
    }
    
    Serial.print("\r\nLQI/RSSI=");
    Serial.print(mrf.get_rxinfo()->lqi, DEC);
    Serial.print("/");
    Serial.println(mrf.get_rxinfo()->rssi, DEC);
}

void handle_tx() {
    if (mrf.get_txinfo()->tx_ok) {
        Serial.println("TX went ok, got ack");
    } else {
        Serial.print("TX failed after ");Serial.print(mrf.get_txinfo()->retries);Serial.println(" retries\n");
    }
}
