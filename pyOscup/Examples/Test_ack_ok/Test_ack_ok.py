import struct
from time import sleep
import serial

port = 'COM4' #change COM port
baudrate = 115200 #change baudrate


def computeCRC(array: bytearray):
        '''funzione che calcola il CRC degli ultimi due byte'''
        crc = 0xFFFF

        for j in range(len(array)):
            byteValue = array[j]
            byteValue &= 0xff

            tmpCrc = (crc ^ byteValue) & 0xFFFF

            for _ in range(8):
                if (tmpCrc & 0x0001) != 0:
                    tmpCrc >>= 1
                    tmpCrc ^= 49061
                else:
                    tmpCrc >>= 1
            crc = tmpCrc

        return crc


def encodeInteger(value, length):
        ''' funzione che converte un intero in un array di bytes, utile per il crc'''
        data = bytearray(length)

        if (length == 1):
            data[0] = (value >> 0) & 0xFF
            return data

        if (length == 2):
            data[0] = (value >> 0) & 0xFF
            data[1] = (value >> 8) & 0xFF
            return data

        if (length == 3):
            data[0] = (value >> 0) & 0xFF
            data[1] = (value >> 8) & 0xFF
            data[2] = (value >> 16) & 0xFF
            return data

        if (length == 4):
            data[0] = (value >> 0) & 0xFF
            data[1] = (value >> 8) & 0xFF
            data[2] = (value >> 16) & 0xFF
            data[3] = (value >> 24) & 0xFF
            return data



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
                #value = struct.unpack('Q', data[3:len(data)-2])
                #print("value received: {}\n".format(value))
                
                ''' hex format '''
                print()
                print("raw data: {}".format(data))
                print("cleaned data: " + "".join([str(hex(b)) + " " for b in data]))
                print()
                
                print("data length:{}; last byte:{}".format(len(data), data[len(data )-2:]))
                actualcrc = data[len(data)-2:]

                newdata = data[:len(data) - 2]
                print("new data: " + "".join([str(hex(b)) + " " for b in newdata]))
                crc = encodeInteger(computeCRC(newdata), 2) 
                print("crc: " + "".join([str(hex(b)) + " " for b in crc]))
                print("computed CRC:{}; actual CRC: {}".format(crc, actualcrc))

                ''' hex just of the data we are converting'''
                #print("".join([str(hex(data[i])) + " " for i in range(3,len(data)-2)]))


                ''' sleep is not mandatory '''
                #sleep(0.5)
            except:
                pass
            data = b''