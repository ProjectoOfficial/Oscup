from dataclasses import dataclass
import time
import serial
import struct

MAX_PAYLOAD_LENGTH = 255

MAX_ACK_WAIT = 150
RETRY_INTERVAL = 50
MAX_ATTEMPTS = 10


# definiamo le struct
@dataclass
class timer_info_t:
    alarm_value: int
    auto_reload: bool
    # mancano due timer_group_t


@dataclass
class packet_t:
    id: int
    command: int
    length: int
    payload: bytearray
    crc: int


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
    UARTREAD_ERROR: int
    OK = 0
    LENGTH_ERROR: int
    PACKMEMMOVE_ERROR: int
    WRITE_ERROR: int
    NULLPOINTER: int
    ACK_TIMEOUT: int


class Oscup:
    def __init__(self, id: int, baudrate: int):
        self.id = id
        self.baudrate = baudrate
        self.intr_alloc_flags = 0
        self.packet_rx: packet_t
        self.packet_tx: packet_t
        self.serialWriter = serial.Serial(port='COM4', baudrate=baudrate, timeout=.1)
        """
        Implemento i due buffer come liste, perché siccome qui non ci sono
        gli array di caratteri ma direttamente le stringhe verrebbe scomodo 
        castarli a stringhe, quindi facciamo una lista in cuo il primo elemento
        è l'id, il secondo è il comando, il terzo è la lunghezza, il quarto
        è il payload (stringa), il quinto è l'eventuale crc, che se nonn c'è è None,
        ovvero il NULL di python
        """
        self.TXBuffer = bytearray()
        self.RXBuffer = bytearray()

    def write(self, command: int, length: int, payload: str):
        """
        This functions uses the Serial object in its attrbibutes to write
        data to the serial port
        """
        totalLength = 5 + length
        arr = bytearray(totalLength)
        arr[0] = self.serialWriter[0]
        arr[1] = command
        arr[2] = length
        for i in range(3, length):
            arr[i] = payload[i-3]
        crc = self.computeCRC(arr)
        assert len(crc) == 2
        arr[totalLength - 2] = crc[0]
        arr[totalLength - 1] = crc[1]
        self.serialWriter.write(arr)


            
    def get_timer(self,):
        return time.monotonic()
    
    def read(self,):
        length: int

        self.RXBuffer = self.serialWriter.read(MAX_PAYLOAD_LENGTH + 5)
        length = len(self.RXBuffer)
        self.unpack(length)

        # scrivo l'ack se il crc che calcolo io sul buffer è uguale a quello ricevuto nel buffer
        if self.computeCRC(self.RXBuffer, length - 2) == self.packet_rx.crc:
            self.write(RxCommands.ACK, 0, "")
        return ErrorCodes.OK

    def unpack(self, len: int):
        self.packet_rx.id = self.RXBuffer[0]
        self.packet_rx.command = self.RXBuffer[1]
        self.packet_rx.length = self.RXBuffer[2]
        for i in range(3, len):
            self.packet_rx.payload[i - 3] = self.RXBuffer[i]
        self.packet_rx.crc = (self.RXBuffer[len - 2] << 8) | self.RXBuffer[len - 1]

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

        print(crc, type(crc), len(crc))
        return crc

    