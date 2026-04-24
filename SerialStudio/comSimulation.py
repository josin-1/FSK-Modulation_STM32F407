import serial
import numpy as np
import time
import struct

# SETUP
COM_out = 'COM1'
terminal_marker1 = 0xAA
terminal_marker2 = 0xBB
sstudio_marker1  = 0xCC
sstudio_marker2  = 0xDD

# Write to one end of the virtual pair
serCom = serial.Serial(COM_out, baudrate=115200)

f0 = 440
fs = 10000
A = 0.2

w0 = 2 * np.pi * f0 / fs
sine_coeff_0 = 2 * np.cos(w0)

sine_prev2 = np.sin(-2 * w0)
sine_prev1 = np.sin(-1 * w0)

samples_per_period = fs // f0
half_period = samples_per_period // 2

triangle_counter = 0
triangle_direction = 1  # 1 = going up, -1 = going down
triangle = -1.0

printCounter = 0

while True:
    # Match whatever format your Serial Studio JSON project expects
    # Example: CSV-style frame
    sine = sine_coeff_0 * sine_prev1 - sine_prev2
    sine_prev2 = sine_prev1
    sine_prev1 = sine

    adc_val = np.uint16((sine + 1.5) // (3/pow(2,12)))

    triangle += triangle_direction * (2.0 / half_period)  # step size to go from -1 to 1
    triangle_counter += 1
    
    if triangle_counter >= half_period:
        triangle_counter = 0
        triangle_direction *= -1  # flip direction
        
    data = struct.pack('<BBHff',
                       sstudio_marker1,
                       sstudio_marker2,
                       np.uint16(adc_val),
                       float(triangle),
                       float(-triangle))

    serCom.write(data)


    if printCounter == 1:
        data = struct.pack('<BB', terminal_marker1, terminal_marker2)
        data += "Msg START!\n".encode()
        serCom.write(data)
    if printCounter % 10 == 0:        
        data = struct.pack('<BB', terminal_marker1, terminal_marker2)
        data += "Hi\n".encode()
        serCom.write(data)
    if printCounter >= 100:
        printCounter = 0
        data = struct.pack('<BB', terminal_marker1, terminal_marker2)
        data += "Msg END!\n".encode()
        serCom.write(data)
    printCounter += 1

    time.sleep(0.05)
