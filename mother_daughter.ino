/***************************************************
  Mother/Daughter Pod Sourcecode

  Written by Weston Wedding as a contribution to 
  Tabitha Darrah's 2017 senior thesis.

  Based at least in part on example code
  written by Tony DiCola for Adafruit Industries.
 ****************************************************/
#include <TweenDuino.h>
#include <Adafruit_NeoPixel.h>
#include "touch.h"

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed.
#include "config.h"

/*************************** Sketch Code ************************************/
int motherVal = 0;

Adafruit_NeoPixel stripLEDs = Adafruit_NeoPixel(PIXEL_COUNT, LED_PIN, NEO_GRBW);

// 10M resistor between "read" pin and read pin, add a capacative surface/foil to read pin.
Touch touch(CAP_SEND_PIN, CAP_READ_PIN);

// Start dim (0).
float brightness = 0.0;

TweenDuino::Timeline timeline;

void setup() {
  Serial.begin(115200);
  stripLEDs.begin();
  wifi_begin();
  touch.setOnTouchChanged(onTouchChanged);

  addTweensTo(timeline);

  timeline.begin(millis());

  stripLEDs.clear();
  stripLEDs.show();
  
  Serial.println(F("Mother/Daughter MQTT Objects demo"));

  pinMode(ONBOARD_LED, OUTPUT);
}

void loop() {
  long loopStart = millis();
  timeline.update(loopStart);
  touch.update();
  wifi_update();

  // Sensing touch makes things halt briefly, leading to jerky animations.
  // Don't sense touch if we're playing our animation or planning to do so.
  bool shouldReadTouch = !timeline.isActive() || motherVal > 0;
  if (shouldReadTouch) {
    touch.watch();
  } else {
    touch.ignore();
  }

  if (timeline.isComplete()) {
    brightness = 0.0;
  }

  // Override brightness if remote is being touched.
  //Serial.print("Remote value: "); Serial.println(motherVal);
  //Serial.print("TL is active: "); Serial.println(timeline.isActive());

  if (motherVal > 0) {
    Serial.println("Mother val is good.");
    if (timeline.isActive()) {
      Serial.println("Not going to start lights because they're already going.");
    }
  }
  if (motherVal > 0 && !timeline.isActive()) {
    Serial.println("Show LEDS because motherVal is good.");
    timeline.restartFrom(millis());
    touch.ignore();
  }

  // 3 colors: Red, Green, Blue whose values range from 0-255.
  const uint32_t stripColor = stripLEDs.Color(255 * brightness , 100, 255 * brightness, 0); // Colors are off, white LED is pure white.
  
  setStripColors(stripLEDs, stripColor);

  // This sends the updated pixel color to the hardware.
  stripLEDs.show();
}

void onTouchChanged(bool *isTouched) {
  Serial.print("Sending touch state: " ); Serial.println(*isTouched);
  wifi_send(*isTouched);
}

void setStripColors(Adafruit_NeoPixel &strip, uint32_t color) {
  const int numPixels = strip.numPixels();

  for (int i=0;i<numPixels;i++) {
    strip.setPixelColor(i, color);
  }
}

// Make a pattern.
void addTweensTo(TweenDuino::Timeline &timeline) {

  timeline.addTo(brightness, 1.0, 1000);
  timeline.addTo(brightness, 0.5, 1000);
  timeline.addTo(brightness, 1.0, 1000);
  timeline.addTo(brightness, 0.0, 1000);
}


