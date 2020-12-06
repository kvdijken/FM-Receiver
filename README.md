# FM Receiver

In this project you will find my home built FM receiver. The goal is to create a receiver only. It delivers line level audio. It only needs an additional audio amplifier to be able to hear your favorite radio station from the speakers.

Performance is very good. I receive all FM radio stations with great clarity. Its sensitivity is 11𝜇V.

It lets you dial into preprogrammed radio stations, or to select frequencies yourself. Also you can change some internal receiver parameters such as intermediate frequencies and lowside/highside injection. 

The user interface runs on an Arduino Nano 328P. The receiver itself is built off discrete parts, no integrated parts are used.

In this project you will find
* Arduino source code of the digital section (in folder 'firmware/FM Radio 5.8')
* Kicad project with the schematic and pcb layout of the analog section (in folder 'hardware')
* documentation (in folder 'documentation')
* a Python project to model differential pair amplifiers (in folder 'IF Amplifier')
* some photos of my implementation
* measurements I have done on this receiver (in folder 'measurements').

In this project you will not find the schematic for the digital section yet. The digital section is not complicated however. It consists of an Arduino Nano, three pushbuttons, one LED, 1 rotary encoder, one display and one Adafruit Si5351 breakout board. From the Arduino sourcecode you should be able to discover the wiring. The Si5351 delivers the two local oscillator frequencies which are needed for the receiver.

Keep in mind that the schematic, and the pcb which is derived from the schematic contain an serious error. This error is described in the documentation. Also is described how to work around the error. You can still use the pcb design, even with this bug. in the next version of this receiver this error will be solved.

In the next few weeks, months, my main focus will be on measurements on the receiver. I will try to characterise the receiver as well as possible. From there the receiver can be improved.

Current version is 1.0

Feel free to ask/contribute.



> Written with [StackEdit](https://stackedit.io/).