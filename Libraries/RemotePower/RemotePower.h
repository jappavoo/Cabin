#ifndef __REMOTE_POWER_h__
#define __REMOTE_POWER_h__

class RemotePower {
 public:
  enum State { OFF=0, ON=1 };
 private:
  char _onPin, _offPin;
  enum State _state;
  const char *_name;
 public:
 RemotePower(char onpin, char offpin, const char *n) : 
  _onPin(onpin), _offPin(offpin), _state(OFF), _name(n) {}
  
  inline void On() {
    if (Globals.verbose()) Serial.println("Remote Power: ON");
    //  digitalWrite(REMOTE_POWER_ON_PIN,LOW);
    //  delay(1000);
    digitalWrite(_onPin,HIGH);
    delay(1000);
    digitalWrite(_onPin,LOW);
    _state = ON;
    //  delay(3000);
  }

  inline void Off() {
    if (Globals.verbose()) Serial.println("Remote Power: OFF");
    //  digitalWrite(REMOTE_POWER_OFF_PIN,LOW);
    //  delay(1000);
    digitalWrite(_offPin,HIGH);
    delay(1000);
    digitalWrite(_offPin,LOW);
    _state = OFF;
    //  delay(3000);
  }

  inline void printStatus(Stream &s) {
    s.print("Remote Power(");
    s.print(_onPin, DEC); s.print(","); s.print(_offPin, DEC);
    s.print(") : ");
    s.print(_name);
    if (_state == ON) s.print(" - ON");
    else s.print(" - OFF");
  }
  
  inline void printShortStatus(Stream &s) {
    s.print(_name);
    if (_state == ON) s.print(": ON");
    else s.print(": OFF");
  }
  
  inline void setup() {
    pinMode(_onPin, OUTPUT);
    digitalWrite(_onPin,LOW);
    pinMode(_offPin, OUTPUT);
    digitalWrite(_offPin,LOW);
    Off();  
    printStatus(Serial);
  }

  // no loop action 
  inline void loopAction() { return; }

  // does not trigger the alarm
  inline bool isAlarm() { return FALSE; }
};

#endif
