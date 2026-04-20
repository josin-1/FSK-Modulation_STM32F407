import serial
import time
import math

f0 = 1
A = 0.2

# Write to one end of the virtual pair
serCom1 = serial.Serial('COM1', baudrate=115200)

t = 0
printCNT = 0
BitCNT = 0

while True:
    # Match whatever format your Serial Studio JSON project expects
    # Example: CSV-style frame
    adcVal = round((math.sin(2 * math.pi * f0 * t) * A + 1.5) * (4096/3), 0)

    line = f"adc:{adcVal}\r\n"
    serCom1.write(line.encode())

    BitCNT += 1
    if BitCNT >= 30:
        BitCnt = 0
        serCom1.write("term:Hello\r\n".encode())


    printCNT += 1
    if printCNT == 50:
        string = "t: " + str(t) + " | " + "adcVal: " + str(adcVal)
        line = "string:" + string + "\n"
        serCom1.write(line.encode())
    t += 0.01
    time.sleep(0.05)  # 20 Hz