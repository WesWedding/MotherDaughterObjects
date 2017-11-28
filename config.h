#ifndef _MOTHER_DAUGHTER_CONFIG_H

#define _MOTHER_DAUGHTER_CONFIG_H

#define TOUCH_THRESHOLD 300

// Uncomment this if you are building for the mother pod.
//#define IS_MOTHER_POD

/************************* Adafruit.io Setup *********************************/

#define AIO_USERNAME  "Adafruit IO Username"
#define AIO_KEY       "Adafruit IO Key"


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Wireless Network Name"
#define WLAN_PASS       "Wireless Network Password"
#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, WLAN_SSID, WLAN_PASS);

/**************************** I/O Pin Setups *********************************/

#define ONBOARD_LED    0
#define LED_PIN        2
#define PIXEL_COUNT    7
#define CAP_SEND_PIN 5
#define CAP_READ_PIN   4



#endif

