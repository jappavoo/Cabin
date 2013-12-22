#ifndef __TEMPERATURE_h__
#define __TEMPERATURE_h__

#define TEMP_CHECK_DELAY_MS 1000

class TempBus : public DallasTemperature {
  OneWire _oneWire;
  char _numDev; 
public:
  TempBus(char pin) : DallasTemperature(&_oneWire), 
		      _oneWire(pin), _numDev(0) {}

  void printTemperature(DeviceAddress deviceAddress, Stream &s) {
    float tempC = getTempC(deviceAddress);
    Serial.print(tempC);
    Serial.print("C/");
    Serial.print(toFahrenheit(tempC));
    Serial.print("F");
  }

  void printAlarms(DeviceAddress deviceAddress, Stream &s)
  {
    char temp;
    temp = getHighAlarmTemp(deviceAddress);
    s.print("High Alarm: ");
    s.print(temp, DEC);
    s.print("C/");
    s.print(toFahrenheit(temp));
    s.print("F | Low Alarm: ");
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
    s.print("Resolution: ");
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
    Serial.print("TempBus: Global Power Mode: ");
    printGlobalPowerMode(Serial);
    Serial.println();
    Serial.println("TempBus: Searching for Devices:");
    _numDev = getDeviceCount();
    Serial.print(" Found ");
    Serial.print(_numDev, DEC);
    Serial.print(" devices. Requesting Temperatures ...");
    requestTemperatures();
    Serial.println("Probing Thermometers:");
    for (char i=0; i<_numDev; i++)  {
      Serial.print(i, DEC);
      Serial.print(": ");
      if (getAddress(addr, i)) {
        printDetails(addr, Serial);
      } else {
	Serial.println(" Error getting address!");
      }
    }
  }

  void loopAction() {
#if 0
    if ((Globals.now - _last) > TEMP_CHECK_DELAY_MS) {
      for (i=0; i<_numTherms; i++) {
	_thermometers[i].getTempC();
      }
    }
#endif
  }

};

#if 0
class DS18B20 {
  DeviceAddress _addr;
public:
  DS18B20(uint8_t addr);

  void printStatus(Stream &s) {
  }

  void printShortStatus(Stream &s) {
  }

  bool addrMatch(uint8_t *addr) {
    return _addr[0] == addr[0] && _addr[1] == addr[1] && _addr[2] == addr[2] && 
      _addr[3] == addr[3] &&
  }

  void setup() {
  }

  void loopAction() {
  }

  bool isAlarm() {
  }

};
#endif

#endif
