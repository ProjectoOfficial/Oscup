'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2021 Daniel Rossi & Riccardo Salami
Version: ALPHA 1.2.0
'''

import serial

port = 'COM3' #change COM port
baudrate = 115200 #change baudrate

LENNN = 45

def computeCRC(array: bytearray):
        '''funzione che calcola il CRC degli ultimi due byte'''
        crc = 0xFFFF

        for j in range(len(array)):
            byteValue = array[j]
            byteValue &= 0xff

            crc = (crc ^ byteValue) & 0xFFFF

            for _ in range(8):
                if (crc & 0x0001) != 0:
                    crc = (crc >> 1) ^ 49061
                else:
                    crc >>= 1

        return crc


def encodeInteger(value, length):
        ''' funzione che converte un intero in un array di bytes, utile per il crc'''
        data = bytearray(length)

        if (length == 1):
            data[0] = value & 0xFF
            return data

        if (length == 2):
            data[0] = value & 0xFF
            data[1] = (value >> 8) & 0xFF
            return data

        if (length == 3):
            data[0] = value & 0xFF
            data[1] = (value >> 8) & 0xFF
            data[2] = (value >> 16) & 0xFF
            return data

        if (length == 4):
            data[0] = value & 0xFF
            data[1] = (value >> 8) & 0xFF
            data[2] = (value >> 16) & 0xFF
            data[3] = (value >> 24) & 0xFF
            return data


def write(command: int, length: int, payload: str, ser: serial):
    deviceID = 0xC4 # class implementation will be different

    # ID COMMAND LENGTH PAYLOAD CRC
    array = bytearray(LENNN - 2)
    array[0] = deviceID
    array[1] = command
    array[2] = length
    for i in range(3, length):
        array[i] = payload[i - 3]
    crc = computeCRC(array)
    crcarray = encodeInteger(crc, 2)
    combined =array + crcarray
    print("answer: " + "".join([str(hex(b)) + " " for b in combined]))

    ser.write(array)


ser = serial.Serial(port, baudrate, timeout=0.01)
data = b''
while True:
    data = ser.read(LENNN)
    if data != b'':
        try:
            ''' extracts uint64_t from payload '''
            #value = struct.unpack('Q', data[3:len(data)-2])
            #print("value received: {}\n".format(value))
            
            ''' hex format '''
            #print()
            #print("raw data: {}".format(data))
            print("cleaned data: " + "".join([str(hex(b)) + " " for b in data]))
            #print()
            
            #print("data length:{}; last byte:{}".format(len(data), data[len(data )-2:]))
            actualcrc = data[LENNN-2:]

            newdata = data[:LENNN - 2]
            #print("new data: " + "".join([str(hex(b)) + " " for b in newdata]))
            crc = encodeInteger(computeCRC(newdata), 2) 
            #print("crc: " + "".join([str(hex(b)) + " " for b in crc]))
            #print("computed CRC:{}; actual CRC: {}".format(crc, actualcrc))
            print("\n")
            if(crc == actualcrc):
                print("they are equal, fine!")
                dummybuff = bytearray([0,0,0,0,0]) 
                write(0xFE, 5, dummybuff, ser)
                print("----------------------------------------------------")
            
            ''' hex just of the data we are converting'''
            #print("".join([str(hex(data[i])) + " " for i in range(3,len(data)-2)]))


            ''' sleep is not mandatory '''
            #sleep(0.5)
            
        except:
            pass
        data = b''