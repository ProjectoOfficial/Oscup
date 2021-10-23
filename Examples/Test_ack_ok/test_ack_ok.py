import struct
from time import sleep
from pyOscup import pyOscup

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
                ''' extracts uint64_t from payload '''
                value = struct.unpack('Q', data[3:len(data)-2])
                print(value)

                ''' hex format '''
                #print("".join([str(hex(b)) + " " for b in data]))

                ''' hex just of the data we are converting'''
                #print("".join([str(hex(data[i])) + " " for i in range(3,len(data)-2)]))


                ''' sleep is not mandatory '''
                #sleep(0.5)
            except:
                pass
            data = b''