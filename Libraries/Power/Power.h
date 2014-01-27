#ifndef __VOLTAGE_h__
#define __VOLTAGE_h__

class Power {
 public:
  enum STATE { ON=LOW, OFF=HIGH };
 private:
  const char *_name;
   enum STATE _state;
  char _pin;
 public:
 Power(char pin, const char *name) : _name(name), _state(OFF), _pin(pin) {}
  inline enum STATE state() { return _state; }
  inline boolean printShortStatus(Stream &s) {
    s.print(_name);
    if (_state==ON) s.print(": ON");
    else s.print(": OFF");
    return TRUE;
  }
  inline void printStatus(Stream &s) {
    printShortStatus(s);
    s.print(" (");
    s.print(_pin, DEC);
    s.print("):");
    s.print(" (");
    s.print(_state, DEC);
    s.print(")");
  }
  inline void setup() {
    pinMode(_pin, INPUT);
    loopAction();
    printStatus(Serial);
  }
  inline void loopAction() {
    _state = (enum STATE) digitalRead(_pin);
  }
  inline boolean isAlarm() {
    if (_state==HIGH) return TRUE;
    else return FALSE;
  }
  inline void resetAlarm() {}
};
#endif
