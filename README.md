# FSK-Modulation STM32F407 Discovery Board

This Group-Project was done for a DSP course during a Bachelors Degree in Applied Electronics.<br>

> ## Authors:
> - Johra-Markus Singh
> - Alexander Innerbichler
> - Sebastian Heide
> - Elias Lermann

## Goal
The goal of this Project is to generate a FSK Signal from an ASCII string with Matlab, feed the signal into a STM32F407 ADC, demodulate the signal and output it through UART on a PC-Terminal.<br>
The Matlab script contains a proof-of-concept demodulator, to test the working mechanism of it, before implementing it on the STM32F407.<br>
The resulting FSK-Signal uses a Preambel of 10101010!

## Matlab
The Matlab script is build on a dialog sequence.<br>
Choosing **Yes** on **Continous Messaging**, results in an endless loop to input strings, which will be outputted through the speaker/AUX (with added noise), and can be picked up by the STM32's ADC.<br>
**No** gives the possibility to enter one string, add noise to it, output it, and demodulate it.<br>
There are also animatedLine's implemented in the script, to visualize the demodulation.<br>
The generated signal will be saved to a WAV-file with a hardcoded filename (msg.wav), set at line 11<br>

Right now there are two possibile Demodulations implemented, one with a convolution, using conv(), and one with Dot-Products, and an IQ-Method. Both versions can be choosen, as part of the dialog sequence 

## STM32
The STM32F407 will be programmed using STM32CubeIDE.<br>
It is similiar to the Matlab script implemented in filter.h and filter.c, the calls for the filter interface are all done in the Timer Callback.<br>
The Demodulation is done with the Dot-Product method from the Matlab script, as the convolution took too long to calculate, and wasnt viable.<br>
To speed up the UART sending process, the float variables are sent as pure binary and then interpreted from the comParser.py<br>

## SerialStudio
The SerialStudio folder provides a debugging interface for windows. It needs Serial-Studio-GPL3 installed and com0com to be setup.<br>
startCOM.py sets up the whole debugging pipeline automatically:<br>
- setting COM port pairs with com0com and hub4com
- starting Serial-Studio, setting it up and connecting it to the right COM port
- starting PuTTy and connecting it to the COM port
- starting the comParser.py script (and if simulation=True, starting the comSimulation.py)

The comSimulation.py was just a debugging tool while programming the comParser.py and startCOM.py, to provide signals that were send over a COM port, without needing the STM32. It can be activated with setting "simulation" in startCOM.py to True.<br>
comParser.py reads in the data from the USB2TTL converter (hardcoded COM6 Port), parses it, and either sends the data over COM to the PuTTY terminal or the Serial Studio.<br>

These scripts are set up to work on a specific machine, and would need modifications to run on another machine (also it needs some PATH links to be set)