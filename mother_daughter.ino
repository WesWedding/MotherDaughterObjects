/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <CapacitiveSensor.h>
#include <TweenDuino.h>
#include <Adafruit_NeoPixel.h>

#define TOUCH_THRESHOLD 300

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "CorvaleDeux"
#define WLAN_PASS       "PaperbackPaup3r"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Razoras"
#define AIO_KEY         "c5c8a51fbbd44ed79b31c7fda5b33a29"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'touched' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish touched = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pdxtouched");

// Setup a feed called 'altouched' for subscribing to whether alabama device is touched.
Adafruit_MQTT_Subscribe altouched = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/altouched");

// Watch throttling.
Adafruit_MQTT_Subscribe throttle = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

// Name some pins.
const int redLED = 0; // On-board red LED.
const int LEDStripPin = 2; // My pixel strip's data line is on pin 9.
const int capSensorPin = 4;
const int capReadPin = 5;

bool shouldReadTouch;

CapacitiveSensor   cs_2_3 = CapacitiveSensor(capReadPin,capSensorPin);        // 10M resistor between pins 2 & 3, pin 2 is sensor pin, add a wire and or foil if desired
Adafruit_NeoPixel stripLEDs = Adafruit_NeoPixel(7, LEDStripPin, NEO_GRBW);

// Start dim (0).
float brightness = 0.0;
uint8_t alTouched = 0;

TweenDuino::Timeline timeline;

void setup() {
  Serial.begin(115200);
  stripLEDs.begin();

  cs_2_3.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  adil(timeline);

  stripLEDs.show();
  shouldReadTouch = false;

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&altouched);
}

void loop() {

  long loopStart = millis();
  timeline.update(loopStart);

  long capReading = 0;
  if (shouldReadTouch) {
    capReading = cs_2_3.capacitiveSensor(30);
    Serial.print("Touch value: "); Serial.println(capReading);
  }

  if (timeline.isComplete()) {
    brightness = 0.0;
    shouldReadTouch = true;
    sendTouched(0);
  }

  // Override brightness if touching
  if (capReading > TOUCH_THRESHOLD) {
    timeline.restartFrom(millis());
    shouldReadTouch = false;
  }

  // 3 colors: Red, Green, Blue whose values range from 0-255.
  const uint32_t stripColor = stripLEDs.Color(0, 0, 0, brightness); // Colors are off, white LED is pure white.
  
  setStripColors(stripLEDs, stripColor);

  // This sends the updated pixel color to the hardware.
  stripLEDs.show();
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  if (subscription = mqtt.readSubscription(10)) {
    if (subscription == &altouched) {
      Serial.print(F("Got: "));
      alTouched = *altouched.lastread;
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\cap reading "));
  Serial.print(capReading);
  Serial.print("...");

  // Override brightness if touching
  if (alTouched == '1' || capReading > TOUCH_THRESHOLD) {
    Serial.print("Sending touch");
    sendTouched(1);
  }
  
  Serial.print("alTouched: ");
  Serial.println(alTouched);

  Adafruit_MQTT_Subscribe *weThrottled;
  if (weThrottled = mqtt.readSubscription(10)) {
    if (weThrottled == &throttle) {
      Serial.print(F("Throttled: "));
      alTouched = *throttle.lastread;
    }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

void sendTouched(uint8_t val) {
  if (! touched.publish(val)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void setStripColors(Adafruit_NeoPixel &strip, uint32_t color) {
  const int numPixels = strip.numPixels();

  for (int i=0;i<numPixels;i++) {
    strip.setPixelColor(i, color);
  }
}

void adil(TweenDuino::Timeline &timeline) {
  // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));
      // Dash
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,150UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

  // Space
  timeline.add(*TweenDuino::Tween::to(brightness,150UL, 0.0));


    // Dash
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,150UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

    // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));
  
  // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

    // Space
  timeline.add(*TweenDuino::Tween::to(brightness,150UL, 0.0));

    // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

    // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

      // Space
  timeline.add(*TweenDuino::Tween::to(brightness,150UL, 0.0));

      // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

      // Dash
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,150UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

      // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));

    // Dot
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 255.0));
  timeline.add(*TweenDuino::Tween::to(brightness,50UL, 0.0));
}

