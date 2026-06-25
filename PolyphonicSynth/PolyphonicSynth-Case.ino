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

const int octaveDownButton = 33;
const int octaveUpButton   = 34;

bool octaveDown = false;
bool octaveUp = false;

bool keyState[numberButtons];
bool prevKeyState[numberButtons];

bool voiceActive[numberVoices];
int voiceNote[numberVoices];

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <SparkFun_TPA2016D2_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_TPA2016D2
TPA2016D2 amp;

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=465.5,354
AudioSynthWaveform       waveform3;      //xy=465.5,444
AudioSynthWaveform       waveform2;      //xy=466,400
AudioSynthWaveform       waveform4;      //xy=466.5,489
AudioPlaySdWav           playSdWav3;     //xy=469.5,634
AudioPlaySdWav           playSdWav2;     //xy=470.5,593
AudioPlaySdWav           playSdWav1;     //xy=471.5,552
AudioEffectEnvelope      envelope2;      //xy=611.5,401
AudioEffectEnvelope      envelope3;      //xy=611.5,442
AudioEffectEnvelope      envelope4;      //xy=611.5,482
AudioEffectEnvelope      envelope1;      //xy=613.5,360
AudioMixer4              mixer2;         //xy=764.5,573
AudioMixer4              mixer1;         //xy=769,421
AudioMixer4              mixer3;         //xy=941.5,495
AudioOutputI2S           i2s1;           //xy=1117.5,495
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(waveform3, envelope3);
AudioConnection          patchCord3(waveform2, envelope2);
AudioConnection          patchCord4(waveform4, envelope4);
AudioConnection          patchCord5(playSdWav3, 0, mixer2, 2);
AudioConnection          patchCord6(playSdWav2, 0, mixer2, 1);
AudioConnection          patchCord7(playSdWav1, 0, mixer2, 0);
AudioConnection          patchCord8(envelope2, 0, mixer1, 1);
AudioConnection          patchCord9(envelope3, 0, mixer1, 2);
AudioConnection          patchCord10(envelope4, 0, mixer1, 3);
AudioConnection          patchCord11(envelope1, 0, mixer1, 0);
AudioConnection          patchCord12(mixer2, 0, mixer3, 1);
AudioConnection          patchCord13(mixer1, 0, mixer3, 0);
AudioConnection          patchCord14(mixer3, 0, i2s1, 1);
AudioConnection          patchCord15(mixer3, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=552.5,758
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

  waves[v]->frequency(getNoteFrequency(noteIndex));

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

float getNoteFrequency(int noteIndex) {

  float freq = notes[noteIndex];

  if (octaveUp && !octaveDown) {
    freq *= 2.0f;
  }
  else if (octaveDown && !octaveUp) {
    freq *= 0.5f;
  }

  return freq;
}

void updateActiveNotes() {

  for (int v = 0; v < numberVoices; v++) {

    if (voiceActive[v]) {

      waves[v]->frequency(
        getNoteFrequency(voiceNote[v])
      );
    }
  }
}

void setup() {

  Serial.begin(9600);
  Wire.begin();

  if (amp.begin() == false) //Begin communication over I2C
  {
    Serial.println("The device did not respond. Please check wiring.");
    while (1); //Freeze
  }
  Serial.println("Device is connected properly.");

  // for gain control to react to changes quickly, we need to adjust some of the AGC settings as so...
  amp.disableLimiter(); // note this also changes compression ratio to 1:1, then disables limiter.
  amp.disableNoiseGate(); // disabling the noisegate allows us to always change the gain, even with very little sound at the source.
  amp.writeRelease(1); // 1-63 are valid values. 1 being the shortest (aka fastest) release setting, this allows gain increases to happen quickly.
  amp.writeAttack(1); // 1-63 are valid values. 1 being the shortest (aka fastest) attack setting, this allows gain decreases to happen quickly.

  Serial.println("gain:+10");
  amp.writeFixedGain(30); // aka "full gain at +30dB", accepts values from 0 to 30
  delay(5000);

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

  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform3.begin(WAVEFORM_SAWTOOTH);
  waveform4.begin(WAVEFORM_SAWTOOTH);

  waveform1.frequency(100);
  waveform2.frequency(100);
  waveform3.frequency(100);
  waveform4.frequency(100);

for (int i = 0; i < numberButtons; i++) {
    lastChangeTime[i] = 0;
}

// Configure button input modes

for (int i = 0; i < numberButtons; i++) {
  pinMode(buttonPins[i], INPUT_PULLUP);
}
  pinMode(octaveDownButton, INPUT_PULLUP);
  pinMode(octaveUpButton, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

// Main loop

void loop() {

bool newOctaveDown = (digitalRead(octaveDownButton) == LOW);
bool newOctaveUp   = (digitalRead(octaveUpButton) == LOW);

if (newOctaveDown != octaveDown ||
    newOctaveUp   != octaveUp) {

  octaveDown = newOctaveDown;
  octaveUp   = newOctaveUp;

  updateActiveNotes();
}

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
