/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed.
#include "config.h"

/****************************** Feeds ***************************************/

// Setup a feed called 'touched' for publishing our touch state.
AdafruitIO_Feed *touched = io.feed("pdxtouched");

// Setup a feed called 'motherTouched' for subscribing to whether remote device is touched.
AdafruitIO_Feed *motherSub = io.feed("mothertouched");

// TODO: Watch throttling


void wifi_begin() {
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  io.connect();

  motherSub->onMessage(handleMotherTouch);

    // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println(io.statusText());
}

void wifi_update() {

  io.run();
}

void wifi_send(bool isTouched) {
  if (isTouched) {
    touched->save(1);
  } else {
    touched->save(0);
  }
}

void handleMotherTouch(AdafruitIO_Data *data) {

  Serial.print(F("Mothersub"));
  Serial.print(F("received <- "));
  
  motherVal = data->toInt();
  Serial.println(motherVal);
}

