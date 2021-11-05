'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2021 Daniel Rossi & Riccardo Salami
Version: ALPHA 1.1.0
'''

from dataclasses import dataclass
import time
import serial
from random import random

MAX_PAYLOAD_LENGTH = 255

MAX_ACK_WAIT = 15
RETRY_INTERVAL = 5
MAX_ATTEMPTS = 10


@dataclass
class RxCommands:
    ACK = 0xFE
    NACK = 0xFF


@dataclass
class TxCommands:
    SHARE = 0x01
    CONFIRM = 0x02


@dataclass
class ErrorCodes:
    OK = 0
    LENGTH_ERROR = 0x01
    PACKMEMMOVE_ERROR = 0x02
    WRITE_ERROR = 0x03
    NULLPOINTER = 0x04
    ACK_TIMEOUT = 0x05
    CRC_ERROR = 0x06
    NACK = 0x07
    PACK_ERROR = 0x08
    NO_DATA = 0x09
    


class packet_t:
    '''this class defines the Oscup's packet and method for managing it'''

    def __init__(self, id=None):
        '''
        @brief initializes the packet_t class
        
        @param id of this device
        '''

        self.id = id
        self.command = None
        self.length = None
        self.payload = None
        self.crc = None

        self.buff_TX = None
        self.buff_RX = None


    def reset(self):
        '''
        @brief resets all the class attributes
        '''

        self.id = None
        self.command = None
        self.length = None
        self.payload = None
        self.crc = None

        self.buff_TX = None
        self.buff_RX = None


    def pack(self, command: int, length: int, payload: bytearray):
        '''
        @brief prepares data to be sent and obtains the crc
         
        @param command command to execute on the receiver
        @param length length of the payload
        @param buffer payload containing data 
        
        @return it returns feedback on writing result
        '''
        self.command = command
        self.length = length
        self.payload = payload
        self.__updateBuff()
        if self.buff_TX != None:
            self.crc = self.computeCRC(self.buff_TX)
            self.buff_TX.extend(self.encodeInteger(self.crc, 2))


    def __updateBuff(self):
        '''
        @brief updates the values inside the TX buffer
        '''
        self.buff_TX = bytearray([self.id])
        self.buff_TX.extend(bytearray([self.command]))
        self.buff_TX.extend(bytearray([self.length]))
        self.buff_TX.extend(self.payload)
        if self.crc != None:
            self.buff_TX.extend(self.encodeInteger(self.crc, 2))


    def setBuff(self, data: bytearray):
        '''
        @brief assings the read data to the RX buffer
        '''
        self.buff_RX = bytearray()
        self.buff_RX.extend(data)


    def unpack(self):
        '''
        @brief it unpacks data incoming from UART
        
        @param len it is the lenght of the received buffer
        '''
        if self.buff_RX[2] < 5:
            return

        self.id = self.buff_RX[0]
        self.command = self.buff_RX[1]
        self.length = self.buff_RX[2]
        for i in range(0, self.length):
            self.payload[i] = self.buff_RX[i + 3]
        self.crc = (self.buff_RX[self.length + 4] << 8) | self.buff_RX[self.length + 3]


    def getcrc(self):
        '''
        @brief returns the crc of the current packet
        '''
        return self.crc


    def getBuff(self):
        '''
        @brief returns the content of the buffer just updated
        '''
        self.__updateBuff()
        return self.buff_TX


    def getParams(self):
        '''
        @brief an all in one return which provides all the data about the packet
        '''
        return self.id, self.command, self.length, self.payload, self.crc


    @staticmethod
    def computeCRC(array: bytearray):
        '''
        @brief this function calculates the crc on ID, COMMAND, LENGTH, PAYLOAD

        @param array the buffer which contains all the needed parameters, in the right order

        @return it returs an uint16_t crc value
        '''
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
        '''
        @brief this function converts an integer value into an array

        @param value is the integer to convert
        @param length is the length of the final bytearray created

        @return the converted bytearray
        '''
        data = bytearray(length)

        if (length == 1):
            data[0] = value & 0xFF

        if (length == 2):
            data[0] = value & 0xFF
            data[1] = (value >> 8) & 0xFF

        if (length == 3):
            data[0] = value & 0xFF
            data[1] = (value >> 8) & 0xFF
            data[2] = (value >> 16) & 0xFF

        if (length == 4):
            data[0] = value & 0xFF
            data[1] = (value >> 8) & 0xFF
            data[2] = (value >> 16) & 0xFF
            data[3] = (value >> 24) & 0xFF
        
        return data


class Oscup:

    def __init__(self, id: int, baudrate: int, com_port: str):
        '''
        @brief initializes the Oscup class
        
        @param id of this device
        @param baudrate communication baudrate
        @param com_port the name of the virtual USB port
        '''

        self.id = id
        self.baudrate = baudrate
        self.intr_alloc_flags = 0
        self.serialWriter = serial.Serial(port=com_port, baudrate=baudrate, timeout=0, write_timeout=0)


    def write(self, command: int, length: int, payload: bytearray):
        '''
        @brief Writes data on Uart. 
                If ACK does not arrive, it will retry to send data again
                If NACK arrives, it will resend and delay the resend's stop time
         
        @param command it is command to execute on receiver
        @param length payload length
        @param payload the payload buffer
         
        @return it returns feedback on writing result
        '''

        TX = packet_t(self.id)
        TX.pack(command, length, payload)
        if TX.getBuff() != None:
            self.serialWriter.write(TX.getBuff())
        else:
            return ErrorCodes.NULLPOINTER

        RX = packet_t()

        cont = 0
        start_time = self.get_timer()
        crc = random()

        data = b''
        last_buff = b''
        while (crc != RX.getcrc() or cont == 0) and (self.get_timer() - start_time <= MAX_ACK_WAIT) and (cont <= MAX_ATTEMPTS):
            RX.reset()

            while True:
                x = self.serialWriter.read()
                if x != b'':
                    data+= x[0:1]
                else:
                    RX.setBuff(data)
                    RX.unpack()

                    cont += 1
                    _, command, _, _, _ =RX.getParams()
                    if command == RxCommands.NACK:
                        self.serialWriter.write(TX.getBuff())
                    elif command == RxCommands.ACK:
                        return ErrorCodes.OK

                    last_buff = data
                    data = b''
                    break
        
        if RX.getcrc() != packet_t.computeCRC(last_buff[:len(last_buff) - 2]):
            return ErrorCodes.CRC_ERROR

        return ErrorCodes.OK

            
    def get_timer(self):
        '''
        @brief returns a millisecond counter
        '''

        return time.time_ns() // 1_000_000
    

    def read(self):
        '''
        @brief this function reads data incoming from Serial and put them inside the packet.
        
        @return it returns feedback on writing result and the packet containing data
        '''

        packetTX = packet_t(self.id)
        packetRX = packet_t()

        data = b''
        while True:
            x = self.serialWriter.read()
            if x != b'':
                data+= x[0:1]
            else:
                if data != b'':
                    try:
                        actualcrc = data[len(data)-2:]

                        data_without_crc = data[:len(data) - 2]
                        crc = packet_t.encodeInteger(packet_t.computeCRC(data_without_crc), 2) 

                        if(crc == actualcrc):
                            dummybuff = bytearray([0,0,0,0,0])
                            packetTX.pack(RxCommands.ACK, 5, dummybuff) 
                            self.write(packetTX.getBuff())
                            break
                        else:
                            dummybuff = bytearray([0,0,0,0,0])
                            packetTX.pack(RxCommands.NACK, 5, dummybuff) 
                            self.write(packetTX.getBuff())
                            return ErrorCodes.CRC_ERROR, packetRX
                    except:
                        pass
                    finally:    
                        data = b''
                else:
                    dummybuff = bytearray([0,0,0,0,0])
                    packetTX.pack(RxCommands.NACK, 5, dummybuff) 
                    self.write(packetTX.getBuff())
                    return ErrorCodes.NO_DATA, packetRX
        
        packetRX.setBuff(data)
        packetRX.unpack()
        return ErrorCodes.OK, packetRX




    