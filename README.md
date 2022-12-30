# SPL06-001
SPL06-001 library for Arduino processors

Arduino library for the SPL06-001 pressure sensor.
Derived from Adafruit_Sensor open-source library.
Code was developed and tested on an ESP32-S2 microcontroller.
Code was tested with I2C communication setup.

Provides Temperature in Celcius (default) and Fahrenheit. Though SPL06-001 is fairly innacurate for the temperature (0.5C).
Provides Pressure in Pascal (default), and kPa, millibar, Atmosphere, PSI, mm of Mercury, inches of Mercury.

Provides Barometric pressure corrected for sea level (aka Pressure Altitute). If no correction setting is provided, standard sea level standard day is used.
This corrected Pressure Altitute is useful for aviation purposes.
The correction setting value for Pressure Altitude can be provided either in:
   - mbar with the Pressure Altitude result in meters
   - inHg with the Pressure Altitude result in feet.

Note for non-aviators, the correction setting typically varies day-to-day and is provided by a local weather service. Typically an aviator will adjust their altimeter accordingly, both from the ground and/or during flight if needed, or when ATC or regulations requires it. For example some airspace or ATC would leave the setting to standard sea/standard day level, such as a flight in A airspace at 30,000 feet cruising altitude, for ease of communications and coordinations between all aircrafts. The setting is therefore mostly useful for General Aviation pilots (flying small planes locally). It can also be handy for small quadcopter operators or RC pilots if the flight relies on an altitude calculated from the pressure sensor.

Unadjusted Pressure Altitude can lead to being several feet or meters off, depending on the day.
A Pressure Altitude that is corrected for the time of use will be more precise and reliable than an altitude from a GPS sensor. Provided the sensor is also accurate enough and calibrated. But at least, for being more precise, it will provide data that is repeatable, and therefore more reliable.

Link to sensor datasheet:
https://datasheet.lcsc.com/lcsc/2101201914_Goertek-SPL06-001_C2684428.pdf
