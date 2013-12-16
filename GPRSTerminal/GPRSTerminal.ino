//Serial Relay - Arduino will patch a 
//serial link between the computer and the GPRS Shield
//at 19200 bps 8-N-1
//Computer is connected to Hardware UART
//GPRS Shield is connected to the Software UART 
 
#include <SoftwareSerial.h>
 
SoftwareSerial GPRS(7, 8);
unsigned char buffer[1024]; // buffer array for data recieve over serial port
int count=0;     // counter for buffer array 
void setup()
{
  GPRS.begin(9600);               // the GPRS baud rate   
//  GPRS.println("AT+IPR=4800");
//  GPRS.begin(4800);
  Serial.begin(19200);             // the Serial port of Arduino baud rate.
  Serial.println("GPRS SHIELD MONITOR: GPRS BAUD RATE: 9600");
}
 
void loop()
{
  if (GPRS.overflow()) Serial.println("\nOF\n");
  if (GPRS.available())              // if date is comming from softwareserial port ==> data is comming from gprs shield
  {
    while(GPRS.available())          // reading data into char array 
    {
      buffer[count++]=GPRS.read();     // writing data into array
      if(count == 1024)break;
    }
    Serial.write(buffer,count);            // if no data transmission ends, write buffer to hardware serial port
    count = 0;                       // set counter of while loop to zero
  }
  if (Serial.available())            // if data is available on hardwareserial port ==> data is comming from PC or notebook
    GPRS.write(Serial.read());       // write it to the GPRS shield
}

