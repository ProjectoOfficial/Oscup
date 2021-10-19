import serial
import struct
from time import sleep
port = 'COM5' #change COM port
baudrate = 115200 #change baudrate

ser = serial.Serial(port, baudrate, timeout=0.01)
data = b''
while True:
    x = ser.read()
    if x != b'':
        data+= x[0:1]
    else:
        if data != b'':
            try:
                #value = struct.unpack('Q', data[2:len(data)-3])
                #print(value)
                #sleep(0.5)

                print("".join((["0x" + str(b) + " " for b in data])))
            except:
                pass
            data = b''