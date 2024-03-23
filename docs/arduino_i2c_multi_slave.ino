#include <Wire2.h>
// Wire2 extends the standard Wire2 with getLastAddr() function
// based on : https://github.com/alexisgaziello/TwoWireSimulator/issues/1#issuecomment-546332230
/*
 * Implementing i2c slave on arduino that reacts to multiple i2c addresses
 *
 *
*/
byte keyStatus;
byte arValuesFromMaster[20];
byte arAddresses[20];
byte  packetCounter=0;
bool packetComplete=false;
bool hasNewData=false;
unsigned long lastData;
byte readAddress;

void setup() {
  Serial.begin(115200);           // start serial for output
  packetCounter=0;
  packetComplete=false;
  Serial.print("Configuring I2C...");
  Wire.begin(0x24 | 0x34 | 0x35 | 0x36);        // join I2C bus with given slave addresses
  TWAMR = ( 0x24 | 0x34 | 0x35 | 0x36 ) << 1;   // set masking register to allow multiple addresses
  Wire.onRequest(requestEvent);   // register data request event
  Wire.onReceive(receiveEvent);   // register data write event
  Serial.println("DONE");
}

void loop() {
  unsigned long currTime=millis();
  if ((((currTime-lastData>1) && (packetCounter>3))&&hasNewData) || (packetCounter>15))
  {
    Serial.print("Counter");
    for (int i=0; i<packetCounter; i++)
    { Serial.print("\t"); Serial.print(i); }
    Serial.println("");
    Serial.print("Address");
    for (int i=0; i<packetCounter; i++)
    { Serial.print("\t"); Serial.print(arAddresses[i]); }
    Serial.println("");
    Serial.print("Value");
    for (int i=0; i<packetCounter; i++)
    { Serial.print("\t"); Serial.print(arValuesFromMaster[i]); }
    Serial.println("");
    Serial.print("Read from ");Serial.println(readAddress);
    packetCounter=0;                                                // this data is processed, reset counter
    hasNewData=false;
  }
}

// Receive write requests from main control board
// add write requests with addressess to the value & address buffer
void receiveEvent(int howMany) {
  lastData=millis();                              // this is used to identify end of packet
  hasNewData=true;
  arAddresses[packetCounter]=Wire.getLastAddr();  //i2c_addr;
  arValuesFromMaster[packetCounter]=Wire.read();  //val;
  packetCounter++;
}

// Response to keyboard read command
void requestEvent() {
  readAddress = TWDR >> 1;   // retrieve address from last byte on the bus
  Wire.write(76);            // 76=POWER, 70=TIME, 68=BOOST,  78=LOCK, 116=SWAP
}
