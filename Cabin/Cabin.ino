//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 
 
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define __REED_SWITCHES__

enum PINS {  
  REMOTE_POWER_HEAT_ON_PIN=2, 
  REMOTE_POWER_HEAT_OFF_PIN=3,
  PIR_PIN=4,
  FRONT_DOOR_PIN=5,
  SLIDING_DOOR_PIN=6,
  ENTRY_WINDOW_PIN=11,
  BATHROOM_WINDOW_PIN=12,
  KITCHEN_WINDOW_PIN=A0,
  LIVINGROOM_WINDOW_PIN=A1,
  GPRS_SOFTSERIAL_PIN1=7,
  GPRS_SOFTSERIAL_PIN2=8,
  GPRS_POWER_PIN=9,
  TEMP_INDOOR_PIN=10
};

struct Globals {
  boolean verbose;
  unsigned long now;
  boolean cmdMode;
} Globals = { 
  false,
  0,
  false
};

#include <ReedSwitch.h>
class ReedSwitch FD(FRONT_DOOR_PIN, "FD" ), 
  SD(SLIDING_DOOR_PIN, "SD"),
  EW(ENTRY_WINDOW_PIN, "EW"),
  BW(BATHROOM_WINDOW_PIN, "BW"),
  KW(KITCHEN_WINDOW_PIN, "KW"),
  LW(LIVINGROOM_WINDOW_PIN, "LW");

void 
printStatus(Stream &s)
{
  FD.printStatus(s);
  s.println();
  SD.printStatus(s);
  s.println();
  EW.printStatus(s);
  s.println();
  BW.printStatus(s);
  s.println();
  KW.printStatus(s);
  s.println();
  LW.printStatus(s);
  s.println();
}

class USBSerial {
 public:
  void setup() {
    Serial.begin(19200);
  }
  void loopAction() {
    if (Serial.available())  {          
      char serialInByte  = Serial.read();
      if (serialInByte=='~') Globals.cmdMode=!Globals.cmdMode;
      if (Globals.cmdMode) {
	switch(serialInByte) {
	case 's':
	  printStatus(Serial);
	  break;
	case 'v':
	  Globals.verbose=false;
	  break;
	case 'V':
	  Globals.verbose=true;
	  break;  
	default:
	  Serial.write(serialInByte);
	}
      }  
    }
  } 
} theUSBSerial;

void
setup()
{
  theUSBSerial.setup();

  Serial.println();
  Serial.println("CABIN SETUP: BEGIN");
#ifdef __REED_SWITCHES__
  Serial.println("  REED SWITCHES:");
  FD.setup();
  Serial.println();
  SD.setup();
  Serial.println();
  EW.setup();
  Serial.println();
  BW.setup();
  Serial.println();
  KW.setup();
  Serial.println();
  LW.setup();
  Serial.println();
#endif
  Serial.println();
  Serial.println("CABIN SETUP: END");
}

void
loop()
{
  Globals.now = millis();

#ifdef __REED_SWITCHES__  
  FD.loopAction();
  SD.loopAction();
  EW.loopAction();
  BW.loopAction();
  KW.loopAction();
  LW.loopAction();
#endif

  theUSBSerial.loopAction();
}



#if 0
#define ALARM_DEFAULT_RESEND_MINUTES 5
class Alarm {
private:
  boolean _armed;
  char msgs;
  char msgMax;
  unsigned long msgDelay;
  unsigned long lastSend;
  boolean sendMsg;
public:
  Alarm() {
      _armed = false; msgs = 0; msgMax=3; msgDelay=5000;
      lastSend = 0; sendMsg = false;
  }
  boolean armed() { return _armed; }
  void Disarm() { _armed = false; msgs = 0; }
  void Arm();
  void loopAction();
} theAlarm;




#define MOTION_DEFAULT_ALARM_THRESHOLD 5
#define MOTION_TEST_ALARM_THRESHOLD 100
class MotionDevice {
private:
  enum { PIRcalibrationTime=30 };           
  boolean PIRMotion;
  char _count;
  char _pin;
  char _alarmThreshold;
public:  
  MotionDevice(char pin) : _pin(pin) {
      PIRMotion = false;
      _count = 0;
      _alarmThreshold = MOTION_DEFAULT_ALARM_THRESHOLD;
  }
  void alarmThreshold(char v) { _alarmThreshold = v; } 
  boolean CheckAlarm();
  char count() { return _count; }
  void count(char v) { _count = v; }
  void setup();
  void loopAction();
} Motion(PIR_PIN);

void MotionDevice::loopAction()
{  
  if (digitalRead(_pin) ==  HIGH)  {
      if (PIRMotion==false) {
        _count++;
        if (Globals.verbose) {
          char tmp[8];
          snprintf(tmp, 8, "%ld", _count);
          Serial.print(" motion detected");
          Serial.println(tmp);
        }
        PIRMotion=true;
      }
  } else {
    if (PIRMotion==true) {
      if (Globals.verbose) Serial.println("motion ended");
      PIRMotion=false;
    }
  }
}


void MotionDevice::setup() {
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
   delay(50);
}

#define TEMP_DEFAULT_INDOOR_LOW 10
#define TEMP_DEFAULT_INDOOR_HIGH 45
class TempDevice {
private:
  OneWire oneWire;
  DallasTemperature sensors;
 // boolean on;
  char index; 
  char highAlarm;
  char lowAlarm;
  DeviceAddress addr;
public:
  TempDevice(char pin, char idx, char high, char low) : oneWire(pin), index(idx), sensors(&oneWire), highAlarm(high), lowAlarm(low) {
  }
  void PrintAlarms();
  void PrintAddress();
  void PrintTemperature();
  void PrintData();
  boolean CheckAlarm();
  void requestTemperatures() { sensors.requestTemperatures(); }
  char *getTempFByIndex() {
    sensors.getTempFByIndex(index);
  }
  void setup() {
    
  sensors.begin();
    // locate devices on the bus
    Serial.print("Temp: Found ");
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial.println(" devices.");
    if (!sensors.getAddress(addr, 0)) Serial.println("Temp: Unable to find address for Device 0"); 
    // alarm when temp is higher than 30C
    sensors.setHighAlarmTemp(addr, highAlarm);
    // alarm when temp is lower than 10C
    sensors.setLowAlarmTemp(addr, lowAlarm);
    PrintData();
    PrintAlarms();
  }
  
} IndoorTemp(0,TEMP_INDOOR_PIN,TEMP_DEFAULT_INDOOR_LOW,TEMP_DEFAULT_INDOOR_HIGH);

#define GPRSAUTHNUMLEN 15
char  * GPRSAuthorizedNumbers[]= {
  "\"+19175762236\"",
  "\"+19175093685\"",
  0
};
char *smsIdxStrs[] = {
  "0", 
  "1", 
  "2", 
  "3", 
  "4", 
  "5", 
  "6", 
  "7", 
  "8", 
  "9", 
  "10", 
  "11", 
  "12", 
  "13", 
  "14", 
  "15", 
  "16", 
  "17", 
  "18", 
  "19", 
  "20", 
  "21", 
  "22", 
  "23", 
  "24", 
  "25", 
  "26", 
  "27", 
  "28", 
  "29"
};

#define GPRSBUFFERLEN 128
class GPRSDevice {
  SoftwareSerial _serial;
  char buffer[GPRSBUFFERLEN]; // buffer array for data recieve over serial port
  char count;     // counter for buffer array
  
  boolean line;
  boolean authorizedMsg;
  boolean SMS;
  char    smsCnt;
  char    smsIdx;
  
  char numUnAuthorized;
  char from;
public:
  GPRSDevice() : _serial(GPRS_SOFTSERIAL_PIN1, GPRS_SOFTSERIAL_PIN2), count(0),
    line(false), authorizedMsg(false), SMS(false), smsCnt(0), smsIdx(0), numUnAuthorized(0), from(-1) {
      buffer[0]=0;
    }
    
  void MsgMemoryInfo() {
    _serial.println("AT+CPMS?");
    // reponds with +CPMS: "SM",18,30,"SM",18,30,"SM",18,30"
  }

  void TextModeSMS() {
    _serial.println( "AT+CMGF=1" );
  }

  void EchoOff() {
    _serial.println( "ATE0" );
  }

  void EchoOn() {
    _serial.println( "ATE1" );
  }

  void ReadSmsStore( String SmsStorePos ) {
    // Serial.print( "GprsReadSmsStore for storePos " );
    // Serial.println( SmsStorePos ); 
    _serial.print( "AT+CMGR=" );
    _serial.println( SmsStorePos );
  }

  void ReadSmsStore() {
    ReadSmsStore(smsIdxStrs[smsIdx]);
  }
  
  void DeleteMsgs() {
    _serial.println("AT+CMGD=1,4");
  }

  void write(char c) { _serial.write(c); }
  void TogglePower();

  void SendStatus(boolean from, boolean alarm);
  
  void DoSMSCmd();
  char NextIndex(char i);
  void ProcessLine();
  void setup(); 
  void loopAction();
}GPRS;

void 
GPRSDevice::setup()
{
     numUnAuthorized=0; 
     Serial.println("GPRS AUTHORIZED NUMBERS:");
     for (int i=0; GPRSAuthorizedNumbers[i]!=0; i++) {
       Serial.println(GPRSAuthorizedNumbers[i]);
     }
     pinMode(GPRS_POWER_PIN, OUTPUT); 
     digitalWrite(GPRS_POWER_PIN,LOW);
     _serial.begin(9600);               // the GPRS baud rate   
     Serial.println("GPRS SHIELD MONITOR: GPRS BAUD RATE: 9600");
     TogglePower();
}

void
GPRSDevice::loopAction()
{
  if (_serial.available())              // if date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while(_serial.available())          // reading data into char array 
    {  
      char GPRSInByte = _serial.read(); 
      if (GPRSInByte != 10 && GPRSInByte != 13) {
        buffer[count]=GPRSInByte;     // writing data into array
        count++;
      } 
      if (count == (128-1) || GPRSInByte == 13)  {
        buffer[count]=0;
        line=true;
        break;
      }
    }
    if (line==true) {
      ProcessLine();
      count=0;
      buffer[0]=0;
      line=false;
    }
  }
  if (smsCnt!=0) {
    if (SMS==false) ReadSmsStore();
  } else {
    updateMsgCnt();
  }
}



class RemoteDevice {
  char _onPin, _offPin;
public:
  void On();
  void Off();
  void setup();
  RemoteDevice(char onpin, char offpin) : _onPin(onpin), _offPin(offpin) {}
} RemoteHeat(REMOTE_POWER_HEAT_ON_PIN, REMOTE_POWER_HEAT_OFF_PIN);

void GPRSDevice::TogglePower()
{
  digitalWrite(GPRS_POWER_PIN,LOW);
  delay(1000);
  digitalWrite(GPRS_POWER_PIN,HIGH);
  delay(2000);
  digitalWrite(GPRS_POWER_PIN,LOW);
  delay(3000);
}

void RemoteDevice::setup()
{
    pinMode(_onPin, OUTPUT);
    digitalWrite(_onPin,LOW);
    pinMode(_offPin, OUTPUT);
    digitalWrite(_offPin,LOW);
    Off();  
}

void RemoteDevice::On()
{
  if (Globals.verbose) Serial.println("Remote Power: ON");
//  digitalWrite(REMOTE_POWER_ON_PIN,LOW);
//  delay(1000);
  digitalWrite(_onPin,HIGH);
  delay(1000);
  digitalWrite(_onPin,LOW);
//  delay(3000);
}
 
void RemoteDevice::Off()
{
  if (Globals.verbose) Serial.println("Remote Power: OFF");
//  digitalWrite(REMOTE_POWER_OFF_PIN,LOW);
//  delay(1000);
  digitalWrite(_offPin,HIGH);
  delay(1000);
  digitalWrite(_offPin,LOW);
//  delay(3000);
} 

void TempDevice::PrintAlarms()
{
  char temp;
  temp = sensors.getHighAlarmTemp(addr);
  Serial.print("High Alarm: ");
  Serial.print(temp, DEC);
  Serial.print("C/");
  Serial.print(DallasTemperature::toFahrenheit(temp));
  Serial.print("F | Low Alarm: ");
  temp = sensors.getLowAlarmTemp(addr);
  Serial.print(temp, DEC);
  Serial.print("C/");
  Serial.print(DallasTemperature::toFahrenheit(temp));
  Serial.print("F");
} 

// function to print a device address
void TempDevice::PrintAddress()
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (addr[i] < 16) Serial.print("0");
    Serial.print(addr[i], HEX);
  }
}

// function to print the temperature for a device
void TempDevice::PrintTemperature()
{
  float tempC = sensors.getTempC(addr);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// main function to print information about a device
void TempDevice::PrintData()
{
  Serial.print("Device Address: ");
  PrintAddress();
  Serial.print(" ");
  PrintTemperature();
  Serial.println();
}

boolean TempDevice::CheckAlarm()
{
  if (sensors.hasAlarm(addr))
  {
  if (Globals.verbose) {    
      Serial.print("TEMP ALARM: ");
      PrintData();
  }
  return true;
  }
  return false;
}

boolean MotionDevice::CheckAlarm()
{
  if (_count > _alarmThreshold) {
    return true;
  } else return false;
}




void GPRSDevice::SendStatus(boolean fromflag, boolean alarm)
{
  char tmp[20];
  char inByte=0;
  
 for (int i=0; GPRSAuthorizedNumbers[i]!=0; i++) {
    if (fromflag == false || from == i) { 
#ifdef DEBUG
    Serial.print("Sending Status to : ");
    Serial.println(GPRSAuthorizedNumbers[i]);
#endif
    _serial.print("AT+CMGS=");
//    GPRSSerial.println("\"+19175762236\"");
    _serial.println(GPRSAuthorizedNumbers[i]);
    while (inByte!=' ') {
     while (!_serial.available());
       inByte=_serial.read();
    }
#ifdef DEBUG
    Serial.println("READY TO SEND STATUS");
#endif    
    if (alarm==true) _serial.print("* ALARM * :"); else _serial.print("STATUS:");
    if (theAlarm.armed() == true) _serial.print(" ARMED");
    else _serial.print(" DISARMED");
    
    _serial.print(" Temp: ");
    IndoorTemp.requestTemperatures(); // Send the command to get temperatures
    _serial.print(IndoorTemp.getTempFByIndex()); 
    snprintf(tmp, 20, " Motion: %ld", Motion.count());
    _serial.print(tmp);
    snprintf(tmp, 20, " SMS: %ld", numUnAuthorized);
    _serial.println(tmp);
    tmp[0]=0x1A;
    _serial.write((unsigned char *)tmp,1); 
  }
 }
}




void GPRSDevice::DoSMSCmd()
{
  boolean sndStatus=false;
  if (strncmp(buffer, "Heat on", 7)==0) {
    RemoteHeat.On();
    sndStatus=true;
  }
  if (strncmp(buffer, "Heat off", 8)==0) {
    RemoteHeat.Off();
    sndStatus=true;
  }
  if (strncmp(buffer, "Status", 6) == 0) {
    sndStatus=true;
  }
  if (strncmp(buffer, "Arm", 3) == 0) {
    theAlarm.Arm();
    sndStatus = true;
  }
  if (strncmp(buffer, "Disarm", 6) == 0) {
    theAlarm.Disarm();
    sndStatus = true;
  }
  if (sndStatus == true) SendStatus(true, false);
}

char
GPRSDevice::NextIndex(char i)
{
   for (;i<count; i++) {
       if (buffer[i]==',') break;
   }
   return i;
}

void GPRSDevice::ProcessLine()
{
  if (count == 0) return;

 if (SMS) {
    if (authorizedMsg) {
      DoSMSCmd();
      authorizedMsg=false;
    }
    
    DeleteMsgs();
    SMS=false;  
    return;
  }
  
#ifdef DEBUG  
  Serial.print("GPRS LINE:");
  Serial.write((unsigned char *)GPRS.buffer, GPRS.count);
  Serial.println();
#endif

  if (strstr(buffer, "NORMAL POWER DOWN") != NULL) {
    TogglePower();
  }
  
  if (strstr(buffer, "Call Ready") != NULL) {
    TextModeSMS();
    EchoOff();
    return;
  }

#if 0  
  if (strstr(GPRS.buffer, "+CMTI") != NULL) {
    int i=GPRSNextIndex(0);
    if (i==GPRS.count) {
      Serial.println("ERROR: Parsing +CMTI");
      return;
    }
    i++;
    GPRSReadSmsStore(&GPRS.buffer[i]);
    return;
  }
#endif  
  
  if (strstr(buffer, "+CMGR:") != NULL) {
    char i=NextIndex(0);
    if (i==count) {
      Serial.println("ERROR: Parsing +CMGR");
      return;
    }
    i++;
    char j=NextIndex(i);
    buffer[j]=0;
    authorizedMsg=false; 
    for (j=0; GPRSAuthorizedNumbers[j]!=0; j++) {
#ifdef DEBUG
        Serial.print("Checking from: ");
        Serial.write((unsigned char *)&buffer[i],GPRSAUTHNUMLEN);
        Serial.print(" against ");
        Serial.print(GPRSAuthorizedNumbers[j]);
        Serial.println();
#endif
      if (strncmp(&buffer[i], GPRSAuthorizedNumbers[j], GPRSAUTHNUMLEN) == 0) {
        authorizedMsg=true;
        from=j;
#ifdef DEBUG
        Serial.print("Authorized Message from: ");
        Serial.println(GPRSAuthorizedNumbers[j]);
#endif
        break;
      }
    }
    if (authorizedMsg==false) numUnAuthorized++;
    SMS=true;
    return;
  }
 
}

void
Alarm::Arm()
{
 _armed = true;
 msgs = 0;
 Motion.alarmThreshold(MOTION_DEFAULT_ALARM_THRESHOLD);
 Motion.count(0);
}

void
Alarm::loopAction()
{
  if (_armed==true) {
    sendMsg = false;
    if (IndoorTemp.CheckAlarm() || Motion.CheckAlarm()) {
      if (msgs==0) { 
        lastSend=Globals.now; 
        sendMsg = true;
        Motion.alarmThreshold(5); 
       } else {
        if ((Globals.now - lastSend) > msgDelay) sendMsg=true;
      }
      if (sendMsg==true) {
        Serial.println("Sending Alarm");
        GPRS.SendStatus(false, true);
        Motion.count(0);
        msgs++;
        if (msgs>msgMax) { 
          Serial.print("Alarm.msgs sent turning off");
          Disarm();
        }
      }
    }
  }
}


void setup()
{
  Serial.begin(19200);             // the Serial port of Arduino baud rate.
 
#ifdef PIR
  Motion.setup();
#endif

  FD.setup();
  SD.setup();

  IndoorTemp.setup();
  
  Serial.println();
  
  RemoteHeat.setup();

  GPRS.setup();
}

void loop()
{
  Globals.now = millis();
  
  GPRS.loopAction();
  if (Serial.available())  {          // if data is available on hardwareserial port ==> data is comming from PC or notebook
     char serialInByte  = Serial.read();
     if (serialInByte=='~') Globals.cmdMode=!Globals.cmdMode;
     if (Globals.cmdMode) {
       switch(serialInByte) {
         case 'P':
           RemoteHeat.On();
           break;
         case 'p':
           RemoteHeat.Off();
           break;
         case 'r':
           GPRS.TogglePower();
           break;
         case 't':
           IndoorTemp.PrintTemperature();
           break;
         case 'v':
           Globals.verbose=false;
           break;
         case 'V':
           Globals.verbose=true;
           break;  
         default:
           Serial.write(serialInByte);
       }
     } else {
       GPRS.write(serialInByte);       // write it to the GPRS shield
     }
  }

#ifdef PIR
  Motion.loopAction();
#endif
  FD.loopAction();
  SD.loopAction();
  theAlarm.loopAction();
}

#endif
