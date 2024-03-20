# Hardware
| | |
|----|----|
|Device type|Chlorinator / Salinator / Salt water chlorine generator|
|Manufacturer|Intex|
|Model|26670|
|URL|https://www.intexdevelopment.com/support/26670/productdetail?lst_lang=2|

![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/53268200-36d2-4510-a99d-1985ac8555df)


The device has two PCBs (Printed Circuit Board). Let's call them the Control-Board (CB) and the Keyboard-Display-Board (KDB).
Quick look at the Control-Board reveals that it is using an control chip [HT66F0185](https://datasheet.lcsc.com/lcsc/1810010214_Holtek-Semicon-HT66F0185_C81481.pdf) by Holtek, Which is a general purpose A/D MCU. But there is another interesting thing and that's the connection to the Keyboard-Display-Board:
![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/3642bd01-eb4b-4c7a-8ae5-577d9a6d2d70)

So - it looks like we have an I2C bus between CB and KDB.
Let's look at the Keyboard-Display-Board:
![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/71c18893-f580-4531-bad6-c72e5386f779)

There is only one chip - the TM1650. According to it's [datasheet](https://github.com/maxint-rd/TM16xx/blob/master/documents/LED%20driver%20TM1650%20v2.1%20EN.pdf), it's a LED driver with Keyboard scanning interface.  Well - it does not say that it is using I2C, but it calls that 2-wire interface with CLD and DAT. According to charts in the datasheet, I would say, it's an I2C bus, so let's analyze this further.

# I2C
Using an DSO (Digital Signal Oscilloscope) we will do a preliminary measurement of timing (so we can identify the I2C bitrate) and also the know the voltage levels.

## Timing
Approx 9ms pause between transmissions
![Image](https://github.com/users/aelias-eu/projects/3/assets/71124636/0cd080c4-25f0-442c-ac6b-393191f060ae)

SCL Pulse width approx.  28.8us
![Image](https://github.com/users/aelias-eu/projects/3/assets/71124636/18cf7536-315f-4e62-bf81-332fc88fc793)

## Voltage level
The I2C on this device uses 5V voltage level -> **do not connect directly to the ESP32!!**
Use Voltage Level shifting.

### Voltage Level shifting
![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/3109b3a3-3bf7-42cd-88b1-d8994b045582)
*Source: Level shifting techniques in I2C-bus design, AN10441, NXP Semiconductors, url: https://cdn-shop.adafruit.com/datasheets/AN10441.pdf*

## Communication Protocol
Now, we will connect a Logic analyzer to that bus and sniff something :)  
I'm using the [LHT00SU1](https://sigrok.org/wiki/Noname_LHT00SU1) with [PulseView](https://sigrok.org/wiki/PulseView)

![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/1b440afc-2610-4e66-b283-141ffc31b929)

The acquired data looks like this:
![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/57479c8d-6449-4e1f-93ec-af6544aaf55d)
Acquired data for state: Pressed Buttons: [TIME], LED Displays shows "04", All standalone LED's are off.

I did multiple acquisitions with various states of LED Displays , LED's and Keys pressed. 
There were following requests:
 * Read from address 0x27
 * Write to address 0x24
 * Write to address 0x34
 * Write to address 0x35
 * Write to address 0x36

But if we look at the datasheet, there are different addresses:
* Display Addresses: 0x68, 0x6A, 0x6C, 0x6E
* Command1 : 0x48
* Read key command 0x4F

The addresses do not match... Why?
Let's look again at the datasheet - the Read key sequence and our acquired data:
![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/0f700216-7649-4797-a79b-fbb722c70cb9)
We can see, that in the datasheet, they are using 8bit *command* and the I2C standard is using 7bit address+0/1 for write or read.
Let's do a bit of binary conversions
|Hex Value from I2C|7 bit binary representation|
|---|---|
|0x27|0100111|

Now, we will shift this to the left and add "1" as first bit. That would make b'01001111', which is 0x4F - And that is the **Read key command** according to the datasheet. For the "write" commands we should do SHL and add "0" - so just the SHL.

|Hex Value from I2C|7 bit binary representation|SHL+Add Read/Write bit"|Transformed "Command"|
|---|---|---|---|
|0x27|0100111|01001111|0x4F|
|0x24|0100100|01001000|0x48|
|0x34|0110100|01101000|0x68|
|0x35|0110101|01101010|0x6A|
|0x36|0110110|01101100|0x6C|

So this mystery is solved. It is an I2C but it's documented like it's not :) We just have to keep that in mind when we try to communicate with that.

Running an I2C scan example code ( Arduino IDE -> File - > Examples -> Wire -> i2c_scanner ) on Arduino Nano against this device shows following:

```
19:31:37.655 -> Scanning.
19:31:37.655 -> I2C device at address 0x24  !
19:31:37.688 -> I2C device at address 0x25  !
19:31:37.721 -> I2C device at address 0x26  !
19:31:37.754 -> I2C device at address 0x27  !
19:31:37.788 -> I2C device at address 0x34  !
19:31:37.821 -> I2C device at address 0x35  !
19:31:37.854 -> I2C device at address 0x36  !
19:31:37.887 -> I2C device at address 0x37  !
19:31:37.887 -> done
19:31:37.887 -> 
```
But for now we are interested only in the addresses 0x24, 0x34, 0x35 and 0x36 that were identified from the communication inspection.

Now we need to identify which key sets which bit and which bit sets which segment/LED.

## Data identification
I have used Arduino nano to send test commands via I2C because of the 5V level compatibility. With Voltage level shifter, you could use the ESP...

Arduino code to test the LED outputs and keyboard in:
```c++
/*
 Send data to the display and LEDS of the TM1650 based LED/Keyboard board from Intex Chlorinator
*/

#include <Wire.h>

void setup() {
  Wire.begin(); // join I2C bus (address optional for master)
  Serial.begin(115200);
  Serial.println("Reset LED displays");
  Wire.beginTransmission(0x24); Wire.write(0x11); Wire.endTransmission();   // According to manual: It's the Command "Display ON, Brightness Level" 
  Wire.beginTransmission(0x34); Wire.write(0);    Wire.endTransmission();  
  Wire.beginTransmission(0x35); Wire.write(0);    Wire.endTransmission();    
  Wire.beginTransmission(0x36); Wire.write(0);    Wire.endTransmission(); 
  Wire.beginTransmission(0x37); Wire.write(0);    Wire.endTransmission();   
}

byte displayValue = 1;
byte displaySettings=0x11;
void loop() { 
  Serial.print("Output data to 0x24: ");Serial.print(displaySettings);
  Serial.print(", Display value: "); Serial.println(displayValue);
  Wire.requestFrom(0x24,1);
  byte key=Wire.read();
  Wire.beginTransmission(0x24); Wire.write(displaySettings); Wire.endTransmission();  // Send setting (brightness & display on/off
  //Wire.beginTransmission(0x34); Wire.write(displayValue);    Wire.endTransmission();  // Right LED Display
  //Wire.beginTransmission(0x35); Wire.write(displayValue);    Wire.endTransmission();  // Left LED Display
  Wire.beginTransmission(0x36); Wire.write(displayValue);    Wire.endTransmission();   // Status LEDs
  //Wire.beginTransmission(0x37); Wire.write(displayValue);    Wire.endTransmission();  // This is unused
  displayValue= displayValue << 1;
  if (displayValue==0) {displayValue=1;}
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

  delay(2000);
}
```
This is the outcome:
What they call in the datasheet Command 1 and Command 2, means: via I2C Send Data "Command2" to address 0x24:

Binary data explanation  of Command2 a.k.a. System Command:
![image](https://github.com/aelias-eu/open-pool-control-module/assets/71124636/80b40237-544a-4c44-95a7-5363728fda2c)

### Possible problems
The chip acts not as one I2C slave, but as 4 slaves with addresses 0x24,0x34,0x35,0x36 (in reality there are more addreses the chip is using, but these 4 are interesting for this case).

ToDo:
 - [ ] **Find out if it is possible on the ESP to emulate more than 1 slave on a single I2C interface.** It is possible on Arduino  [Example](https://github.com/alexisgaziello/TwoWireSimulator) via address masking and also on some other chips, but it is a non-standard approach. Maybe this will need a saparate module for the Salinator/Chlorinator communication.


# Used tools
 - DVM
 - Oscilloscope
 - Logic analyzer [LHT00SU1](https://sigrok.org/wiki/Noname_LHT00SU1)
 - Software [PulseView](https://sigrok.org/wiki/PulseView)
 - Spreadsheet [LibreOffice Calc](https://www.libreoffice.org/discover/calc/)
 - Arduino Nano
