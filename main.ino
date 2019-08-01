
#include "Adafruit_MPR121.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif


uint16_t touchedPins = 0;
uint16_t keyboardSwitched = 1;
uint16_t currentKeyboard = 0;
uint16_t keyboardLength = 3;
uint16_t numNotes = 0;
float cGain = 0.25;
byte currentNote = 0;
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
AudioInputI2S            i2s2;  
AudioRecordQueue         queue1;         
AudioPlaySdRaw           playRaw1;
AudioAnalyzePeak         peak1; 
AudioConnection          patchCord1(playWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playWav1, 1, mixer2, 0);
AudioConnection          patchCord3(playWav2, 0, mixer1, 1);
AudioConnection          patchCord4(playWav2, 1, mixer2, 1);
AudioConnection          patchCord5(playWav3, 0, mixer1, 2);
AudioConnection          patchCord6(playWav3, 1, mixer2, 2);
AudioConnection          patchCord7(playWav4, 0, mixer1, 3);
AudioConnection          patchCord8(playWav4, 1, mixer2, 3);
AudioConnection          patchCord9(mixer1, 0, audioOutput, 0);
AudioConnection          patchCord10(mixer2, 0, audioOutput, 1);
AudioConnection          patchCord11(i2s2, 0, queue1, 0);
AudioConnection          patchCord12(i2s2, 0, peak1, 0);
AudioConnection          patchCord13(playRaw1, 0, audioOutput, 0);
AudioConnection          patchCord14(playRaw1, 0, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;


// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

const int myInput = AUDIO_INPUT_MIC;
#define RECORD_BUTTON 15

int mode = 0; 
File frec;

elapsedMillis timeElapsed;
uint16_t interval = 10000;


void setup() {
  Serial.begin(9600);
   pinMode(KEYBOARD_SWITCH, INPUT_PULLUP);
  pinMode(RECORD_BUTTON, INPUT_PULLUP);

  
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(60);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
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
  
  

  mixer1.gain(0, cGain);
  mixer1.gain(1, cGain);
  mixer1.gain(2, cGain);
  mixer1.gain(3, cGain);
  mixer2.gain(0, cGain);
  mixer2.gain(1, cGain);
  mixer2.gain(2, cGain);
  mixer2.gain(3, cGain);
  
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
    
      if (notes != 0){
      cGain = 1.0/notes;
      mixer1.gain(0, cGain);
      mixer1.gain(1, cGain);
      mixer1.gain(2, cGain);
      mixer1.gain(3, cGain);
      mixer2.gain(0, cGain);
      mixer2.gain(1, cGain);
      mixer2.gain(2, cGain);
      mixer2.gain(3, cGain);
    }
  
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
  
  
    while (playWav1.isPlaying() || playWav2.isPlaying() || playWav3.isPlaying() || playWav4.isPlaying()) {
   
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
void startRecording() {
  Serial.println("startRecording");
  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if (frec) {
    queue1.begin();
    mode = 1;
  }
}

void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    //elapsedMicros usec = 0;
    frec.write(buffer, 512);
    // Uncomment these lines to see how long SD writes
    // are taking.  A pair of audio blocks arrives every
    // 5802 microseconds, so hopefully most of the writes
    // take well under 5802 us.  Some will take more, as
    // the SD library also must write to the FAT tables
    // and the SD card controller manages media erase and
    // wear leveling.  The queue1 object can buffer
    // approximately 301700 us of audio, to allow time
    // for occasional high SD card latency, as long as
    // the average write time is under 5802 us.
    //Serial.print("SD write, us=");
    //Serial.println(usec);
  }
}

void stopRecording() {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  mode = 0;
}

void playRawFile(){
  Serial.println("startPlaying");
  playRaw1.play("RECORD.RAW");
  while(playRaw1.isPlaying()){
    
  }
  Serial.println("donePlaying");
}

void recordRaw(uint16_t seconds){
  timeElapsed = 0;
  Serial.println("Oh No!");
  startRecording();
  while(timeElapsed < seconds) {
     continueRecording();
  };
  stopRecording();
  
}

void loop() {
  
 //check if keyboard has been switched
   //keyboard switching by incrementing array of filenames
   if(!digitalRead(KEYBOARD_SWITCH)){
    currentKeyboard = (currentKeyboard+1)%keyboardLength;
    //print name of first note in the new keyboard (for debugging help)
    Serial.print(keyboards[currentKeyboard][0]);
    delay(500);
   }

   if(!digitalRead(RECORD_BUTTON)){
    currentKeyboard = keyboardLength;
    delay(500);
    recordRaw(interval);
    }

   touchedPins = cap.touched(); 
   
   if(currentKeyboard == keyboardLength){
    if(touchedPins > 0){
      playRawFile(); 
    }
   }

   else{
  //check while inputs have been pressed on MPR 121 board and play corresponding chord
    numNotes = readNotes(touchedPins);
    playChord(chord, numNotes, currentKeyboard);
   }

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
