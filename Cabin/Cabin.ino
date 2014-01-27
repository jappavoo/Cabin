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

  CABIN_POWER_PIN=A2,        // 2 wire external barrel connector
  REMOTE_POWER_PIN=A3,        // 2 wire external barrel connector

  RED_LED_PIN=A4,
  YELLOW_LED_PIN=A5
};

#include <Led.h> 

class Globals {
  enum { FLGS_verbose_BIT=0, FLGS_cmdMode_BIT=1,   
	 FLGS_indoorTemp_BIT=2, FLGS_outdoorTemp_BIT=3,
	 FLGS_motion_BIT=4,
	 FLGS_remoteHeat_BIT=5,
	 FLGS_gprs_BIT=6,
	 FLGS_FD_BIT=7, FLGS_SD_BIT=8, FLGS_EW_BIT=9, FLGS_BW_BIT=10, FLGS_KW_BIT=11,
	 FLGS_LW_BIT=12,
	 FLGS_speaker_BIT=13,
	 FLGS_cabinPower_BIT=14,
	 FLGS_remotePower_BIT=15,
	 FLGS_gprsModem_BIT=16,
	 FLGS_Alarm_BIT=17
  };
	 
  uint32_t flags;
  uint32_t statusCnt;

#define DEFINE_FLG_METHODS(FLG)					\
  inline bool FLG () { return flags & ((uint32_t)1 << FLGS_##FLG##_BIT); } \
  inline void enable_##FLG () { flags |= ((uint32_t)1 << FLGS_##FLG##_BIT); } \
  inline void disable_##FLG () { flags &= ~((uint32_t)1 << FLGS_##FLG##_BIT); }

public:
  boolean hasSerialByte;
  uint8_t serialInByte;
  Led redLed;
  Led yellowLed;
  Globals() : flags(0), statusCnt(0),
	      redLed(RED_LED_PIN, "Red"), yellowLed(YELLOW_LED_PIN, "Yellow") 
  {}
  //  inline void set_now(unsigned long t) { _now = t; }
  //  inline unsigned long get_now() { return _now; }

 
  DEFINE_FLG_METHODS(verbose);
  DEFINE_FLG_METHODS(cmdMode);

  DEFINE_FLG_METHODS(indoorTemp);
  DEFINE_FLG_METHODS(outdoorTemp);

  DEFINE_FLG_METHODS(motion);

  DEFINE_FLG_METHODS(remoteHeat);

  DEFINE_FLG_METHODS(FD);
  DEFINE_FLG_METHODS(SD);
  DEFINE_FLG_METHODS(EW);
  DEFINE_FLG_METHODS(BW);
  DEFINE_FLG_METHODS(KW);
  DEFINE_FLG_METHODS(LW);

  DEFINE_FLG_METHODS(speaker);
  DEFINE_FLG_METHODS(cabinPower);
  DEFINE_FLG_METHODS(remotePower);

  DEFINE_FLG_METHODS(gprsModem);
  
  DEFINE_FLG_METHODS(Alarm);

  void printStatus(Stream &s);
  void printShortStatus(Stream &s);

  void ExitCmdMode(); 

  void EnterCmdMode();

  void setup() {
    redLed.setup();
    Serial.print(" ");
    yellowLed.setup();
    //set_now(0);
    // turn everything on by default
    flags = 0xFFFFFFFF;
    
    // turn off these features
    disable_verbose();
    // disable_cmdMode();
    // disable_motion();
    // disable_FD();
    // disable_SD();
    // disable_EW();
    // disable_BW();
    // disable_KW();
    // disable_LW();
    // disable_remoteHeat();
    // disable_indoorTemp();
    // disable_outdoorTemp();
    // disable_cabinPower();
    // disable_remotePower();
    // disable_gprsModem();
    // disable_Alarm();
  }
} Globals;

#include <ReedSwitch.h>
class ReedSwitch FD(FRONT_DOOR_PIN, "FD" ), 
  SD(SLIDING_DOOR_PIN, "SD"),
  EW(ENTRY_WINDOW_PIN, "EW"),
  BW(BATHROOM_WINDOW_PIN, "BW"),
  KW(KITCHEN_WINDOW_PIN, "KW"),
  LW(LIVINGROOM_WINDOW_PIN, "LW");

#include <Power.h>
class Power cabinPower(CABIN_POWER_PIN, "Power");
class Power remotePower(REMOTE_POWER_PIN, "Remote Power");

#include <MotionDetector.h>
class MotionDetector theMotionDetector(MOTION_DETECTOR_PIN);

#include <RemotePower.h>
class RemotePower theRemoteHeat(REMOTE_POWER_HEAT_ON_PIN, 
				REMOTE_POWER_HEAT_OFF_PIN,
				"HEAT");

#include <Temperature.h>
class TempBus theTempBus(TEMP_BUS_PIN);
#define TEMP_DEFAULT_INDOOR_LOW 9
#define TEMP_DEFAULT_INDOOR_HIGH 40
uint8_t INDOORTHERMOMETERADDR[8] = { 0x28,0x0A,0x73,0x2F,0x05,0x00,0x00,0x2D };
#define TEMP_DEFAULT_OUTDOOR_LOW -35
#define TEMP_DEFAULT_OUTDOOR_HIGH 40
// original adafruit plain sensor
//uint8_t OUTDOORTHERMOMETERADDR[8] = { 0x28,0x9A,0x9C,0x2F,0x05,0x00,0x00,0x29 };
// new enclosed probe sensor ordered from AMAZON
uint8_t OUTDOORTHERMOMETERADDR[8] = { 0x28,0x45,0x4F,0xD0,0x04,0x00,0x00,0x99 };
class DS18B20 indoorThermometer(INDOORTHERMOMETERADDR,"Indoor");
class DS18B20 outdoorThermometer(OUTDOORTHERMOMETERADDR,"Outdoor");


#include <GPRSModem.h>
const char  * GPRSAuthorizedNumbers[]= {
  "\"+19175762236\"",
  "\"+19175093685\"",
  0
}; 
const long GPRS_SOFSERIAL_BAUD=4800;

GPRSModem gprsModem(GPRS_SOFTSERIAL_PIN1, GPRS_SOFTSERIAL_PIN2, GPRS_POWER_PIN);


void 
Globals::ExitCmdMode()  
{
  Serial.println("Exiting Command Mode");
  disable_cmdMode();
  if (gprsModem()) {
    ::gprsModem.flush();
    ::gprsModem.EchoOn();
    ::gprsModem.waitForResponse();
  }
  hasSerialByte=FALSE;
}

void 
Globals::EnterCmdMode() {
  Serial.println("Entering Command Mode");
  enable_cmdMode();
  if (gprsModem()) {
    ::gprsModem.flush();
    ::gprsModem.EchoOff();
    ::gprsModem.waitForResponse();
    
    ::gprsModem.flush();
    ::gprsModem.turnOffMsgIndications();
    ::gprsModem.waitForResponse();

    ::gprsModem.flush();
    ::gprsModem.TextModeSMS();
    ::gprsModem.waitForResponse();
  }
}

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
    _armed = false; msgs = 0; msgMax=3; msgDelay=60000 * ALARM_DEFAULT_RESEND_MINUTES;
    lastSend = 0; sendMsg = false;
  }
  boolean isArmed() { return _armed; }
  void Disarm() { 
    _armed = false; msgs = 0; 
    Globals.redLed.Off();
    //    Serial.println("Alarm: Disarmed"); 
  }
  void Clear() {
      // reset all device Alarms
      theMotionDetector.resetAlarm();
      FD.resetAlarm(); SD.resetAlarm(); EW.resetAlarm(); 
      BW.resetAlarm(); KW.resetAlarm(); LW.resetAlarm();
      indoorThermometer.resetAlarm(); outdoorThermometer.resetAlarm();
      theRemoteHeat.resetAlarm();
      cabinPower.resetAlarm(); remotePower.resetAlarm();
      gprsModem.resetAlarm();
  } 
  void Arm() {
    if (!_armed) {
      _armed = TRUE;
      msgs = 0;
      Clear();
      Globals.redLed.flash(Led::SLOW);
      // Serial.println("Alarm: Armed");
    }
  }

  inline boolean isAlarm() {
    return (Globals.motion() && theMotionDetector.isAlarm()) || 
      (Globals.FD() && FD.isAlarm()) || (Globals.SD() && SD.isAlarm()) || 
      (Globals.EW() && EW.isAlarm()) || (Globals.BW() && BW.isAlarm()) || 
      (Globals.KW() && KW.isAlarm()) || (Globals.LW() && LW.isAlarm()) ||
      (Globals.indoorTemp() && indoorThermometer.isAlarm()) || 
      (Globals.outdoorTemp() && outdoorThermometer.isAlarm()) ||
      (Globals.remoteHeat() && theRemoteHeat.isAlarm()) ||
      (Globals.cabinPower() && cabinPower.isAlarm()) || 
      (Globals.gprsModem() && gprsModem.isAlarm());
  }

  boolean printShortStatus(Stream &s) {
    s.print("Alarm:");
    if (isAlarm()) { s.print("*ALARM "); s.print(msgs, DEC); s.print("* "); }
    if (isArmed()) s.print("ARMED");
    else s.print("Disarmed"); 
    return TRUE;
  }

  void printStatus(Stream &s) {
    if (isAlarm()) s.print("*ALARM* ");
    else s.print("NO Alarms ");
    if (isArmed()) s.print("ARMED"); 
    else s.print("Disarmed");
  }

  void setup() {
    printStatus(Serial);
  }

  void ackMsg() {
    if (sendMsg) sendMsg=FALSE;
  }

  void loopAction() {
    if (_armed==true) {
      sendMsg = false;
      if (isAlarm()) {
	if (msgs==0) {
	  Globals.redLed.flash(Led::FAST);
	  lastSend=millis(); 
	  sendMsg = true;
	} else {
	  if ((millis() - lastSend) > msgDelay) sendMsg=true;
	}
	if (sendMsg==true) {
	  Globals.printShortStatus(Serial);
	  if (Globals.gprsModem()) {
	    for (int i=gprsModem.firstAuthorizedIndex(); 
		 gprsModem.validAuthorizedIndex(i); 
		 i=gprsModem.nextAuthorizedIndex(i)) {
#if 0
	      Serial.print("Sending Alarm SMS to ");
	      Serial.println(gprsModem.AuthorizedNumber(i));
#endif
 	      gprsModem.flush();
	      gprsModem.openSMS(i);
	      Globals.printShortStatus(gprsModem);
	      gprsModem.closeSMS();
	      gprsModem.waitForResponse();
	    }
	  }	 
	  msgs++;
	  if (msgs==msgMax) { 
	    //	    Serial.print("Alarm.msgs sent turning off");
	    Disarm();
	  } else {
	    lastSend=millis();
	  }
	}
      }
    }
  }
} theAlarm;

class USBSerial {
 public:
  void setup() {
    Serial.begin(19200);
  }
  void loopAction() {
    Globals.hasSerialByte = FALSE;
    if (Serial.available())  {          
      Globals.serialInByte  = Serial.read();
      Globals.hasSerialByte = TRUE;
      if (Globals.serialInByte=='~') {
	if (Globals.cmdMode()) {
	  Globals.ExitCmdMode();
	} else {
	  Globals.EnterCmdMode();
	}  
      }
      if (Globals.cmdMode()) {
	switch(Globals.serialInByte) {
	case 's':
	  Globals.printShortStatus(Serial);
	  break;
	case 'S':
	  Globals.printStatus(Serial);
	  break;
	case 'p':
	  theRemoteHeat.On();
	  // Globals.printStatus(Serial);
	  break;
	case 'P':
	  theRemoteHeat.Off();
	  // Globals.printStatus(Serial);
	  break;
	case 'v':
	  Globals.enable_verbose();
	  break;
	case 'V':
	  Globals.disable_verbose();
	  break;
	case 'a':
	  if (Globals.Alarm()) theAlarm.Arm();
	  break;
	case 'A':
	  if (Globals.Alarm()) theAlarm.Disarm();
	  break;
	case 'C':
	  if (Globals.Alarm()) theAlarm.Clear();
	  break;
#if 0
	case 'c':
	  Globals.redLed.flash(Led::SLOW);
	  Globals.yellowLed.flash(Led::SLOW);
	  break;
	case 'm':
	  Globals.redLed.flash(Led::MEDIUM);
	  Globals.yellowLed.flash(Led::MEDIUM);
	  break;
	case 'f':
	  Globals.redLed.flash(Led::FAST);
	  Globals.yellowLed.flash(Led::FAST);
	  break;
	case 'l':
	  Globals.redLed.On();
	  Globals.yellowLed.On();
	  break;
	case 'L':
	  Globals.redLed.Off();
	  Globals.yellowLed.Off();
	  break;
#endif
	case 'm':
	  Globals.enable_motion();
	  break;
	case 'M':
	  Globals.disable_motion();
	  break;
	case 'o':
	  if (Globals.gprsModem()) {
	    if (gprsModem.isOn()) Serial.println("gprsModem: ON");
	    else Serial.println("gprsModem: OFF");
	  }
	  break;
	case 'g':
	  if (Globals.gprsModem()) gprsModem.TogglePower();
	  break;
	case 'w':
	  Globals.printShortStatus(Serial);
	  if (Globals.gprsModem()) {
	    gprsModem.flush();
	    gprsModem.openSMS(0);
	    Globals.printShortStatus(gprsModem);
	    gprsModem.closeSMS();
	    gprsModem.waitForResponse();
	  }
	  break;
	case 'f':
	  if (Globals.gprsModem()) gprsModem.flush();
	  break;
	case 'd':
	  if (Globals.gprsModem()) { 
	    Serial.println(gprsModem.last4(), HEX);
	    Serial.println(gprsModem.responseTerminator(), HEX);
	    gprsModem.dumpBuffer();
	  }
	  break;
	case 'r':
	  if (Globals.gprsModem()) { 
	    gprsModem.ReadSmsStore((uint8_t)2);
	    Serial.println(gprsModem.waitForResponse(), DEC);
	  }
	case 'z':
	  if (Globals.gprsModem()) gprsModem.ListSMS();
	  break;
	default:
	  Serial.write(Globals.serialInByte);
	}
      }  
    }
  } 
} theUSBSerial;


void 
Globals::printShortStatus(Stream &s) 
{
  s.print(statusCnt, DEC); s.print(": ");
  statusCnt++;
  if (Alarm() && theAlarm.printShortStatus(s)) s.print(" "); 
  if (FD() && ::FD.printShortStatus(s)) s.print(" ");
  if (SD() && ::SD.printShortStatus(s)) s.print(" ");
  if (EW() && ::EW.printShortStatus(s)) s.print(" ");
  if (BW() && ::BW.printShortStatus(s)) s.print(" ");
  if (KW() && ::KW.printShortStatus(s)) s.print(" ");
  if (LW() && ::LW.printShortStatus(s)) s.print(" ");
  if (indoorTemp() && indoorThermometer.printShortStatus(s,theTempBus)) s.print(" ");
  if (outdoorTemp() && outdoorThermometer.printShortStatus(s,theTempBus)) s.print(" ");
  if (motion() && theMotionDetector.printShortStatus(s)) s.print(" ");
  if (remoteHeat() && theRemoteHeat.printShortStatus(s)) s.print(" ");
  if (cabinPower() && ::cabinPower.printShortStatus(s)) s.print(" ");
  //  if (remotePower() && ::remotePower.printShortStatus(s)) s.print(" ");
  if (gprsModem() && ::gprsModem.printShortStatus(s)) s.print(" ");
  s.println();
}

void 
Globals::printStatus(Stream &s)
{
  s.print(statusCnt, DEC); s.print(": ");
  statusCnt++;
  if (Alarm()) { theAlarm.printStatus(s); s.println(); }
  if (FD()) { ::FD.printStatus(s); s.println();  }
  if (SD()) { ::SD.printStatus(s); s.println(); }
  if (EW()) { ::EW.printStatus(s); s.println(); }
  if (BW()) { ::BW.printStatus(s); s.println(); }
  if (KW()) { ::KW.printStatus(s); s.println(); }
  if (LW()) { ::LW.printStatus(s); s.println(); }
  if (indoorTemp()) { indoorThermometer.printStatus(s,theTempBus); s.println(); }
  if (outdoorTemp()) { outdoorThermometer.printStatus(s,theTempBus); s.println(); }
  if (motion()) { theMotionDetector.printStatus(s); s.println(); }
  if (remoteHeat()) { theRemoteHeat.printStatus(s); s.println(); }
  if (cabinPower()) { ::cabinPower.printStatus(s); s.println(); }
  if (remotePower()) { ::remotePower.printStatus(s); s.println(); }
  if (gprsModem()) { ::gprsModem.printStatus(s); s.println(); }
  redLed.printStatus(s); s.println();
  yellowLed.printStatus(s); s.println();

}


void processCmd(int anidx, char *buffer) 
{
  boolean sndStatus=false;

  Serial.print("processCmd:");
  for (int j=0; buffer[j]!='\n'; j++) { if (buffer[j]!='\r') Serial.print(buffer[j]); }
  Serial.println();
  if (strncmp(buffer, "Disarm", 6) == 0) {
    theAlarm.Disarm();
    sndStatus = true;
  } else if (strncmp(buffer, "Status", 6) == 0) {
    sndStatus=true;
  } else if (strncmp(buffer, "Arm", 3) == 0) {
    theAlarm.Arm();
    sndStatus = true;
  } else if (strncmp(buffer, "Clear", 5) == 0) {
    theAlarm.Clear();
    sndStatus = true;
  } else if (strncmp(buffer, "Heat on", 7)==0) {
    theRemoteHeat.On();
    sndStatus=true;
  } else if (strncmp(buffer, "Heat off", 8)==0) {
    theRemoteHeat.Off();
    sndStatus=true;
  } else if (strncmp(buffer, "Motion off", 10) == 0) {
    Globals.disable_motion();
  } else if (strncmp(buffer, "Motion on", 9) == 0) {
    Globals.enable_motion();
  } 
  if (sndStatus == true) {    
    Serial.print(anidx, DEC);
    Serial.print(": ");
    Serial.print(gprsModem.AuthorizedNumber(anidx));
    Serial.print(": ");
    Globals.printShortStatus(Serial);

    gprsModem.flush();
    gprsModem.openSMS(anidx);
    Globals.printShortStatus(gprsModem);
    gprsModem.closeSMS();
    gprsModem.waitForResponse();
  }
}


void
setup()
{
  theUSBSerial.setup();
  Globals.setup();

  Serial.println();
  Serial.println("CABIN SETUP: BEGIN");

  if (Globals.remoteHeat()) {
    //    Serial.println("  REMOTE POWER:");
    theRemoteHeat.setup();
    Serial.println();
  }

  if (Globals.FD() || Globals.SD() || Globals.EW() || Globals.BW() ||
      Globals.KW() || Globals.LW()) {
    //    Serial.println("  REED SWITCHES:");
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
    //    Serial.println("  TEMP BUS:");
    theTempBus.setup();
    if (Globals.indoorTemp() && 
	!indoorThermometer.setup(theTempBus, 
				 TEMP_DEFAULT_INDOOR_LOW, 
				 TEMP_DEFAULT_INDOOR_HIGH)) {
      Globals.disable_indoorTemp();
    } 
    if (Globals.outdoorTemp() && 
	!outdoorThermometer.setup(theTempBus,
				  TEMP_DEFAULT_OUTDOOR_LOW,
				  TEMP_DEFAULT_OUTDOOR_HIGH)) {
      Globals.disable_outdoorTemp();
    }
  }

  if (Globals.cabinPower() || Globals.remotePower()) {
    //    Serial.println(" POWER DETECTORS:");
    if (Globals.cabinPower()) {
      cabinPower.setup();
      Serial.println();
    }
    if (Globals.remotePower()) {
      remotePower.setup();
      Serial.println();
    }
  }

  if (Globals.remoteHeat() && Globals.remotePower()) {
    theRemoteHeat.setPowerSensor(&remotePower);
  }

  if (Globals.motion()) {
    Serial.println("  Wait MOTION DETECTOR Initializing:");
    theMotionDetector.setup();
    Serial.println();
  }

  if (Globals.gprsModem()) {
    //    Serial.println("  GPRS MODEM");
    gprsModem.setup(4800, (char **)GPRSAuthorizedNumbers, processCmd);
    while (!gprsModem.isOn()) {
      gprsModem.TogglePower();
    }
    Serial.println();
  }

  if (Globals.Alarm()) {
    //    Serial.println("  ALARM");
    theAlarm.setup();
    Serial.println(); 
  }

  //  Globals.redLed.On();
  //  Globals.yellowLed.On();
  Serial.println("CABIN SETUP: END");

  if (Globals.cmdMode()) {
    Globals.EnterCmdMode();
    Serial.println("COMMAND MODE");
  } else {
    Globals.ExitCmdMode();
    Serial.println("TERMINAL MODE");
  }
}

void
loop()
{
  //Globals.set_now(millis());

  Globals.redLed.loopAction();
  Globals.yellowLed.loopAction();

  if (Globals.FD()) FD.loopAction();
  if (Globals.SD()) SD.loopAction();
  if (Globals.EW()) EW.loopAction();
  if (Globals.BW()) BW.loopAction();
  if (Globals.KW()) KW.loopAction();
  if (Globals.LW()) LW.loopAction();

  if (Globals.motion()) theMotionDetector.loopAction();

  if (Globals.remoteHeat()) theRemoteHeat.loopAction();

  if (Globals.indoorTemp() || Globals.outdoorTemp()) {
    boolean ready = theTempBus.loopAction();
    if (ready && Globals.indoorTemp()) indoorThermometer.loopAction(theTempBus);
    if (ready && Globals.outdoorTemp()) outdoorThermometer.loopAction(theTempBus);
  }

  if (Globals.cabinPower()) cabinPower.loopAction();
  if (Globals.remotePower()) remotePower.loopAction();

  theUSBSerial.loopAction();

  if (Globals.gprsModem()) gprsModem.loopAction();
  
  if (Globals.Alarm()) theAlarm.loopAction();
}



