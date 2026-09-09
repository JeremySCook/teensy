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

const char* wavFiles[17] = {
  "COWBOY01.WAV",
  "COWBOY02.WAV",
  "COWBOY03.WAV",
  "COWBOY04.WAV",
  "COWBOY05.WAV",
  "COWBOY06.WAV",
  "COWBOY07.WAV",
  "COWBOY08.WAV",
  "COWBOY09.WAV",
  "COWBOY10.WAV",
  "COWBOY11.WAV",
  "COWBOY12.WAV",
  "COWBOY13.WAV",
  "COWBOY14.WAV",
  "COWBOY15.WAV",
  "COWBOY16.WAV",
  "COWBOY17.WAV"
};

const int numClips = 17;
int lastClip = -1;

// -------------------------
// Servos
// -------------------------

Servo rightArm;
Servo leftArm;

const int rightArmPin = 34;
const int leftArmPin  = 35;

// Servo endpoints
// Adjust these to suit the linkage.

const int rightArmMin = 800;
const int rightArmMax = 1200;  // Max dictates how low right arm goes

const int leftArmMin = 1000;
const int leftArmMax = 1200;   // Max dictates how high left arm goes

// -------------------------
// Servo movement
// -------------------------

// Servo update interval.
// Both servos are updated every 20 ms.

const unsigned long servoUpdateMs = 20;

// Independent servo speeds.
// Larger value = faster movement.

const int rightArmStep = 5;
const int leftArmStep  = 2;

int rightArmPosition = rightArmMax;
int leftArmPosition  = leftArmMin;

int rightArmDirection = 1;
int leftArmDirection = -1;

unsigned long lastServoUpdate = 0;

// -------------------------
// Automatic servo motion
// -------------------------

// Time between automatic movements.
// A random value between these two limits is selected.

const unsigned long autoMotionMinMs = 20000;  // 30 seconds
const unsigned long autoMotionMaxMs = 40000;  // 60 seconds

// How long each arm moves during automatic motion.
// These can be adjusted independently.

const unsigned long rightArmMotionDurationMs = 12000;   // 8 seconds
const unsigned long leftArmMotionDurationMs  = 8000;  // 12 seconds

bool autoMotionActive = false;

bool rightArmMoving = false;
bool leftArmMoving  = false;

unsigned long rightArmMotionStartTime = 0;
unsigned long leftArmMotionStartTime  = 0;

unsigned long nextAutoMotionTime = 0;

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
  sgtl5000_1.volume(0.7);

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

  // Start both arms at their defined starting positions
  rightArm.writeMicroseconds(rightArmPosition);
  leftArm.writeMicroseconds(leftArmPosition);

  // Seed random number generator
  randomSeed(analogRead(A0));

  // Schedule first automatic movement
  scheduleNextAutoMotion();

  Serial.println("Ready.");
}

// -------------------------
// Main loop
// -------------------------

void loop() {
  updateButton();
  updateAutoMotion();
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

  // Reset arm motion to defined starting positions
  rightArmPosition = rightArmMax;
  leftArmPosition = leftArmMin;

  rightArmDirection = 1;
  leftArmDirection = -1;

  rightArm.writeMicroseconds(rightArmPosition);
  leftArm.writeMicroseconds(leftArmPosition);

  lastServoUpdate = millis();

  // Cancel any automatic motion currently in progress
  autoMotionActive = false;
  rightArmMoving = false;
  leftArmMoving = false;

  // Schedule the next automatic movement
  scheduleNextAutoMotion();
}

// -------------------------
// Schedule next automatic movement
// -------------------------

void scheduleNextAutoMotion() {

  unsigned long interval = random(
    autoMotionMinMs,
    autoMotionMaxMs + 1
  );

  nextAutoMotionTime = millis() + interval;

  Serial.print("Next automatic motion in ");
  Serial.print(interval / 1000);
  Serial.println(" seconds.");
}

// -------------------------
// Automatic motion handling
// -------------------------

void updateAutoMotion() {

  unsigned long now = millis();

  // -------------------------
  // Automatic motion currently active
  // -------------------------

  if (autoMotionActive) {

    // Check right arm independently
    if (rightArmMoving &&
        now - rightArmMotionStartTime >= rightArmMotionDurationMs) {

      rightArmMoving = false;

      Serial.println("Right arm automatic motion finished.");
    }

    // Check left arm independently
    if (leftArmMoving &&
        now - leftArmMotionStartTime >= leftArmMotionDurationMs) {

      leftArmMoving = false;

      Serial.println("Left arm automatic motion finished.");
    }

    // Both arms have finished
    if (!rightArmMoving && !leftArmMoving) {

      autoMotionActive = false;

      Serial.println("Automatic motion finished.");

      scheduleNextAutoMotion();
    }

    return;
  }

  // -------------------------
  // Don't start automatic motion
  // while audio is playing
  // -------------------------

  if (playWav.isPlaying()) {
    return;
  }

  // -------------------------
  // Time for automatic motion
  // -------------------------

  if ((long)(now - nextAutoMotionTime) >= 0) {

    Serial.println("Automatic motion started.");

    autoMotionActive = true;

    // Start right arm
    rightArmMoving = true;
    rightArmMotionStartTime = now;

    // Start left arm
    leftArmMoving = true;
    leftArmMotionStartTime = now;

    // Start arms at their defined starting positions
    rightArmPosition = rightArmMax;
    leftArmPosition = leftArmMin;

    rightArmDirection = 1;
    leftArmDirection = -1;

    rightArm.writeMicroseconds(rightArmPosition);
    leftArm.writeMicroseconds(leftArmPosition);

    lastServoUpdate = now;
  }
}

// -------------------------
// Servo motion
// -------------------------

void updateServos() {

  // Nothing to do if audio isn't playing
  // and neither automatic arm is moving.

  if (!playWav.isPlaying() &&
      !rightArmMoving &&
      !leftArmMoving) {
    return;
  }

  unsigned long now = millis();

  if (now - lastServoUpdate < servoUpdateMs) {
    return;
  }

  lastServoUpdate = now;

  // -------------------------
  // Right arm
  // -------------------------

  // During audio, right arm moves continuously.
  // During automatic motion, it moves only while enabled.

  if (playWav.isPlaying() || rightArmMoving) {

    rightArmPosition += rightArmDirection * rightArmStep;

    if (rightArmPosition >= rightArmMax) {
      rightArmPosition = rightArmMax;
      rightArmDirection = -1;
    }

    if (rightArmPosition <= rightArmMin) {
      rightArmPosition = rightArmMin;
      rightArmDirection = 1;
    }

    rightArm.writeMicroseconds(rightArmPosition);
  }

  // -------------------------
  // Left arm
  // -------------------------

  // During audio, left arm moves continuously.
  // During automatic motion, it moves only while enabled.

  if (playWav.isPlaying() || leftArmMoving) {

    leftArmPosition += leftArmDirection * leftArmStep;

    if (leftArmPosition >= leftArmMax) {
      leftArmPosition = leftArmMax;
      leftArmDirection = -1;
    }

    if (leftArmPosition <= leftArmMin) {
      leftArmPosition = leftArmMin;
      leftArmDirection = 1;
    }

    leftArm.writeMicroseconds(leftArmPosition);
  }
}
