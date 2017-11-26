/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed.
#include "config.h"

#define MAX_FEEDS 7

/****************************** Feeds ***************************************/

// Setup a feed called 'touched' for publishing our touch state.
AdafruitIO_Feed *touched = io.feed("pdxtouched");

// TODO: Watch throttling

class Wifi {
public:

  Wifi() : paused(false) {
    for (int i = 0; i < MAX_FEEDS; i++) {
      feeds[i] = NULL;
    }
  }

  int addFeed(const char *n) {
    return addFeed(n, 0);
  }
  int addFeed(const char *n, AdafruitIODataCallbackType cb) {
    int nextFeed = -1;
    for (int i = 0; i < MAX_FEEDS; i++) {
      if (feeds[i] == NULL) {
        nextFeed = i;
        break;              
      }
    }

    if (nextFeed == -1) {
      // Fail silently... meeehhh... probably won't matter?
      return -1;
    }

    AdafruitIO_Feed *feed = io.feed(n);
    if (cb) {
      feed->onMessage(cb);
    }
   
    feeds[nextFeed] = feed;
    return nextFeed;
  }
    
  void begin() {
    // Connect to WiFi access point.
    Serial.println(); Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);
    io.connect();


    // wait for a connection
    while(io.status() < AIO_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  
    Serial.println("WiFi connected");
    Serial.println(io.statusText());
  }

  void update() {
    if (paused) {
      //io.run(0);
      return;
    }
    io.run();
  }

  bool isPaused() {
    return paused;
  }

  void pause() {
    paused = true;
    Serial.println("Pausing wifi");
  }
  void unpause() {
    paused = false;
    Serial.println("Unpausing wifi");
  }

  void sendTouch(const int num, bool isTouched) {
    if (num >= MAX_FEEDS || num < 0 || feeds[num] == NULL) {
      Serial.println("Couldn't send touch; unrecognized feed number");
      return;            
    }

    AdafruitIO_Feed* feed = feeds[num];
    
    if (isTouched) {
      feed->save(1);
    } else {
      feed->save(0);
    }
  }

private:
  AdafruitIO_Feed *feeds[MAX_FEEDS];
  bool paused;
};

