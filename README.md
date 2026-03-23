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
The generated signal will be saved to a WAV-file with a hardcoded filename (msg.wav), set at line 11

## STM32
The STM32F407 will be programmed using STM32CubeIDE.<br>
**Work in Progress...**
