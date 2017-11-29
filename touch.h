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

  // For reading smoothings
  long readings[3];
  int readIndex;
  long total;
  long average;
  int numReadings;


public:
  Touch(int sendPin, int receivePin): 
      capSensor(CapacitiveSensor(sendPin, receivePin)),
      previousIsTouchedVal(false),
      isTouched(false),
      shouldReadTouch(false),
      readIndex(0),
      total(0),
      average(0),
      numReadings(3),
      cb(0) {
        capSensor.set_CS_Timeout_Millis(2000);
        for (int thisReading = 0; thisReading < numReadings; thisReading++) {
          readings[thisReading] = 0;
        }
      }

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
      //Serial.println("Not reading touch.");
    }

    isTouched = averageTouchesAboveThreshold(capReading);
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

  bool averageTouchesAboveThreshold(long capReading ) {
    total = total - readings[readIndex];
    readings[readIndex] = capReading;
    total = total + readings[readIndex];

    readIndex = readIndex + 1;
    // if we're at the end of the array...
    if (readIndex >= numReadings) {
      // ...wrap around to the beginning:
      readIndex = 0;
    }
    average = total / numReadings;
    Serial.print("Ave: "); Serial.println(average);
    if (average > TOUCH_THRESHOLD) {
      return true;
    }

    return false;
  }
};

