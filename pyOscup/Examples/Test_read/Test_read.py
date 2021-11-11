'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2021 Daniel Rossi & Riccardo Salami
Version: ALPHA 1.2.0
'''

from time import sleep, time as t
import time
from random import random
import serial

port = 'COM4' #change COM port
baudrate = 115200 #change baudrate

MAX_ACK_WAIT = 300
RETRY_INTERVAL = 1
MAX_ATTEMPTS = 100
deviceID = 0xC4 # class implementation will be different

class packet_t:

    def __init__(self, id=None):
        self.id = id
        self.command = None
        self.length = None
        self.payload = None
        self.crc = None

        self.buff = None

    def reset(self):
        self.id = None
        self.command = None
        self.length = None
        self.payload = None
        self.crc = None

        self.buff = None

    def update(self, command: int, length: int, payload: bytearray):
        self.command = command
        self.length = length
        self.payload = payload
        self.__updateBuff()
        if self.buff != None:
            self.crc = self.computeCRC(self.buff)
            self.buff.extend(self.encodeInteger(self.crc, 2))

    def __updateBuff(self):
        self.buff = bytearray([self.id])
        self.buff.extend(bytearray([self.command]))
        self.buff.extend(bytearray([self.length]))
        self.buff.extend(self.payload)
        if self.crc != None:
            self.buff.extend(self.encodeInteger(self.crc, 2))

    def getcrc(self):
        return self.crc

    def getBuff(self):
        self.__updateBuff()
        return self.buff

    def getParams(self):
        return self.id, self.command, self.length, self.payload, self.crc

    @staticmethod
    def computeCRC(array: bytearray):
        '''funzione che calcola il CRC degli ultimi due byte'''
        assert array != None
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

    @staticmethod
    def encodeInteger(value: int, length: int):
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

def getTime():
    return time.time_ns() // 1_000_000

def write(command: int, length: int, payload: bytearray, ser: serial):

    # ID COMMAND LENGTH PAYLOAD CRC
    TX = packet_t(deviceID)
    TX.update(command, length, payload)
    if TX.getBuff() != None:
        print("WRITING " + "".join([str(hex(b)) + "     " for b in TX.getBuff()]))
        ser.write(TX.getBuff())

    RX = packet_t()

    cont = 0
    start_time = getTime()
    crc = random()

    data = b''
    while (crc != RX.getcrc() or cont == 0) and (getTime() - start_time <= MAX_ACK_WAIT) and (cont <= MAX_ATTEMPTS):
        RX.reset()

        while True:
            x = ser.read()
            if x != b'':
                data+= x[0:1]
            else:
                if data != b'' and len(data) > 5:
                    print("data: " + "".join([str(hex(b)) + "     " for b in data]))
                cont += 1
                if data[2] == 0xFF:
                    ser.write(TX.getBuff())
                data = b''
                break

ser = serial.Serial(port, baudrate,timeout=0,write_timeout=0)
cont = 0
while True:
    dummybuff = bytearray([1,2,3,4,5]) 
    write(0x01, 5, dummybuff, ser)
    sleep(1)
    #read(ser)
