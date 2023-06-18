# SLMX4
This project contains a program that interfaces with a SLMX4 sensor and etimates the breathing rate of a subject within range of the sensor.
This project is still under development.
Author: Vincent Lacasse

## Credits
This projet was initiated as part of and undergraduate course (ELE400, summer 2022) at École de Technologies Supérieure in Montréal. 
The original contributors are the members of the team ProtoPulmo: Tom Chow, Julian Davis, Alexis Macagno, Benoit Malenfant, Philippe Rochefort, Olivier St-Pierre Robillard.
Very little of the original code was kept, but there are good ideas that were reused, notablly: 
 - use of the serialib.cpp library for serial communication
 - use of the slmx4_vcom.cpp (totally rewritten) which was inspired from a MatLab version from SensorLogic.

The serialib.cpp library (V2.0, december the 27th of 2019) was written by Philippe Lucidarme (University of Angers).  
It had been modified by the students, but I replaced it with the original version from Philippe Lucidarme.

## Programs
- The 'showpsd' directory is my first attempt to sense respiration rate.  It's a serie of interactive programs that were used to develop an algorithm. The latest of which is showfreq4.cpp.  There are also python programs developped to visualize data.
- The 'breath' directory contains programs to sense respiration rate in real time. They use the showfreq4.cpp algorithm. breath2.cpp is the final version which communicates breath frequency data through OSC (for Max).
- The 'firstreflex' is a new and simpler algothithm than showfreq4.cpp to detect respiration rate (in devlepoment)
