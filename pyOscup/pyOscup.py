'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2022 Daniel Rossi
Version: 1.2.4
'''

from dataclasses import dataclass
import time
import serial
from random import random

FIX_PACKET_LENGTH = 45
MAX_PAYLOAD_LENGTH = FIX_PACKET_LENGTH - 5

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
    UNPACK_ERROR = 0x10
    


class packet_t:
    '''this class defines the Oscup's packet and method for managing it'''

    def __init__(self, id=0):
        '''
        @brief initializes the packet_t class
        
        @param id of this device
        '''

        self.id = id
        self.command = 0
        self.length = 0
        self.payload = bytearray(MAX_PAYLOAD_LENGTH)
        self.crc = 0

        self.buff = bytearray(FIX_PACKET_LENGTH)


    def reset(self):
        '''
        @brief resets all the class attributes
        '''

        self.id = 0
        self.command = 0
        self.length = 0
        self.payload = bytearray(MAX_PAYLOAD_LENGTH)
        self.crc = 0

        self.buff = bytearray(FIX_PACKET_LENGTH)


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
        if self.buff != None:
            self.crc = self.computeCRC(self.buff)
            self.buff.extend(self.encodeInteger(self.crc, 2))


    def __updateBuff(self):
        '''
        @brief updates the values inside the TX buffer
        '''
        self.buff = bytearray([self.id])
        self.buff.extend(bytearray([self.command]))
        self.buff.extend(bytearray([self.length]))
        self.buff.extend(self.payload)
        if(len(self.payload) < MAX_PAYLOAD_LENGTH):
            self.buff.extend(bytearray([0] * (MAX_PAYLOAD_LENGTH - len(self.payload))))

    def unpack(self, buffer):
        '''
        @brief it unpacks data incoming from UART
        
        @param len it is the lenght of the received buffer
        '''
        if buffer == None or len(buffer) < 3:
            return ErrorCodes.UNPACK_ERROR

        if buffer[2] < 5:
            return ErrorCodes.UNPACK_ERROR
        
        self.buff = buffer

        self.id = buffer[0]
        self.command = buffer[1]
        self.length = buffer[2]
        for i in range(0, self.length):
            self.payload[i] = buffer[i + 3]
        self.crc = (buffer[FIX_PACKET_LENGTH - 1] << 8) | buffer[FIX_PACKET_LENGTH - 2]
        return ErrorCodes.OK


    def getcrc(self):
        '''
        @brief returns the crc of the current packet
        '''
        return self.crc


    def getBuff(self):
        '''
        @brief returns the content of the buffer
        '''
        return self.buff


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

    
    @staticmethod
    def translate_bytes(array: bytearray):
        print(''.join("{} ".format(hex(x)) for x in array))


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
        self.serialWriter = serial.Serial(port=com_port, baudrate=baudrate, timeout=0.01)
        

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
        if TX.getBuff() is not None:
            self.serialWriter.write(TX.getBuff())
        else:
            return ErrorCodes.NULLPOINTER

        RX = packet_t()

        start_time = self.get_timer()
        crc = random()
        times = 0

        while True:
            RX.reset()

            buff = self.serialWriter.read(FIX_PACKET_LENGTH)
            if buff != b'' and len(buff) == FIX_PACKET_LENGTH:
                try:
                    RX.unpack(buff)

                    _, command, _, _, _ = RX.getParams()
                    
                    crc = packet_t.computeCRC(buff[:FIX_PACKET_LENGTH - 2])
                    if crc == RX.getcrc() :
                        if command == RxCommands.ACK:
                            break
                        else:
                            return ErrorCodes.NACK
                    else:
                        return ErrorCodes.CRC_ERROR
                    
                except Exception as e:
                    print(str(e))
                    pass
                finally:    
                    buff = b''

            if self.get_timer() - start_time <= MAX_ACK_WAIT:
                return ErrorCodes.NO_DATA

            if times > MAX_ATTEMPTS:
                return ErrorCodes.NO_DATA
            times += 1

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

        times = 0

        while True:
            buff = self.serialWriter.read(FIX_PACKET_LENGTH)
            if buff != b'' and len(buff) == FIX_PACKET_LENGTH:
                try:
                    error = packetRX.unpack(buff)
                    if error == ErrorCodes.UNPACK_ERROR:
                        continue
                    
                    crc = packet_t.computeCRC(buff[:FIX_PACKET_LENGTH - 2])
                    if(crc == packetRX.getcrc()):
                        dummybuff = bytearray([0,0,0,0,0])
                        packetTX.pack(RxCommands.ACK, 5, dummybuff) 
                        self.serialWriter.write(packetTX.getBuff())
                        break
                    else:
                        dummybuff = bytearray([0,0,0,0,0])
                        packetTX.pack(RxCommands.NACK, 5, dummybuff) 
                        self.serialWriter.write(packetTX.getBuff())
                        return ErrorCodes.CRC_ERROR, packetRX
                except Exception as e:
                    print(str(e))
                    pass
                finally:    
                    buff = b''
            else:
                dummybuff = bytearray([0,0,0,0,0])
                packetTX.pack(RxCommands.NACK, 5, dummybuff) 
                self.serialWriter.write(packetTX.getBuff())
                return ErrorCodes.NO_DATA, packetRX
            
            if times > MAX_ATTEMPTS:
                return ErrorCodes.NO_DATA
            times += 1
        
        return ErrorCodes.OK, packetRX




    