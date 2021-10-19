import serial
from time import sleep
port = 'COM5' #change COM port
baudrate = 115200 #change baudrate

ser = serial.Serial(port, baudrate, timeout=0.01)
stringa = ''
while True:
    x = ser.read()
    if x != b'':
        stringa+= " "+str(int.from_bytes(x, "big"))
    else:
        print(stringa)
        #sleep(0.5)
        stringa = ''