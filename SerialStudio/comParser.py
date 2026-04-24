import serial
import time
import struct

# SETUP
COM_in = 'COM2'
COM_out_SerialStudio = 'COM8'
COM_out_terminal = 'COM10' 
terminal_marker1 = b'\xaa'
terminal_marker2 = b'\xbb'
sstudio_marker1  = b'\xcc'
sstudio_marker2  = b'\xdd'


# Input COM
serCOM_in = serial.Serial(COM_in, baudrate=115200)

# Output COM Serial Studio (Serial Studio reads on COM9)
serCOM_SerialStudio = serial.Serial(COM_out_SerialStudio, baudrate=115200)

# Output COM Terminal
serCOM_terminal = serial.Serial(COM_out_terminal, baudrate=115200)



buf = ''
msg = b''
while True:
    buf = serCOM_in.read(1)

    # Search for terminal data frame marker
    if buf == terminal_marker1:
        buf = serCOM_in.read(1)
        if buf == terminal_marker2:
            buf = serCOM_in.read(1)
            if buf == terminal_marker1:
                buf = serCOM_in.read(1)
                if buf == terminal_marker2:
                    while True:
                        # Read entire string message until \n is found
                        msg += serCOM_in.read(1)
                        if msg[-1] == 10: # == b'\n'
                            break
                    # SENT THAT SH*T
                    msg += '\r'.encode()
                    serCOM_terminal.write(msg)
                    msg = b''
    
    # Search for serial studio data frame marker
    if buf == sstudio_marker1:
        buf = serCOM_in.read(1)
        if buf == sstudio_marker2:
                buf = serCOM_in.read(1)
                if buf == sstudio_marker1:
                    buf = serCOM_in.read(1)
                    if buf == sstudio_marker2:
                        sstudio_data = serCOM_in.read(10)
                        v1 = struct.unpack('<H', sstudio_data[:2])[0]
                        v2 = struct.unpack('<f', sstudio_data[2:6])[0]
                        v3 = struct.unpack('<f', sstudio_data[6:10])[0]
                        serCOM_SerialStudio.write(f"{v1:d}, {v2:f}, {v3:f}\n".encode())
