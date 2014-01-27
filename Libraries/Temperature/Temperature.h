#ifndef __TEMPERATURE_h__
#define __TEMPERATURE_h__

#define TEMP_CHECK_DELAY_MS 1000

class TempBus : public DallasTemperature {
  OneWire _oneWire;
  unsigned long _last;
  char _numDev; 
public:
  TempBus(char pin) : DallasTemperature(&_oneWire), 
		      _oneWire(pin), _last(0), _numDev(0) {}

  void printTemperature(DeviceAddress deviceAddress, Stream &s) {
    float tempC = getTempC(deviceAddress);
    s.print(tempC);
    s.print("C/");
    s.print(toFahrenheit(tempC));
    s.print("F");
  }

  void printAlarms(DeviceAddress deviceAddress, Stream &s)
  {
    char temp;
    temp = getHighAlarmTemp(deviceAddress);
    s.print("High Alarm:");
    s.print(temp, DEC);
    s.print("C/");
    s.print(toFahrenheit(temp));
    s.print("F Low Alarm:");
    temp = getLowAlarmTemp(deviceAddress);
    s.print(temp, DEC);
    s.print("C/");
    s.print(toFahrenheit(temp));
    s.print("F");
  }

  // function to print a device address
  void printAddress(DeviceAddress deviceAddress, Stream &s) {
    for (uint8_t i = 0; i < 8; i++)
      {
	// zero pad the address if necessary
	if (deviceAddress[i] < 16) s.print("0");
	s.print(deviceAddress[i], HEX);
      }
  }

  // function to print a device's resolution
  void printResolution(DeviceAddress deviceAddress, Stream &s) {
    s.print("Resolution:");
    s.print(getResolution(deviceAddress), DEC);
  }

  void printGlobalPowerMode(Stream &s) {
    if (isParasitePowerMode()) s.print("PARASITIC");
    else s.print("POWERED");
  }

  void printPowerSuppy(DeviceAddress deviceAddress, Stream &s)  {
    if (readPowerSupply(deviceAddress)) s.print("POWERED");
    else s.print("PARASITIC");
  }

  void printDetails(DeviceAddress addr, Stream &s) {
    printAddress(addr, s);
    s.print(",");
    printResolution(addr,s);
    s.print(",");
    printPowerSuppy(addr, s);
    s.print(",");
    printAlarms(addr, s);
    s.print(",");
    printTemperature(addr, s);
    s.println();
  }

  void setup() {
    DeviceAddress addr;

    begin();
    //    Serial.print("TempBus: Global Power Mode: ");
    printGlobalPowerMode(Serial);
    Serial.println();
    //    Serial.println("TempBus: Searching for Devices:");
    _numDev = getDeviceCount();
    //    Serial.print(" Found ");
    //    Serial.print(_numDev, DEC);
    //    Serial.print(" devices. ");
    if (_numDev) {
      //      Serial.println("Requesting Temperatures ...");
      requestTemperatures();
      //      Serial.println("Probing Thermometers:");
      for (char i=0; i<_numDev; i++)  {
	//	Serial.print(i, DEC);
	//	Serial.print(": ");
	if (getAddress(addr, i)) {
	  printDetails(addr, Serial);
	} else {
	  Serial.println(" Error getting address!");
	}
      }
      // set non-blocking mode 
      setWaitForConversion(FALSE);
      // initialize first request for temperatures
      requestTemperatures();
      _last = millis();
    } else {
      Serial.println();
    }
  }

  boolean loopAction() {
    if ((millis() - _last) > TEMP_CHECK_DELAY_MS) {
      requestTemperatures();
      _last = millis();
      return TRUE;
    }
    return FALSE;
  }

};

class DS18B20 {
  DeviceAddress _addr;
  const char *_name;
  float _temp;
  boolean _alarm;
public:
  DS18B20(uint8_t *addr, const char *name) : 
    _name(name), _temp(0.0), _alarm(FALSE) {
    _addr[0] = addr[0]; _addr[1]=addr[1]; _addr[2]=addr[2]; _addr[3]=addr[3];
    _addr[4] = addr[4]; _addr[5]=addr[5]; _addr[6]=addr[6]; _addr[7]=addr[7];
  }
    
  void printTemperature(Stream &s, TempBus bus) {
    s.print(_temp);
    s.print("C/");
    s.print(bus.toFahrenheit(_temp));
    s.print("F");
  }

  void printStatus(Stream &s, TempBus &bus) {
    s.print(_name);
    s.print(": ");
    bus.printAddress(_addr, s);
    s.print(",");
    bus.printResolution(_addr,s);
    s.print(",");
    bus.printPowerSuppy(_addr, s);
    s.print(",");
    bus.printAlarms(_addr, s);
    s.print(",");
    printTemperature(s, bus);
  }

  boolean printShortStatus(Stream &s, TempBus &bus) {
    s.print(_name);
    if (_alarm == TRUE) s.print(":*ALARM*");
    else s.print(":");
    printTemperature(s,bus);
    return TRUE;
  }

  bool addrMatch(uint8_t *addr) {
    return (_addr[0] == addr[0] && _addr[1] == addr[1] && 
	    _addr[2] == addr[2] && _addr[3] == addr[3] &&
	    _addr[4] == addr[4] && _addr[5] == addr[5] && 
	    _addr[6] == addr[6] && _addr[7] == addr[7]);
  }

  void setAlarmTemps(TempBus &bus, char low, char high) {
    bus.setHighAlarmTemp(_addr, high);
    bus.setLowAlarmTemp(_addr, low);
  }

  boolean setup(TempBus &bus, char low, char high) {
    if (bus.isConnected(_addr)) {
      setAlarmTemps(bus, low, high);
      printStatus(Serial, bus);
      Serial.println();
      return TRUE;
    }
    return FALSE;
  }

  void loopAction(TempBus &bus) {
    _alarm = bus.hasAlarm(_addr, _temp);
    if (Globals.verbose()) {
      printShortStatus(Serial,bus);
      Serial.println();
    }
  }

  bool isAlarm() { return _alarm; }
  void resetAlarm() { _alarm = FALSE; }

};

#endif
