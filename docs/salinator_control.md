# Functions
 - Turn main power for the salinator module
 - I2C master - Read & Set status of the Salinator keyboard/display module via one I2C bus
 - I2C slave - impersonate the Salinator's keyboard/display module
 - Hijack the Salinator's flow sensor signal - possibly the whole sensor to be working independently from the Salinator itself.
   - Looks like the sensor (Flow+) is connected to MCU through serial 1k resistor (R23)
   - The other pin of the sensor is connected direcly to GND -> Open when no flow, The Flow+ pin is grounded when flow is OK
   - Maybe add two diodes to avoid two-voltage-sources collision
   - Measure No-Flow voltage level on the Flow+ 

# Hardware
 -  Salinator: Intex 26670
 -  Keyboard uses the IC TM1650 
 
# Power requirements
 - to be determined

# Required I/O
| I/O Type | Usage | Type / Voltage / Memo |
|--------------|---------|------------------------------------|
|Digital Out |  DO_SalinatorPower  | optocoupler + Thyristor [max. designed load 200W] |
|Digital In | DI_SalinatorFlowSensor| |
|I2C-1 | Salinator main control board| |
|I2C-2 | Salinator keyboard/display module| |

## Media 

Keyboard from user view:
![Image](https://github.com/users/aelias-eu/projects/3/assets/71124636/e7d618a5-16a1-42ea-85a5-4f3f74433b55)

Keyboard inside:
![Image](https://github.com/users/aelias-eu/projects/3/assets/71124636/29ba8ac3-8dd1-4281-9bc4-fae8b8de6dd5)

Main control module:
![Image](https://github.com/users/aelias-eu/projects/3/assets/71124636/01419685-bad1-499e-9417-d8fa928ed8f7)

