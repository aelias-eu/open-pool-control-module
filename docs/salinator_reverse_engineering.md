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

Now we need to identify the which key sets which bit and which bit sets which segment/LED.

## Data identification



# Used tools
 - DVM
 - Oscilloscope
 - Logic analyzer [LHT00SU1](https://sigrok.org/wiki/Noname_LHT00SU1)
 - Software [PulseView](https://sigrok.org/wiki/PulseView)
 - Spreadsheet [LibreOffice Calc](https://www.libreoffice.org/discover/calc/)
