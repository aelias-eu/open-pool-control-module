/*
 Demo code to communicate with the TM1650 based keyboard-led-board of Intex Chlorinator
  - sends data to the displays
  - read and decode pressed keys
*/

#include <Wire.h>

void setup() {
  Wire.begin(); // join I2C bus (address optional for master)
  Wire.setClock(100000);
  Wire.setWireTimeout(1000);
  Serial.begin(115200);
  Serial.println("Reset LED displays");
  Wire.beginTransmission(0x24); Wire.write(0x11); Wire.endTransmission();   // According to manual: It's the Command "Display ON, Brightness Level" 
  Wire.beginTransmission(0x34); Wire.write(0);    Wire.endTransmission();  
  Wire.beginTransmission(0x35); Wire.write(0);    Wire.endTransmission();    
  Wire.beginTransmission(0x36); Wire.write(0);    Wire.endTransmission(); 
  Wire.beginTransmission(0x37); Wire.write(0);    Wire.endTransmission();   
}

byte displayValue = 1;
byte displaySettings=0x31;
void loop() { 
  Wire.requestFrom(0x27,1,true);
  byte key=Wire.read();
  Wire.beginTransmission(0x27); 
  Wire.write(displaySettings); 
  Wire.endTransmission(true);  // Send setting (brightness & display on/off
  Wire.beginTransmission(0x34); Wire.write(displayValue);       Wire.endTransmission(true);  // Right LED Display
  Wire.beginTransmission(0x35); Wire.write(displayValue+32);    Wire.endTransmission(true);  // Left LED Display
  Wire.beginTransmission(0x36); Wire.write(displayValue+64);    Wire.endTransmission(true);   // Status LEDs
  Serial.print(displayValue);Serial.print("\t");
  Serial.print(displayValue+32);Serial.print("\t");
  Serial.print(displayValue+64);Serial.print("\n");
  //Wire.beginTransmission(0x37); Wire.write(displayValue);    Wire.endTransmission();  // This is unused
  //displayValue= displayValue << 1;
  displayValue++;
  if (displayValue>=24) {displayValue=1;}
  if ( ( key & 0x40 )== 0x40 )
   {
      Serial.print("Key value: ");
      switch (key & 0x3F){
        case 0x0E: Serial.println("[LOCK]");
                  break;
        case 0x0C: Serial.println("[POWER]");
                  break;
        case 0x06: Serial.println("[TIME]");
                  break;
        case 0x04: Serial.println("[BOOST]");
                  break;
        case 0x34: Serial.println("[SWAP]");
                  break;
        case 0x3C: Serial.println("[POWER+BOOST]");
                  break;
        default : 
              Serial.println((key & 0x3F));
              break;
      }
   }
  delay(40);
}
