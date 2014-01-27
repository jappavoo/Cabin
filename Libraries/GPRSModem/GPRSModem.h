#ifndef __GPRS_DEVICE_h__
#define __GPRS_DEVICE_h__

#define GPRSAUTHNUMLEN 15

#define GPRSBUFFERLEN _SS_MAX_RX_BUFF

extern char *smsIdxStrs[];

#define MAX_SMS_INDEXS 30

class GPRSModem : public SoftwareSerial {
  enum { MAX_AUTHORIZED_NUMBERS = 4 };
  char _pwrpin;
  char count;     // counter for buffer array
  
  char *_authorizedNumbers[MAX_AUTHORIZED_NUMBERS];
  char numUnAuthorized;
  void (*cmdFunc)(int, char *);

public:
 GPRSModem(char sspin1, char sspin2, char pwrpin) :
  SoftwareSerial(sspin1, sspin2), _pwrpin(pwrpin), count(0),
    numUnAuthorized(0), cmdFunc(NULL) {
    for (int i=0; i<MAX_AUTHORIZED_NUMBERS; i++) { _authorizedNumbers[i]=NULL; }
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

  // experimentally found that 250 milliseconds is long enough to
  // get back an OK response from the modem for a simple AT query
  // The expectation is that longer commands is taken care of by 
  // the maxTries 
  inline boolean waitForResponse(int wait=250, char maxTries=20) 
  {
    boolean rc;
    char cnt=0;
    do {
      delay(wait);
      rc = hasResponse();
      if (rc == TRUE) break;
      cnt++;
    } while (cnt < maxTries);
    if (rc==FALSE) Serial.println("ERROR: no Response");
    return rc;
  }

  uint32_t last4() { return _last4; }
  uint32_t responseTerminator() { return RESPONSE_TERMINATOR; }

  boolean isOn() {
    flush();
    println("AT");
    return waitForResponse();
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


  void turnOffMsgIndications() {
    println("AT+CNMI=0,0,0,0,0");
  }

  void turnOnMsgIndications() {
    println("AT+CNMI=2,1,0,0,0");
  }

  void MsgMemoryInfo() {
    println("AT+CPMS?");
    // reponds with +CPMS: "SM",18,30,"SM",18,30,"SM",18,30"
  }

  void TextModeSMS() {
    println("AT+CMGF=1");
  }

  void ListSMS() {
    println("AT+CMGL=\"ALL\"");
  }

  void EchoOff() {
    println( "ATE0" );
  }

  void EchoOn() {
    println( "ATE1" );
  }

  void ReadSmsStore( char * SmsStorePos ) {
    // Serial.print( "GprsReadSmsStore for storePos " );
    // Serial.println( SmsStorePos ); 
    print( "AT+CMGR=" );
    println( SmsStorePos );
  }

  void ReadSmsStore(uint8_t idx) {
    ReadSmsStore(smsIdxStrs[idx]);
  }
  
  void DeleteMsg(char *SmsStorePos ) {
    print("AT+CMGD=");
    println(SmsStorePos);
  }

  void DeleteMsg(uint8_t idx) {
    DeleteMsg(smsIdxStrs[idx]);
  }

  void TogglePower() {
    Serial.println("GPRSModem: Toggling Power");
    digitalWrite(_pwrpin,LOW);
    delay(1000);
    digitalWrite(_pwrpin,HIGH);
    delay(2000);
    digitalWrite(_pwrpin,LOW);
    delay(8000);
  }

  void openSMS(int i) {
    char inByte=0;
    //    Serial.print("AT+CMGS=");
    print("AT+CMGS=");
    //    GPRSSerial.println("\"+19175762236\"");
    println(_authorizedNumbers[i]);
    //    Serial.println(_authorizedNumbers[i]);
    while (inByte!=' ') {
      while (!available());
      inByte=read();
    }
    // reset count
    count=0;
  }

  void closeSMS() {
    unsigned char tmp=0x1A;
    //    Serial.println("closeSMS");
    write(&tmp,1); 
  }
  
  void setup(long baud, char *anums[], void (*cfptr)(int, char *)) {
    cmdFunc = cfptr;
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
    //    TogglePower();
  }

  boolean printShortStatus(Stream &s) 
  {
    if (numUnAuthorized>0) {
      s.print("UA-SMS:");
      s.print(numUnAuthorized, DEC);
      return TRUE;
    }
    return FALSE;
  }

  void printStatus(Stream &s) {
    s.print("UnAuthorized: ");
    s.print(numUnAuthorized, DEC);
  }

  char next(char i) {
    return i+1 % _SS_MAX_RX_BUFF;
  }

  uint8_t nextToken(uint8_t i, const char sep) {
    for (;i!=_receive_buffer_tail; i=next(i)) {
      if (_receive_buffer[i] == sep) return i;
    } 
    return -1;
  }

  void dumpBufferRaw() {
    uint8_t i;
    for (i=_receive_buffer_head; 
	 i!=_receive_buffer_tail; i=next(i)) {
      Serial.write(_receive_buffer[i]);
    }
  }

  void dumpBuffer() {
    uint8_t i;
    for (i=_receive_buffer_head; 
	 i!=_receive_buffer_tail; i=next(i)) {
      Serial.print(i,DEC);
      Serial.print(":");
      if (_receive_buffer[i]=='\r') Serial.print("\\r");
      else if (_receive_buffer[i]=='\n') Serial.print("\\n");
      else Serial.write(_receive_buffer[i]);
      Serial.print(" [0x");
      Serial.print(_receive_buffer[i], HEX);
      Serial.print("] ");
    }
    Serial.println();
  }

  char idx(char i) {
    if ((_receive_buffer_head + i) > _SS_MAX_RX_BUFF) 
      return i - (_SS_MAX_RX_BUFF - _receive_buffer_head);
    else
      return _receive_buffer_head + i;
  }

  int isAuthorized(char *str) {
    int8_t j;
    //    for (j=0; str[j]!=','; j++) { Serial.print(str[j]); }
    for (int i=firstAuthorizedIndex(); 
	 validAuthorizedIndex(i); 
	 i=nextAuthorizedIndex(i)) {
      //      Serial.print(i, DEC); Serial.print(": "); Serial.println(_authorizedNumbers[i]); 
      char *num = _authorizedNumbers[i];
      for (j=0; num[j]!=0 && num[j] == str[j]; j++);
      if (num[j]==0) return i;
    }
    return -1; 
  }

  void processMessages(int num) 
  {
    for (uint8_t i=1; i<=MAX_SMS_INDEXS; i++) {
      flush();
      ReadSmsStore(i);
      if (waitForResponse() == TRUE) {
	if (_receive_buffer[2] == '+') {
	  //	  Serial.print(i,DEC); Serial.print(":");
	  //Serial.print(num,DEC); Serial.print(":");
	  uint8_t idx = nextToken(14,',');
	  idx = nextToken(idx,',');
	  //	  for (int8_t j=1; _receive_buffer[idx+j]!=','; j++) { Serial.print(_receive_buffer[idx+j]); }
	  int anidx = isAuthorized((char *)(_receive_buffer+idx+1));
	  if (anidx != -1) {
	    if (cmdFunc != NULL) cmdFunc(anidx, (char *)_receive_buffer+nextToken(idx,'\n')+1);
	  } else {
	    //	    Serial.println(" UNAUTHORIZED SMS");
	    numUnAuthorized++;
	  }
	  flush();
	  DeleteMsg(i);
	  waitForResponse();
	  num--;
	}

      }
      if (num==0) break;
    } 
  }

  void loopAction() {
    if (!Globals.cmdMode()) {
      // Terminal mode input from serial goes to modem
      // and modem goes to 
      if (Globals.hasSerialByte==TRUE) write(Globals.serialInByte);
      while (available()) {
	Serial.write(read());
      }
    } else {
      // ignore any messages that come in async
      int cnt=0;
      flush();
      // check for new SMS
      MsgMemoryInfo();
      waitForResponse();
      //should responde with: '\r\n+CPMS: "SM",7,30,"SM",7,30,"SM",7,30"'
      sscanf((const char *)(_receive_buffer+14), "%d", &cnt);
      if (cnt) {
	//	Serial.println(cnt, DEC);
	processMessages(cnt);
      }
    }
  }

 
  boolean isAlarm() { return FALSE;  }
  void resetAlarm() {}
};

#endif
