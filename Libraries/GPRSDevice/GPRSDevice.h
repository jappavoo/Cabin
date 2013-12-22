#ifndef __GPRS_DEVICE_h__
#define __GPRS_DEVICE_h__

#define GPRSBUFFERLEN _SS_MAX_RX_BUFF

class GPRSDevice : public SoftwareSerial {
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
};

#endif
