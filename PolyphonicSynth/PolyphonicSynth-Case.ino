#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7   // Teensy 4 ignores this, uses pin 11
#define SDCARD_SCK_PIN   14  // Teensy 4 ignores this, uses pin 13

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

const int playSD1Button = 35;
const int playSD2Button = 36;
const int playSD3Button = 37;
const int playSD4Button = 38;

bool octaveDown = false;
bool octaveUp = false;

bool SD1State = false;
bool SD2State = false;
bool SD3State = false;
bool SD4State = false;

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
TPA2016D2 tpaAmp;

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=465.5,354
AudioSynthWaveform       waveform3;      //xy=465.5,444
AudioSynthWaveform       waveform4;      //xy=465.5,487
AudioSynthWaveform       waveform2;      //xy=466,400
AudioPlaySdWav           playSD4;     //xy=468.5,673
AudioPlaySdWav           playSD3;     //xy=469.5,633
AudioPlaySdWav           playSD2;     //xy=470.5,592
AudioPlaySdWav           playSD1;     //xy=471.5,550
AudioEffectEnvelope      envelope2;      //xy=611.5,401
AudioEffectEnvelope      envelope3;      //xy=611.5,442
AudioEffectEnvelope      envelope4;      //xy=611.5,482
AudioEffectEnvelope      envelope1;      //xy=613.5,360
AudioMixer4              mixerSample;         //xy=764.5,573
AudioMixer4              mixerKeys;         //xy=769,421
AudioMixer4              mixerMaster;         //xy=955.5,493
AudioAmplifier           amp;           //xy=1110.5,493
AudioOutputI2S           i2s1;           //xy=1278.5,492
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(waveform3, envelope3);
AudioConnection          patchCord3(waveform4, envelope4);
AudioConnection          patchCord4(waveform2, envelope2);
AudioConnection          patchCord5(playSD4, 0, mixerSample, 3);
AudioConnection          patchCord6(playSD3, 0, mixerSample, 2);
AudioConnection          patchCord7(playSD2, 0, mixerSample, 1);
AudioConnection          patchCord8(playSD1, 0, mixerSample, 0);
AudioConnection          patchCord9(envelope2, 0, mixerKeys, 1);
AudioConnection          patchCord10(envelope3, 0, mixerKeys, 2);
AudioConnection          patchCord11(envelope4, 0, mixerKeys, 3);
AudioConnection          patchCord12(envelope1, 0, mixerKeys, 0);
AudioConnection          patchCord13(mixerSample, 0, mixerMaster, 1);
AudioConnection          patchCord14(mixerKeys, 0, mixerMaster, 0);
AudioConnection          patchCord15(mixerMaster, amp);
AudioConnection          patchCord16(amp, 0, i2s1, 0);
AudioConnection          patchCord17(amp, 0, i2s1, 1);
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

void updateOctaveButtons(){
  bool newOctaveDown = (digitalRead(octaveDownButton) == LOW);
  bool newOctaveUp   = (digitalRead(octaveUpButton) == LOW);

  if (newOctaveDown != octaveDown ||
    newOctaveUp   != octaveUp) {

    octaveDown = newOctaveDown;
    octaveUp   = newOctaveUp;
  }
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


void updateKeyboard() {

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
}

void updateSamples() {

    bool newSD1State = (digitalRead(playSD1Button) == LOW);

    if (newSD1State && !SD1State) {
        playSD1.play("KICK.WAV");
    }
    SD1State = newSD1State;


    bool newSD2State = (digitalRead(playSD2Button) == LOW);

    if (newSD2State && !SD2State) {
        playSD2.play("SNARE.WAV");
    }
    SD2State = newSD2State;


    bool newSD3State = (digitalRead(playSD3Button) == LOW);

    if (newSD3State && !SD3State) {
        playSD3.play("HIHAT.WAV");
    }
    SD3State = newSD3State;

  //Add a fourth voice for sample 4

}

void setup() {

  Serial.begin(9600);
  Wire.begin();

  if (tpaAmp.begin() == false) //Begin communication over I2C
  {
    Serial.println("The device did not respond. Please check wiring.");
    while (1); //Freeze
  }
  Serial.println("Device is connected properly.");

  // for gain control to react to changes quickly, we need to adjust some of the AGC settings as so...
  tpaAmp.disableLimiter(); // note this also changes compression ratio to 1:1, then disables limiter.
  tpaAmp.disableNoiseGate(); // disabling the noisegate allows us to always change the gain, even with very little sound at the source.
  tpaAmp.writeRelease(1); // 1-63 are valid values. 1 being the shortest (aka fastest) release setting, this allows gain increases to happen quickly.
  tpaAmp.writeAttack(1); // 1-63 are valid values. 1 being the shortest (aka fastest) attack setting, this allows gain decreases to happen quickly.

  Serial.println("gain:+10");
  tpaAmp.writeFixedGain(30); // aka "full gain at +30dB", accepts values from 0 to 30
  delay(5000);

  AudioMemory(80);
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

  //Keys mixer
  mixerKeys.gain(0, 0.25);
  mixerKeys.gain(1, 0.25);
  mixerKeys.gain(2, 0.25);
  mixerKeys.gain(3, 0.25);

  // Drum mixer
  mixerSample.gain(0, 1.0);   // Hat
  mixerSample.gain(1, 1.0);   // Snare
  mixerSample.gain(2, 1.0);   // Kick

  // Final mixer
  mixerMaster.gain(0, 0.7);   // Synth
  mixerMaster.gain(1, 0.7);   // Drums

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
  pinMode(playSD1Button, INPUT_PULLUP);
  pinMode(playSD2Button, INPUT_PULLUP);
  pinMode(playSD3Button, INPUT_PULLUP);

//SD Card Setup:

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

}

// Main loop

void loop() {
  updateOctaveButtons();
  updateKeyboard();
  updateSamples();
  updateActiveNotes();

// UPDATE LED BELOW DOESN'T SEEM TO WORK

bool anyKey = false;
for (int i = 0; i < numberButtons; i++) {
  if (keyState[i]) anyKey = true;
}

digitalWrite(LED_BUILTIN, anyKey);
}
