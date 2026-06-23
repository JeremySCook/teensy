const int numberButtons = 13;
const int numberVoices = 4;

unsigned long lastChangeTime[numberButtons];
const unsigned long debounceMs = 5;

const int buttonPins[numberButtons] = {24, 0, 25, 1, 26, 27, 2, 28, 3, 29, 4, 30, 31};
const float notes[numberButtons] = {
  130.81, // C3
  138.59, // C#3
  146.83, // D3
  155.56, // D#3
  164.81, // E3
  174.61, // F3
  185.00, // F#3
  196.00, // G3
  207.65, // G#3
  220.00, // A3
  233.08, // A#3
  246.94, // B3
  261.63  // C4
};




bool keyState[numberButtons];
bool prevKeyState[numberButtons];

bool voiceActive[numberVoices];
int voiceNote[numberVoices];

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=465.5,354
AudioSynthWaveform       waveform3;      //xy=465.5,444
AudioSynthWaveform       waveform2;      //xy=466,400
AudioSynthWaveform       waveform4;      //xy=466.5,489
AudioEffectEnvelope      envelope2;      //xy=611.5,401
AudioEffectEnvelope      envelope3;      //xy=611.5,442
AudioEffectEnvelope      envelope4;      //xy=611.5,482
AudioEffectEnvelope      envelope1;      //xy=613.5,360
AudioMixer4              mixer1;         //xy=772,417
AudioOutputI2S           i2s1;           //xy=946.5,416
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(waveform3, envelope3);
AudioConnection          patchCord3(waveform2, envelope2);
AudioConnection          patchCord4(waveform4, envelope4);
AudioConnection          patchCord5(envelope2, 0, mixer1, 1);
AudioConnection          patchCord6(envelope3, 0, mixer1, 2);
AudioConnection          patchCord7(envelope4, 0, mixer1, 3);
AudioConnection          patchCord8(envelope1, 0, mixer1, 0);
AudioConnection          patchCord9(mixer1, 0, i2s1, 0);
AudioConnection          patchCord10(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=649.5,604
// GUItool: end automatically generated code


AudioSynthWaveform* waves[numberVoices] = {
  &waveform1,
  &waveform2,
  &waveform3,
  &waveform4
};

AudioEffectEnvelope* envs[numberVoices] = {
  &envelope1,
  &envelope2,
  &envelope3,
  &envelope4
};

// HELPER FUNCTIONS

int findFreeVoice() {
  for (int i = 0; i < numberVoices; i++) {
    if (!voiceActive[i]) {
      return i;
    }
  }
  return -1;
}

void startNote(int noteIndex) {

  int v = findFreeVoice();

  if (v == -1) {
    return;   // no free voice
  }

  voiceActive[v] = true;
  voiceNote[v] = noteIndex;

  waves[v]->frequency(notes[noteIndex]);

  envs[v]->noteOn();
}

void stopNote(int noteIndex) {

  for (int v = 0; v < numberVoices; v++) {

    if (voiceActive[v] &&
        voiceNote[v] == noteIndex) {

      envs[v]->noteOff();

      voiceActive[v] = false;
      voiceNote[v] = -1;
    }
  }
}


void setup() {

  Serial.begin(9600);

  AudioMemory(40);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.32);

  for (int i = 0; i < numberVoices; i++) {
  voiceActive[i] = false;
  voiceNote[i] = -1;
}

  for (int i = 0; i < numberButtons; i++) {
  keyState[i] = false;
  prevKeyState[i] = false;
}

  envelope1.attack(5);
  envelope1.decay(50);
  envelope1.sustain(0.7);
  envelope1.release(100);

  envelope2.attack(5);
  envelope2.decay(50);
  envelope2.sustain(0.7);
  envelope2.release(100);

  envelope3.attack(5);
  envelope3.decay(50);
  envelope3.sustain(0.7);
  envelope3.release(100);

  envelope4.attack(5);
  envelope4.decay(50);
  envelope4.sustain(0.7);
  envelope4.release(100);

  mixer1.gain(0, 0.25);
  mixer1.gain(1, 0.25);
  mixer1.gain(2, 0.25);
  mixer1.gain(3, 0.25);

  waveform1.amplitude(1.0);
  waveform2.amplitude(1.0);
  waveform3.amplitude(1.0);
  waveform4.amplitude(1.0);

  waveform1.begin(WAVEFORM_SINE);
  waveform2.begin(WAVEFORM_SINE);
  waveform3.begin(WAVEFORM_SINE);
  waveform4.begin(WAVEFORM_SINE);

  waveform1.frequency(100);
  waveform2.frequency(100);
  waveform3.frequency(100);
  waveform4.frequency(100);

for (int i = 0; i < numberButtons; i++) {
    lastChangeTime[i] = 0;
}

for (int i = 0; i < numberButtons; i++) {
  pinMode(buttonPins[i], INPUT_PULLUP);
}
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

for (int i = 0; i < numberButtons; i++) {
bool rawState = (digitalRead(buttonPins[i]) == LOW);

if (rawState != keyState[i] &&
    millis() - lastChangeTime[i] > debounceMs) {

    lastChangeTime[i] = millis();

    keyState[i] = rawState;

    if (keyState[i]) {
        startNote(i);
    } else {
        stopNote(i);
    }
}
}

bool anyKey = false;
for (int i = 0; i < numberButtons; i++) {
  if (keyState[i]) anyKey = true;
}

digitalWrite(LED_BUILTIN, anyKey);

}
