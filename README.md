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

## How to use
Go the the showpsd directory and type 'make' to compile the programs.  
The src folder contains several programs that were developped incrementaly to validate research results.
At this time, the most up-to-date program is showfreq.cpp which interfaces with the slmx4 sensor, processes the signals for about 28 sec, and write final and intermediate results to file.
Several python (in the python folder) were developped to ease data visualisation, notably imshow.py. 
