#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Servo.h>

// -------------------------
// Audio objects
// -------------------------

AudioPlaySdWav       playWav;
AudioOutputI2S       i2s1;
AudioConnection      patchCord1(playWav, 0, i2s1, 0);
AudioConnection      patchCord2(playWav, 1, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;

// -------------------------
// Audio Shield SD card
// -------------------------

const int SD_CS = 10;

// -------------------------
// Button
// -------------------------

const int buttonPin = 33;
const unsigned long debounceMs = 20;

bool buttonState = HIGH;
bool lastReading = HIGH;
unsigned long lastDebounceTime = 0;

// -------------------------
// WAV files
// -------------------------

const char* wavFiles[8] = {
  "COWBOY01.WAV",
  "COWBOY02.WAV",
  "COWBOY03.WAV",
  "COWBOY04.WAV",
  "COWBOY05.WAV",
  "COWBOY06.WAV",
  "COWBOY07.WAV",
  "COWBOY08.WAV"
};

const int numClips = 8;
int lastClip = -1;

// -------------------------
// Servos
// -------------------------

Servo rightArm;
Servo leftArm;

const int rightArmPin = 34;
const int leftArmPin  = 35;

// Servo endpoints
// Adjust these later to suit the linkage.

const int rightArmMin = 1000;
const int rightArmMax = 2000;

const int leftArmMin = 1000;
const int leftArmMax = 2000;

// Movement speed
// Larger value = slower movement.

const unsigned long servoUpdateMs = 20;
const int servoStep = 1;

int rightArmPosition = 1500;
int leftArmPosition  = 1500;

int rightArmDirection = 1;
int leftArmDirection  = -1;

unsigned long lastServoUpdate = 0;

// -------------------------
// Setup
// -------------------------

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  // Audio memory
  AudioMemory(10);

  // Initialize Audio Shield
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  // Initialize Audio Shield SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("Audio Shield SD card initialization failed!");

    while (1) {
      delay(100);
    }
  }

  // Attach servos
  rightArm.attach(rightArmPin);
  leftArm.attach(leftArmPin);

  // Start both arms at center
  rightArm.writeMicroseconds(rightArmPosition);
  leftArm.writeMicroseconds(leftArmPosition);

  // Seed random number generator
  randomSeed(analogRead(A0));

  Serial.println("Ready.");
}

// -------------------------
// Main loop
// -------------------------

void loop() {
  updateButton();
  updateServos();
}

// -------------------------
// Button handling
// -------------------------

void updateButton() {
  bool reading = digitalRead(buttonPin);

  // Input changed -- reset debounce timer
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  // Input has remained stable long enough
  if ((millis() - lastDebounceTime) > debounceMs) {

    if (reading != buttonState) {
      buttonState = reading;

      // Button pressed
      if (buttonState == LOW) {

        // Ignore presses while clip is playing
        if (!playWav.isPlaying()) {
          playRandomClip();
        }
      }
    }
  }

  lastReading = reading;
}

// -------------------------
// Play random clip
// -------------------------

void playRandomClip() {
  int clip;

  // Select a different clip from the previous one
  do {
    clip = random(numClips);
  } while (clip == lastClip);

  lastClip = clip;

  Serial.print("Playing: ");
  Serial.println(wavFiles[clip]);

  playWav.play(wavFiles[clip]);

  // Reset arm motion when a new clip starts
  rightArmPosition = 1500;
  leftArmPosition = 1500;

  rightArmDirection = 1;
  leftArmDirection = -1;

  rightArm.writeMicroseconds(rightArmPosition);
  leftArm.writeMicroseconds(leftArmPosition);

  lastServoUpdate = millis();
}

// -------------------------
// Servo motion
// -------------------------

void updateServos() {

  // Arms only move while audio is playing
  if (!playWav.isPlaying()) {
    return;
  }

  unsigned long now = millis();

  if (now - lastServoUpdate < servoUpdateMs) {
    return;
  }

  lastServoUpdate = now;

  // Right arm
  rightArmPosition += rightArmDirection * servoStep;

  if (rightArmPosition >= rightArmMax) {
    rightArmPosition = rightArmMax;
    rightArmDirection = -1;
  }

  if (rightArmPosition <= rightArmMin) {
    rightArmPosition = rightArmMin;
    rightArmDirection = 1;
  }

  // Left arm
  leftArmPosition += leftArmDirection * servoStep;

  if (leftArmPosition >= leftArmMax) {
    leftArmPosition = leftArmMax;
    leftArmDirection = -1;
  }

  if (leftArmPosition <= leftArmMin) {
    leftArmPosition = leftArmMin;
    leftArmDirection = 1;
  }

  rightArm.writeMicroseconds(rightArmPosition);
  leftArm.writeMicroseconds(leftArmPosition);
}
