//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 
 
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

enum PINS {  
  USB_SERIAL_RX=0,
  USB_SERIAL_TX=1,

  TEMP_BUS_PIN=2,               // 3 ext pins
  MOTION_DETECTOR_PIN=3,
  REMOTE_POWER_HEAT_OFF_PIN=4,
  REMOTE_POWER_HEAT_ON_PIN=5, 

  FRONT_DOOR_PIN=6,             // 2 ext pins 

  GPRS_SOFTSERIAL_PIN1=7,
  GPRS_SOFTSERIAL_PIN2=8,
  GPRS_POWER_PIN=9,

  SLIDING_DOOR_PIN=10,          // 2 ext pins 
  ENTRY_WINDOW_PIN=11,          // 2 ext pins 
  BATHROOM_WINDOW_PIN=12,       // 2 ext pins 
  SPEAKER_PIN=13,
  KITCHEN_WINDOW_PIN=A0,        // 2 ext pins 
  LIVINGROOM_WINDOW_PIN=A1,     // 2 ext pins 

  CABIN_VOLTAGE_PIN=A2,        // 2 wire external barrel connector
  REMOTE_POWER_VOLTAGE_PIN=A3,        // 2 wire external barrel connector

  FREE_PIN0=A4,
  FREE_PIN1=A5
};

class Globals {
  unsigned long _now;
  
  enum { FLGS_verbose_BIT=0, FLGS_cmdMode_BIT=1,   
	 FLGS_indoorTemp_BIT=2, FLGS_outdoorTemp_BIT=3,
	 FLGS_motion_BIT=4,
	 FLGS_remoteHeat_BIT=5,
	 FLGS_gprs_BIT=6,
	 FLGS_FD_BIT=7, FLGS_SD_BIT=8, FLGS_EW_BIT=9, FLGS_BW_BIT=10, FLGS_KW_BIT=11,
	 FLGS_LW_BIT=12,
	 FLGS_speaker_BIT=13,
	 FLGS_cabinVoltage_BIT=14,
	 FLGS_remoteHeatVoltage_BIT=15 };
	 
  unsigned int flags;

#define DEFINE_FLG_METHODS(FLG)					\
  inline bool FLG () { return flags & (1 << FLGS_##FLG##_BIT); } \
  inline void enable_##FLG () { flags |= (1 << FLGS_##FLG##_BIT); }	\
  inline void disable_##FLG () { flags &= ~(1 << FLGS_##FLG##_BIT); }

public:
  inline void set_now(unsigned long t) { _now = t; }
  inline unsigned long get_now() { return _now; }

  DEFINE_FLG_METHODS(verbose);
  DEFINE_FLG_METHODS(cmdMode);

  DEFINE_FLG_METHODS(indoorTemp);
  DEFINE_FLG_METHODS(outdoorTemp);

  DEFINE_FLG_METHODS(motion);

  DEFINE_FLG_METHODS(remoteHeat);

  DEFINE_FLG_METHODS(gprs);

  DEFINE_FLG_METHODS(FD);
  DEFINE_FLG_METHODS(SD);
  DEFINE_FLG_METHODS(EW);
  DEFINE_FLG_METHODS(BW);
  DEFINE_FLG_METHODS(KW);
  DEFINE_FLG_METHODS(LW);

  DEFINE_FLG_METHODS(speaker);
  DEFINE_FLG_METHODS(cabinVoltage);
  DEFINE_FLG_METHODS(remoteHeatVoltage);

  void
  setup() {
    set_now(0);
    // turn everything on by default
    flags = 0xFFFF;
    
    // turn off these features
    disable_verbose();
    disable_cmdMode();
    disable_motion();
    disable_FD();
    disable_SD();
    disable_EW();
    disable_BW();
    disable_KW();
    disable_LW();
    disable_indoorTemp();
    disable_outdoorTemp();
  }
} Globals;
 

#include <ReedSwitch.h>
class ReedSwitch FD(FRONT_DOOR_PIN, "FD" ), 
  SD(SLIDING_DOOR_PIN, "SD"),
  EW(ENTRY_WINDOW_PIN, "EW"),
  BW(BATHROOM_WINDOW_PIN, "BW"),
  KW(KITCHEN_WINDOW_PIN, "KW"),
  LW(LIVINGROOM_WINDOW_PIN, "LW");

#include <MotionDetector.h>
class MotionDetector theMotionDetector(MOTION_DETECTOR_PIN);

#include <RemotePower.h>
class RemotePower theRemoteHeat(REMOTE_POWER_HEAT_ON_PIN, 
				REMOTE_POWER_HEAT_OFF_PIN,
				"HEAT");

#include <Temperature.h>
class TempBus theTempBus(TEMP_BUS_PIN);

void 
printStatus(Stream &s)
{
  if (Globals.FD()) {
    FD.printStatus(s);
    s.println();
  }
  if (Globals.SD()) {
    SD.printStatus(s);
    s.println();
  }
  if (Globals.EW()) {
    EW.printStatus(s);
    s.println();
  }
  if (Globals.BW()) {
    BW.printStatus(s);
    s.println();
  }
  if (Globals.KW()) {
    KW.printStatus(s);
    s.println();
  }
  if (Globals.LW()) {
    LW.printStatus(s);
    s.println();

  }
  if (Globals.motion()) {
    theMotionDetector.printStatus(s);
    s.println();
  }
  if (Globals.remoteHeat()) {
    theRemoteHeat.printStatus(s);
  }
}


class USBSerial {
 public:
  void setup() {
    Serial.begin(19200);
  }
  void loopAction() {
    if (Serial.available())  {          
      char serialInByte  = Serial.read();
      if (serialInByte=='~') {
	if (Globals.cmdMode()) Globals.disable_cmdMode();
	else Globals.enable_cmdMode();
      }
      if (Globals.cmdMode()) {
	switch(serialInByte) {
	case 's':
	  printStatus(Serial);
	  Serial.println();
	  break;
	case 'p':
	  theRemoteHeat.On();
	  printStatus(Serial);
	  Serial.println();
	  break;
	case 'P':
	  theRemoteHeat.Off();
	  printStatus(Serial);
	  Serial.println();
	  break;
	case 'v':
	  Globals.enable_verbose();
	  break;
	case 'V':
	  Globals.disable_verbose();
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
  Globals.setup();

  theUSBSerial.setup();

  Serial.println();
  Serial.println("CABIN SETUP: BEGIN");

  if (Globals.remoteHeat()) {
    Serial.println("  REMOTE POWER:");
    theRemoteHeat.setup();
    Serial.println();
  }

  if (Globals.FD() || Globals.SD() || Globals.EW() || Globals.BW() ||
      Globals.KW() || Globals.LW()) {
    Serial.println("  REED SWITCHES:");
    if (Globals.FD()) {
      FD.setup();
      Serial.println();
    }
    if (Globals.SD()) {
      SD.setup();
      Serial.println();
    }
    if (Globals.EW()) {
      EW.setup();
      Serial.println();
    }
    if (Globals.BW()) {
      BW.setup();
      Serial.println();
    }
    if (Globals.KW()) {
      KW.setup();
      Serial.println();
    }
    if (Globals.LW()) {
      LW.setup();
      Serial.println();
    }
  }

  if (Globals.indoorTemp() || Globals.outdoorTemp()) {
    Serial.println("  TEMP BUS:");
    theTempBus.setup();
  }

  if (Globals.motion()) {
    Serial.println("  MOTION DETECTOR:");
    theMotionDetector.setup();
    Serial.println();
  }

  Serial.println();
  Serial.println("CABIN SETUP: END");
}

void
loop()
{
  Globals.set_now(millis());

  if (Globals.FD()) FD.loopAction();
  if (Globals.SD()) SD.loopAction();
  if (Globals.EW()) EW.loopAction();
  if (Globals.BW()) BW.loopAction();
  if (Globals.KW()) KW.loopAction();
  if (Globals.LW()) LW.loopAction();

  if (Globals.motion()) theMotionDetector.loopAction();

  if (Globals.remoteHeat()) theRemoteHeat.loopAction();

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




void GPRSDevice::TogglePower()
{
  digitalWrite(GPRS_POWER_PIN,LOW);
  delay(1000);
  digitalWrite(GPRS_POWER_PIN,HIGH);
  delay(2000);
  digitalWrite(GPRS_POWER_PIN,LOW);
  delay(3000);
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
