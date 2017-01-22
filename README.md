# smarthome

The goal of the solution is to implement a heating control service that reads the temperature in a room and actuates valve commands to add heat to the room. The software consists of two projects: the actual HeatControl and a RoomSimulator for basic testing.

## HeatControl

The simple heating control service has to maintain a given target temperature (22°). It receives temperature readings and reacts with a valve command.

It is based on the libmosquitto C example program (I could not get the C++ binding to work, which I would have preferred.). For simplicity, most control is done in main_control.cpp, which uses global variables for the mosq struct and the strategy.

The strategy from TemperatureControlStrategy.h is instantiated in main_control.cpp and currently is just a simple binary strategy: If the temperature is below the target temperature, the valve is set to fully open. If it is above target temperature, it is set fully closed. This has the disadvantage, that there the temperature oscillates around the desired target temperature.
A different strategy could be implemented by subclassing the TemperatureControlStrategy class.

## RoomSimulator

To do some basic testing, RoomSimulator simulates a room with a temperature sensor and a valve. The room has a constant loss of temperature of 0.1 degrees/second. The heating with the valve fully open will heaten up the room by 1 degrees/second. (Of course this is a simplistic model.) Every second, mqtt tasks are done, the simulation is advanced, and the sensor readings are published.

## Technological choices

I chose C/C++ as it is the language I use most often. This probably has given me a late start as I had to manage the dependencies by myself (OpenSSL, pthreads, libmosquitto). 

To parse / write JSON files I added quickjson after a brief check online. It seems to be efficient and is header only.

## Next Steps

- test it "AS-IS" with a real room with temperature and heating
- apply a more complex strategy that tries to find a level that can be left more or less constant 
- extend the simulation by computing an error function as the sum of the the quadratic difference between target temperature and current temperature over time
- switching the project description from MSVisual Studio to CMAKE