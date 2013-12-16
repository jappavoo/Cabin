#ifndef __REED_SWITCH_h__
#define __REED_SWITCH_h__

class ReedSwitch {
 public:
  enum State { CLOSED=LOW, OPEN=HIGH };

 private:
  enum State _cur;
  enum State _last;
  char _cnt;
  char _pin;
  const char *_name;

 public:
  ReedSwitch(char pin, const char *n) : 
    _cur(CLOSED), _last(CLOSED), _cnt(0), 
    _pin(pin), _name(n) {}

  inline void printStatus(Stream &s) {
    s.print(_name); 
    s.print(" ("); s.print(_pin, DEC); s.print(")"); 
    if (_cur == OPEN) Serial.print(": OPEN ");
    else Serial.print(": CLOSED ");
    s.print(" cnt="); 
    s.print(_cnt, DEC);
  }
  
  inline void printShortStatus(Stream &s) {
    if (_cnt!=0) {
      s.print(_name);
      s.print(":");
      s.print(_cnt, DEC);
    }
  }

  inline void setup() {
    pinMode(_pin, INPUT);
    digitalWrite(_pin, HIGH);
    _cur = (enum State)digitalRead(_pin);
    if (_cur == OPEN) _cnt++;
    _last = _cur;
    printStatus(Serial);
  }

  inline void loopAction() {
    _cur = (enum State)digitalRead(_pin);
    if (_cur != _last) {
      if (Globals.verbose) {
         printStatus(Serial);
         Serial.println();
      }
      if (_cur == OPEN) _cnt++;
      _last = _cur;
    }
  }

  inline bool isAlarm() { 
    if (_cnt != 0) return TRUE; else return FALSE; 
  }
   
};

#endif

