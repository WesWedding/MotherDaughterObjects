#include "config.h"
#include <CapacitiveSensor.h>

typedef void (*TouchChangedCallback)(bool *data);

class Touch {
  private:
  CapacitiveSensor capSensor;
  bool previousIsTouchedVal;
  bool isTouched;
  bool shouldReadTouch;
  TouchChangedCallback cb;


public:
  Touch(int sendPin, int receivePin): 
      capSensor(CapacitiveSensor(sendPin, receivePin)),
      previousIsTouchedVal(false),
      isTouched(false),
      shouldReadTouch(false),
      cb(0) {}

  void setOnTouchChanged(TouchChangedCallback touched) {
    cb = touched;    
  }
  
  void update() {
    previousIsTouchedVal = isTouched;

    long capReading = 0;
    if (shouldReadTouch) {
      capReading = capSensor.capacitiveSensor(30);
      if (capReading == -2) {
        Serial.println("Capacative sensor timed out.");
      } else {
        Serial.print("Touch value: "); Serial.println(capReading);
      }
    } else {
      Serial.println("Not reading touch.");
    }

    isTouched = capReading > TOUCH_THRESHOLD;
    if (isTouched) {
      digitalWrite(ONBOARD_LED, LOW);
    } else {
      digitalWrite(ONBOARD_LED, HIGH);
    }

    bool touchChanged = isTouched != previousIsTouchedVal;
    if (touchChanged) {
      if (cb) {
        cb(&isTouched);
      }
    }
  }

  void ignore() {
    shouldReadTouch = false;
  }
  void watch() {
    shouldReadTouch = true;
  }
};

