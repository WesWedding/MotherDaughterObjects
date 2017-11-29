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
#include "wifi.h"

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed.
#include "config.h"

/*************************** Sketch Code ************************************/
int remoteVal = 0;
bool remoteWasTouched = false;

Adafruit_NeoPixel stripLEDs = Adafruit_NeoPixel(PIXEL_COUNT, LED_PIN, NEO_RGB);

// 10M resistor between "read" pin and read pin, add a capacative surface/foil to read pin.
Touch touch(CAP_SEND_PIN, CAP_READ_PIN);
Wifi wifi;

// Start dim (0).
float brightness = 0.0;

int myFeed = 0;

TweenDuino::Timeline timeline;

void setup() {
  Serial.begin(115200);
  stripLEDs.begin();

#ifdef IS_MOTHER_POD
  myFeed = wifi.addFeed("mothertouched");
  wifi.addFeed("pdxtouched", handlePdxTouch);
  Serial.println("Added pdx feed.");
#else  
  myFeed = wifi.addFeed("pdxtouched");
  //myFeed = wifi.addFeed("altouched");
  wifi.addFeed("mothertouched", handleMotherTouch);
  Serial.println("Added mothertouched feed.");
#endif
  wifi.begin();
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
  wifi.update();

  // Turn wifi back on if the animations aren't playing.
  if (wifi.isPaused() && !timeline.isActive()) {
    Serial.println("Unpausing wifi");
    wifi.unpause();
  }

  // Sensing touch makes things halt briefly, leading to jerky animations.
  // Don't sense touch if we're playing our animation or planning to do so.
  bool shouldReadTouch = !timeline.isActive() || remoteVal > 0;
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

  if (remoteVal > 0) {
    //Serial.println("Mother val is good.");
    if (timeline.isActive()) {
      //Serial.println("Not going to start lights because they're already going.");
    }
  }
  if (remoteWasTouched && !timeline.isActive()) {
    //Serial.println("Show LEDS because motherVal is good.");
    timeline.restartFrom(millis());
    wifi.pause();
    touch.ignore();

    // Reset motherVal to zero because we might miss packets telling us it has been set to 0.
    remoteVal = 0;

    // We've "consumed" the latest touch.
    remoteWasTouched = false;
  }

  // 3 colors: Red, Green, Blue whose values range from 0-255.
  const uint32_t stripColor = stripLEDs.Color(255 * brightness, 100 * brightness, 255 * brightness, 0); // Colors are off, white LED is pure white.
  
  setStripColors(stripLEDs, stripColor);

  // This sends the updated pixel color to the hardware.
  stripLEDs.show();
}

void handlePdxTouch(AdafruitIO_Data *data) {
  Serial.print(F("Pdx feed"));
  Serial.print(F("received <- "));
  
  remoteVal = data->toInt();
  remoteWasTouched = true;
  Serial.println(remoteVal);
}

void handleMotherTouch(AdafruitIO_Data *data) {

  Serial.print(F("Mother feed"));
  Serial.print(F("received <- "));
  
  remoteVal = data->toInt();
  remoteWasTouched = true;
  Serial.println(remoteVal);
}

void onTouchChanged(bool *isTouched) {
  Serial.print("Sending touch state: " ); Serial.println(*isTouched);
  wifi.sendTouch(myFeed, *isTouched);
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


