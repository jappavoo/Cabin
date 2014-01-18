#ifndef __LED_H__
class Led {
  const char *_name;
  uint8_t _pin;
  uintptr_t _rate;
  uint8_t _state;
  unsigned long _last;
 public:
 Led(uint8_t pin, const char *name) : _name(name),  _pin(pin),  
    _rate(OFF), _state(LOW), _last(0) {}

  enum { OFF=0, SLOW=500, MEDIUM=250, FAST=50, ON=0xFF };

  boolean isOff() { return _rate == OFF; }
  boolean isOn() { return !isOff(); }

  void flash(uintptr_t rate) { 
    if (rate == OFF) { digitalWrite(_pin, LOW); _state = LOW; }
    else if (rate == ON) { digitalWrite(_pin, HIGH); _state = HIGH; }
    _rate = rate;
  } 
  void On() { flash(ON); }
  void Off() { flash(OFF); }

  void printShortStatus(Stream &s) {
  };

  void printStatus(Stream &s) {
    s.print("LED:");
    s.print(_name);
    s.print("("); s.print(_pin, DEC); s.print("),");
    s.print(_rate, DEC);
    if (_rate == OFF) {
      s.print("=OFF");
    } else if (_rate == ON ) {
      s.print("=ON"); 
    } else if (_rate == SLOW ) {
      s.print("=SLOW"); 
    } else if (_rate == MEDIUM ) {
      s.print("=MEDIUM"); 
    } else if (_rate == FAST ) {
      s.print("=FAST"); 
    }
  }

  void setup() {
    pinMode(_pin,OUTPUT);
    for (int i=0; i<10; i++) {
      digitalWrite(_pin, HIGH);
      delay(50);
      digitalWrite(_pin, LOW);
      delay(50);
    }
    Off();
    printStatus(Serial);
  }

  void loopAction() {
    if (_rate == OFF || _rate == ON ) return;
    if (millis() - _last >= _rate) {
      if (_state == LOW) { digitalWrite(_pin, HIGH); _state = HIGH; }
      else { digitalWrite(_pin, LOW); _state = LOW; }
      _last = millis();
    }
  }
  
};
#endif
