# Electrathon Project
An Arduino project for controlling the electrathon car

## Arduino Code

### Objects

* Current Sensing

    * ADS 1152 ADC (i2c

* Throttle Control (0 stop, 180 full speed)

    * Uses Current Sensing and Speed Sensing

    * Startup sequence arms the controller

    * MCP 4725 - DAC, i2c 12bit

    *

* Speed Sensing (Tach / Speed)

    * Wheel circumference and rotation sensor

