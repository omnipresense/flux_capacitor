#include "SevSeg.h"
SevSeg sevseg; //Instantiate a seven segment object

void setup() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  byte numDigits = 2;
  byte digitPins[] = {2, 3};
  byte segmentPins[] = {9, 8, 5, 6, 7, 11, 10, 4};
  bool resistorsOnSegments = false; // 'false' means resistors are per digit, not per segment
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  
  Serial.begin(19200);
}

const int EFFECT_SWITCH_MS = 100;
const unsigned long EFFECT_DURATION = 600;
const float EFFECT_TRIGGER_VELOCITY = 88.0;
// we want this static variable to keep its value between calls to loop
// (but have it initializes it to zero the first time) 
static unsigned long start_effect_time = 0;

// helper routine to hide this math AND allow it to be a signed number
signed long remaining_effect_time(unsigned long now) {
    return (start_effect_time+EFFECT_DURATION) - now;
}

void loop() {
  static float velocity = 0;
  static String inString = "";    // string to hold speed input from serial line
  static bool firstTimeInLoop = true;
  if (firstTimeInLoop) {
    Serial.write("UC");
    firstTimeInLoop = false;
  } 

  unsigned long now = millis();
  
  // can’t use Serial.parseFloat, it would stop the LEDs from updating, thus break it
  // so we only read a byte at a time, only when ready
  if (Serial.available() > 0) {
    int inChar = Serial.read();
    // if it's the end of a line....
    if (inChar != '\n' && inChar != '\r') {
      // As long as the incoming byte is not a newline,
      // convert the incoming byte to a char and add it to the string
      inString += (char)inChar;
    }
    else {
      // if you get a newline with a reading, convert it to a float
      if (inString.length() > 1) {  // toss away blank lines
        velocity = inString.toFloat();
        velocity = fabs(velocity);
        if (velocity < 0.1)
          velocity = 0.0;
      }
      // clear the string for new input:
      inString = "";
    }
  }

  
  
  // then use sevseg to display it
  if (velocity >= 10) {
    sevseg.setNumber(velocity, 0);
  } else { // we can show decimals, so lets do that.
    sevseg.setNumber(velocity, 1);
  }
  // if the effect is playing, we'll override this below.

  if (velocity >= EFFECT_TRIGGER_VELOCITY) {
    if (remaining_effect_time(now) <= 0)
      start_effect_time = millis();
  }

  if (remaining_effect_time(now) > 0 ) { // while doing effect
    // keep the velocity at 88 (or whatever) 
    sevseg.setNumber(EFFECT_TRIGGER_VELOCITY, 0);
    if ( (remaining_effect_time(now) / EFFECT_SWITCH_MS ) % 2) {  // toggle which LEDS are on
      digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
      digitalWrite(13, LOW);   // turn the LED off (LOW is the voltage level)
    } else {
      digitalWrite(12, LOW);   // turn the LED off (LOW is the voltage level)
      digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
  } else {
    digitalWrite(12, LOW);   // turn the LED off
    digitalWrite(13, LOW);   // turn the LED off
  }

  sevseg.refreshDisplay();

}
