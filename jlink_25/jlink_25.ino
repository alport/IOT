/*
 Created by Mike Alport
 AIT 2 March, 2014 
 NB mqtt has BOTH a sensorID =25 and a nodeID=25!
 Receives Packet from serial (RPi) and sends it out 
 Serial Packet has 5 bytes: sourceID,destID,DataLo,DataHi,crcVal
 Received RF: sourceID destID dataLo dataHi crcVal (all bytes)
 
 Receives RF Packet from node and sends it to serial for publishing.
 SErial has format: sourceID,destID,dataLo,DataHi,CRCbyte<cr>
 
 
 NB sending from RFdemo12: 3,25,33,44,55,xxs   to nodexx without ackn
 3,25,33,44,55,xxa   to nodexx with ackn
 https://github.com/jcw/jeelib/tree/master/examples/RF12/RF12demo
 Jeenode on ...V1
 JeeLink on ...VW
 */

#include <JeeLib.h>
#define LED 9   // activity lED
byte mqttID=25;  //  Master Node
byte destID=1;   // to be changed
int data;
boolean debug=false;
byte payload [5];

const int NUMBER_OF_FIELDS = 5; // how many comma separated fields we expect
int fieldIndex = 0;            // the current field being received
int values[NUMBER_OF_FIELDS];   // array holding values for all the fields

void setup()
{
  pinMode(LED, OUTPUT);  // a busy indicator pulse
  Serial.begin(57600); // Initialize serial port to send and receive at 9600 baud
  // node 25, group 10, 868 MHz
  rf12_initialize(mqttID, RF12_868MHZ, 10);
  //Serial.println('jlink running...'); 
}

void sendPayload(){
  rf12_sendNow(0, &payload, sizeof payload);  // 0 indicates broadcast!!
}

void recv(){
  if (rf12_recvDone()) {
    byte n = rf12_len;   // could put a check on n as well??
    if (rf12_crc == 0){
      if (rf12_data[1] == mqttID){  // check if this is for Master
        for (byte i = 0; i < n-1; ++i) { // send to the RPi        
          Serial.print(rf12_data[i]);
          Serial.print(',');
        }
        Serial.print(rf12_data[n-2]);
        Serial.println();
      }
    }
    if(RF12_WANTS_ACK){
      rf12_sendStart(RF12_ACK_REPLY,0,0);
    }
  }
}


void loop()
{
  if( Serial.available())
  {
    char ch = Serial.read();
    if(ch >= '0' && ch <= '9') // is this an ascii digit between 0 and 9?
    {
      // yes, accumulate the value
      values[fieldIndex] = (values[fieldIndex] * 10) + (ch - '0'); 
    }
    else if (ch == ',')  // comma is our separator, so move on to the next field
    {
      if(fieldIndex < NUMBER_OF_FIELDS-1)
        fieldIndex++;   // increment field index
    }
    else
    {
      // any character not a digit or comma ends the acquisition of fields
      // in this example it's the newline character sent by the Serial Monitor
      payload[0]= values[0];
      payload[1]=values[1];
      payload[2]=values[2];
      payload[3]=values[3];
      payload[4]=values[4];   // optional for later expansion
      sendPayload();
      for(int i=0; i <= fieldIndex; i++)
      {
        values[i] = 0; // set the values to zero, ready for the next message
      }
      fieldIndex = 0;  // ready to start over
    }
  }
  recv();

}












































