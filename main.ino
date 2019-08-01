
#include "Adafruit_MPR121.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
uint16_t keyboardswitch = 1;
uint16_t currentkeyboard = 0;
uint16_t keyboardlength =  3;
uint16_t numNotes = 0;
float cGain = 0.25;
byte noteTesting = 0;
int chord[] = {0,0,0,0};
#define KEYBOARD_SWITCH 16


const char* keyboards[][12] = {
{"bd_zome.wav","closed-hat.wav","crash.wav","drum_cowbell.wav",
"elec_blip2.wav","elec_cymbal.wav","elec_hi_snare.wav",
"open-hat.wav","open-then-closed-hat.wav","ride-bell.wav",
"snare.wav","tom-low.wav"},
  {"freq261.wav","freq277.wav","freq294.wav","freq311.wav",
  "freq329.wav","freq349.wav","freq370.wav","freq392.wav",
  "freq415.wav","freq440.wav","freq466.wav","freq494.wav"},
  {"catf.wav","catfs.wav","catgs.wav","catg.wav","cata.wav",
"catbb.wav","catb.wav","catc.wav","catcs.wav","catd.wav",
"catds.wav","cate.wav"}
  };




Adafruit_MPR121 cap = Adafruit_MPR121();

AudioPlaySdWav           playWav1; 
AudioPlaySdWav           playWav2; 
AudioPlaySdWav           playWav3; 
AudioPlaySdWav           playWav4; 
AudioMixer4              mixer1;
AudioMixer4              mixer2;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioConnection          patchCord3(playWav2, 0, audioOutput, 0);
AudioConnection          patchCord4(playWav2, 1, audioOutput, 1);
AudioConnection          patchCord5(playWav3, 0, audioOutput, 0);
AudioConnection          patchCord6(playWav3, 1, audioOutput, 1);
AudioConnection          patchCord7(playWav4, 0, audioOutput, 0);
AudioConnection          patchCord8(playWav4, 1, audioOutput, 1);
AudioConnection          patchCord9(mixer1, 0, audioOutput, 0);
AudioConnection          patchCord10(mixer2, 0, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14


void setup() {
  Serial.begin(9600);
   pinMode(KEYBOARD_SWITCH, INPUT_PULLUP);

  
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  
  
  mixer1.gain(0, 0.25);
  mixer1.gain(1, 0.25);
  mixer1.gain(2, 0.25);
  mixer1.gain(3, 0.25);
  mixer2.gain(0, 0.25);
  mixer2.gain(1, 0.25);
  mixer2.gain(2, 0.25);
  mixer2.gain(3, 0.25);
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  cap.setThresholds(8,6);

}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
   while (playWav1.isPlaying()) {
   
  }
}

void playChord(int chord[], int notes, int ckeyboard){
  
    if (playWav1.isPlaying() == false && notes > 0) {
    Serial.println("Start playing 1");
    playWav1.play(keyboards[ckeyboard][chord[0]]);
    delay(10); // wait for library to parse WAV info
  }
  if (playWav2.isPlaying() == false && notes > 1) {
    Serial.println("Start playing 2");
    playWav2.play(keyboards[ckeyboard][chord[1]]);
    delay(10); // wait for library to parse WAV info
  }
    if (playWav3.isPlaying() == false && notes > 2) {
    Serial.println("Start playing 3");
    playWav1.play(keyboards[ckeyboard][chord[2]]);
    delay(10); // wait for library to parse WAV info
  }
  if (playWav4.isPlaying() == false && notes > 3) {
    Serial.println("Start playing 4");
    playWav2.play(keyboards[ckeyboard][chord[3]]);
    delay(10); // wait for library to parse WAV info
  }
}

int readNotes(int notes){
  for (uint8_t i=0; i<4; i++){
    chord[i]=0;
  }
  numNotes = 0;
   for (uint8_t i=0; i<12; i++) {
     noteTesting =  bitRead(notes, 11-i);
     if(noteTesting == 1){
      chord[numNotes] = i; 
      numNotes ++;
     }
   }
   Serial.print('\n');
   Serial.print("chord:");
   for (uint8_t i=0; i<4; i++){
    Serial.print(chord[i]);
  }
  Serial.print('\n');
   delay(1000);
 return numNotes;
}

void loop() {
  
   keyboardswitch = digitalRead(KEYBOARD_SWITCH);
   if(!keyboardswitch){
    currentkeyboard++;
    if( currentkeyboard == keyboardlength){
      currentkeyboard = 0;
    }
    Serial.print(keyboards[currentkeyboard][0]);
    delay(500);
   }
   
   currtouched = cap.touched();
  numNotes = readNotes(currtouched);
  playChord(chord, numNotes, currentkeyboard);
 
  /*
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched  alert!
    if ((currtouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
      playFile(keyboards[currentkeyboard][i]);  
    }
  }

  // reset our state
  lasttouched = currtouched;
*/
}
