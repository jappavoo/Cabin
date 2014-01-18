#ifndef __GPRS_DEVICE_h__
#define __GPRS_DEVICE_h__

#define GPRSAUTHNUMLEN 15

#define GPRSBUFFERLEN _SS_MAX_RX_BUFF

extern char *smsIdxStrs[];

class GPRSModem : public SoftwareSerial {
  enum { MAX_AUTHORIZED_NUMBERS = 4 };
  char _pwrpin;

  char buffer[GPRSBUFFERLEN]; // buffer array for data recieve over serial port
  char count;     // counter for buffer array
  
  boolean line;
  boolean authorizedMsg;
  boolean SMS;
  char    smsCnt;
  char    smsIdx;
  
  char *_authorizedNumbers[MAX_AUTHORIZED_NUMBERS];
  char numUnAuthorized;
  char from;
public:
 GPRSModem(char sspin1, char sspin2, char pwrpin) :
  SoftwareSerial(sspin1, sspin2), _pwrpin(pwrpin), count(0),
    line(false), authorizedMsg(false), SMS(false), smsCnt(0), smsIdx(0),
    numUnAuthorized(0), from(-1) {
    for (int i=0; i<MAX_AUTHORIZED_NUMBERS; i++) { _authorizedNumbers[i]=NULL; }
    buffer[0]=0;
  }
    
  int registerAuthorizedNumber(char *num) {
    for (int i=0; i<MAX_AUTHORIZED_NUMBERS; i++) {
      if (_authorizedNumbers[i]==NULL) {
	_authorizedNumbers[i] = num;
	return i;
      }
    }
    return -1;
  }

  unsigned int firstAuthorizedIndex() {
    for (int i=0; i<MAX_AUTHORIZED_NUMBERS; i++) {
      if (_authorizedNumbers[i]!=NULL) {
	return i;
      }
    }
    return -1;    
  }

  unsigned int nextAuthorizedIndex(unsigned int cur) {
    cur++;
    for (int i=cur; i<MAX_AUTHORIZED_NUMBERS; i++) {
      if (_authorizedNumbers[i]!=NULL) {
	return i;
      }
    }
    return -1;    
  }

  boolean validAuthorizedIndex(unsigned int i) {
    return (i>=0 && i<MAX_AUTHORIZED_NUMBERS);
  }

  char * AuthorizedNumber(int i) {
    if (i>=0 && i<MAX_AUTHORIZED_NUMBERS) return _authorizedNumbers[i];
    return NULL;
  }

  void MsgMemoryInfo() {
    println("AT+CPMS?");
    // reponds with +CPMS: "SM",18,30,"SM",18,30,"SM",18,30"
  }

  void TextModeSMS() {
    println( "AT+CMGF=1" );
  }

  void EchoOff() {
    println( "ATE0" );
  }

  void EchoOn() {
    println( "ATE1" );
  }

  void ReadSmsStore( String SmsStorePos ) {
    // Serial.print( "GprsReadSmsStore for storePos " );
    // Serial.println( SmsStorePos ); 
    print( "AT+CMGR=" );
    println( SmsStorePos );
  }

  void ReadSmsStore() {
    ReadSmsStore(smsIdxStrs[(int)smsIdx]);
  }
  
  void DeleteMsgs() {
    println("AT+CMGD=1,4");
  }

  void TogglePower() {
    digitalWrite(_pwrpin,LOW);
    delay(1000);
    digitalWrite(_pwrpin,HIGH);
    delay(2000);
    digitalWrite(_pwrpin,LOW);
    delay(3000);
  }

  void openSMS(int i) {
    char inByte=0;
    print("AT+CMGS=");
    //    GPRSSerial.println("\"+19175762236\"");
    println(_authorizedNumbers[i]);
    while (inByte!=' ') {
      while (!available());
      inByte=read();
    }
  }

  void closeSMS() {
    unsigned char tmp=0x1A;
    write(&tmp,1); 
  }

  void SendStatus(boolean from, boolean alarm) 
  {
#if 0
    char tmp[20];
    char inByte=0;
    
    for (int i=0; GPRSAuthorizedNumbers[i]!=0; i++) {
      if (fromflag == false || from == i) { 
#ifdef DEBUG
	Serial.print("Sending Status to : ");
	Serial.println(GPRSAuthorizedNumbers[i]);
#endif
	print("AT+CMGS=");
	//    GPRSSerial.println("\"+19175762236\"");
	println(GPRSAuthorizedNumbers[i]);
	while (inByte!=' ') {
	  while (!available());
	  inByte=_read();
	}
#ifdef DEBUG
	Serial.println("READY TO SEND STATUS");
#endif    
	printShortStatus(this);
    
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
#endif
  }
  
  void DoSMSCmd() 
  {
#if 0
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
  
  char NextIndex(char i) 
  {
    for (;i<count; i++) {
      if (buffer[i]==',') break;
    }
    return i;
  }

  void ProcessLine() 
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
#endif
  }
  
  void setup(long baud, char *anums[]) {
    for (int i=0; anums[i]!=0; i++) {
      if (registerAuthorizedNumber(anums[i])==-1) {
	Serial.print("ERROR: GPRSModem: unable to register: ");
	Serial.println(anums[i]);
      }
    }
    numUnAuthorized=0; 
    Serial.println("GPRS AUTHORIZED NUMBERS:");
    for (int i=firstAuthorizedIndex(); 
	 validAuthorizedIndex(i); 
	 i=nextAuthorizedIndex(i)) {
      Serial.println(_authorizedNumbers[i]);
    }
    pinMode(_pwrpin, OUTPUT); 
    digitalWrite(_pwrpin,LOW);
    begin(baud);               // the GPRS baud rate   
    Serial.print("GPRS SHIELD MONITOR: GPRS BAUD RATE:");
    Serial.println(baud);
    TogglePower();
  }

  boolean printShortStatus(Stream &s) {
    return FALSE;
  }

  void printStatus(Stream &s) {
  }

  void loopAction() {
#if 0
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
#endif
  }
  boolean isAlarm() { return FALSE;  }
  void resetAlarm() {}
};

#endif
