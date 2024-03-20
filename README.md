# Pool control module
Project stage: Planing / Information gathering

My goal is to create this device as open hardware so anyone can extend this according to his conditions and devices.

**Basic thoughts:**
 - Everything will go to an MQTT server and will be integrated into [Homeassistant](https://www.home-assistant.io/)
 - Drive all power pumps via frequency inverter
   - Soft ramp-up/down
   - Power control
 - Switch frequency inverters ON/OFF via Contactor or SSR to save energy if they are not used
 - Use "cheap" chlorinator/salinator and get his status
   - Low / High salt status
   - Flow sensor status
   - Error messages
 - Implement water heating management
 - Water quality measurement
   - pH probe
 - Water quality control
   - pH correction fluid injection  
 - Measure multiple temperatures - also the outside temperature to allow implementing some anti-freezing measures in Winter.
 - Control pool cover & detect it's state (for manual covering)
 - NTP client for time syncing
 - Time & Day-Of-Week rules - e.g. Do not add pH- between 10:00 and 20:00 @ weekend or between 12:00 and 20:00 at workdays
 - Maximize/optimize usage of solar energy when available

## Planned Feature Checklist
 - [ ]  Circulation management  [On/Off, Speed via Frequency inverter]
 - [ ]  Massage pump control [On/Off, Speed via Frequency inverter]
 - [ ]  Counter current control [On/Off, Speed via Frequency inverter]
 - [ ]  Salinator/Chlorinator support
   - [ ]  [Intex 26670](docs/salinator_control.md)
     - [x] [Reverse engineering](docs/Intex_26670_reverse_engineering.md)
     - [ ]  Implement communication interface     
 - [ ]  Heating management
 - [ ]  Temperature measurement
   -  [ ]  Technology room  temperature & humidity
   -  [ ]  Circulation water in temperature
   -  [ ]  Circulation water out temperature (after heater)
   -  [ ]  Outside temperature
- [ ] Water pH regulation
  - [ ]  pH measurement
    - [ ]  Calibration
    - [ ]  Measurement
  - [ ]  pH regulation
    - [ ]  liquid pH- compensation fluid dosing and injecting
