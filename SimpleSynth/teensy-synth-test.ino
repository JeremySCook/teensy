const int button1 = 41;
const int button2 = 40;
const int button3 = 39;
const int button4 = 38;

const int delayTime = 10;

const int noteA4 = 440;
const int noteB4 = 494;
const int noteC5 = 523;
const int noteD5 = 587;

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=784.5,414
AudioOutputI2S           i2s1;           //xy=1022.5,448
AudioConnection          patchCord1(waveform1, 0, i2s1, 0);
AudioConnection          patchCord2(waveform1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=756.5,617
// GUItool: end automatically generated code

void setup() {

  Serial.begin(9600);

  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.32);

  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.0);
  waveform1.frequency(50);
  waveform1.pulseWidth(0.15);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

if(digitalRead(button1) == LOW){
  digitalWrite(LED_BUILTIN, HIGH);
  waveform1.amplitude(0.5);
  waveform1.frequency(noteA4);
  delay(delayTime);
}
else if(digitalRead(button2) == LOW){
  digitalWrite(LED_BUILTIN, HIGH);
  waveform1.amplitude(0.5);
  waveform1.frequency(noteB4);
  delay(delayTime);
}
else if(digitalRead(button3) == LOW){
  digitalWrite(LED_BUILTIN, HIGH);
  waveform1.amplitude(0.5);
  waveform1.frequency(noteC5);
  delay(delayTime);
}
else if(digitalRead(button4) == LOW){
  digitalWrite(LED_BUILTIN, HIGH);
  waveform1.amplitude(0.5);
  waveform1.frequency(noteD5);
  delay(delayTime);
}
else{
  digitalWrite(LED_BUILTIN, LOW);
  waveform1.amplitude(0.0);
  delay(delayTime);
}
}
