#include <Adafruit_NeoPixel.h>

//SLAVE

/**
 * Example code for using a microchip mrf24j40 module to send and receive
 * packets using plain 802.15.4
 * Requirements: 3 pins for spi, 3 pins for reset, chip select and interrupt
 * notifications
 * This example file is considered to be in the public domain
 * Originally written by Karl Palsson, karlp@tweak.net.au, March 2011
 */
 #include <Adafruit_NeoPixel.h>

//WIRELESS
#include <SPI.h>
#include <mrf24j.h>

const int pin_reset = 6;
const int pin_cs = 10; // default CS pin on ATmega8/168/328
const int pin_interrupt = 2; // default interrupt pin on ATmega8/168/328

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

long last_time;
long tx_interval = 1000;



//STATE VARIABLES
byte firstPin = 7;
byte firstState = 0;

byte secondPin = 5;
byte secondState = 0;

byte thirdPin = 4;
byte thirdState = 0;

byte fourthPin = 3;
byte fourthState = 0;

//RGB VARIABLES
#define colorPin 9
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      4
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, colorPin, NEO_GRB + NEO_KHZ800);


//SOUND VARIABLES
char myNote = 'C';

//BUTTON VARIABLES
byte buttonPin = 8;
int buttonState = 0;

void setup() {
  Serial.begin(9600);
  
  mrf.reset();
  mrf.init();
  
  mrf.set_pan(0xcafe);
  // This is _our_ address
  mrf.address16_write(0x6001); //0x4202

  // uncomment if you want to receive any packet on this channel
  mrf.set_promiscuous(true);
  
  // uncomment if you want to enable PA/LNA external control
  //mrf.set_palna(true);
  
  // uncomment if you want to buffer all PHY Payload
  //mrf.set_bufferPHY(true);

  attachInterrupt(0, interrupt_routine, CHANGE); // interrupt 0 equivalent to pin 2(INT0) on ATmega8/168/328
  last_time = millis();
  interrupts();
  
  
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
}

//checks state of the state pins!
void PollMagneticPins(){
  if(digitalRead(firstPin) == 1){
    firstState = 1;     
  }
  else{
    firstState = 0; 
  }
  if(digitalRead(secondPin) == 1){
    secondState = 1;  
  }
  else{
    secondState = 0; 
  }
  if(digitalRead(thirdPin) == 1){
    thirdState = 1; 
  }
  else{
    thirdState = 0; 
  }
  if(digitalRead(fourthPin) == 1){
    fourthState = 1;
  }
  else{
    fourthState = 0; 
  }
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
        
        byte state = firstState + secondState*2 + thirdState*4 + fourthState*8;

        char* stateArray = new char[1];
        Serial.println(String(state));
        
        String(state).toCharArray(stateArray, 1);
        Serial.println(stateArray);
        
        Serial.println("txxxing... to 0x4202");
        Serial.println(stateArray);
       // mrf.send16(0x4202, stateArray);
        
        //delay(100);
        char* noteArray = new char[1];
        noteArray[0] = myNote;
        Serial.println(noteArray);
        mrf.send16(0x4202, noteArray);
        
        //char* noteArray = new char[1];
        //noteArray[0] = myNote;
        //mrf.send16(0x4202, noteArray);
    }
}

void handle_rx() {
    Serial.print("received a packet ");Serial.print(mrf.get_rxinfo()->frame_length, DEC);Serial.println(" bytes long");
    
    if(mrf.get_bufferPHY()){
      Serial.println("Packet data (PHY Payload):");
      for (int i = 0; i < mrf.get_rxinfo()->frame_length; i++) {
          Serial.print(mrf.get_rxbuf()[i]);
      }
    }
    
    Serial.println("\r\nASCII data (relevant data):");
    for (int i = 0; i < mrf.rx_datalength(); i++) {
        Serial.write(mrf.get_rxinfo()->rx_data[i]);
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
