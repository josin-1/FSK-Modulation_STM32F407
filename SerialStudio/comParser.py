import serial
import time
import math

# Input COM
serCom2 = serial.Serial('COM2', baudrate=115200)

# Output COM
serCom8 = serial.Serial('COM8', baudrate=115200)
serCom10 = serial.Serial('COM10', baudrate=115200)

buf = ''
while True:
    buf += serCom2.read().decode('ascii')
    if buf[-1] == '\n':
        if buf.split(':')[0] == 'term':
            #print(buf.split(':')[1])
            serCom8.write((buf.split(':')[1]).encode())
            buf = ''
        if buf.split(':')[0] == 'adc':
            #print(buf.split(':')[1])
            serCom10.write((buf.split(':')[1]).encode())
            buf = ''
