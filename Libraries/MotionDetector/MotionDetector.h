#ifndef __MOTION_DETECTOR_h__
#define __MOTION_DETECTOR_h__

#define MOTION_DEFAULT_ALARM_THRESHOLD 5
#define MOTION_TEST_ALARM_THRESHOLD 100
class MotionDetector {
private:
  enum { PIRcalibrationTime=30 };           
  bool  PIRMotion;
  char _count;
  char _pin;
  char _alarmThreshold;
public:  
 MotionDetector(char pin, char thrshld = MOTION_DEFAULT_ALARM_THRESHOLD) : 
  PIRMotion(FALSE), _count(0), _pin(pin), _alarmThreshold(thrshld) {
  }

  inline void alarmThreshold(char v) { _alarmThreshold = v; } 
  inline char count() { return _count; }
  inline void count(char v) { _count = v; }

  void printStatus(Stream &s) {
    s.print(" Motion: "); s.print(_count, DEC);
    s.print("/"); s.print(_alarmThreshold, DEC);
  }

  void printShortStatus(Stream &s) {
    if (_count) {
      s.print(" Motion: "); s.print(_count, DEC);
    }
  }

  void setup() {
    pinMode(_pin, INPUT); 
    digitalWrite(_pin, LOW);
    //give the sensor some time to calibrate
    Serial.print("calibrating PIR sensor ");
    for(int i = 0; i < PIRcalibrationTime; i++){
      Serial.print(".");
      delay(1000);
    }
    Serial.println(" done");
    Serial.println("PIR SENSOR ACTIVE");
    printStatus(Serial);
  }

  inline void loopAction() {
    if (digitalRead(_pin) ==  HIGH)  {
      if (PIRMotion==false) {
        _count++;
        if (Globals.verbose) {
          Serial.print("  PIR START:");
	  printStatus(Serial);
	  Serial.println();
        }
        PIRMotion=true;
      }
    } else {
      if (PIRMotion==true) {
	if (Globals.verbose) {
	  Serial.println("  PIR ENDED.");
	  printStatus(Serial);
	  Serial.println();
	}
	PIRMotion=false;
      }
    }
  }
  
  inline boolean isAlarm() {
    if (_count > _alarmThreshold) return true;
    else return false;
  }

};

#endif
