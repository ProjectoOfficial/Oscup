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
        if length > MAX_PAYLOAD_LENGTH:
            return ErrorCodes.LENGTH_ERROR
        if payload is None:
            return ErrorCodes.NULLPOINTER
        error = self.pack(command, length, payload)
        if error is not None:
            return error
        self.bufferize(self.packet_tx)
        self.serialWriter.write(self.packet_tx)

        crc: int
        cont = 0
        time_limit = MAX_ACK_WAIT
        start_time = self.get_timer()

        while (self.packet_rx.crc != crc) and ((self.get_timer - start_time) < time_limit) and (cont < MAX_ATTEMPTS):
            if(self.get_timer() - start_time) > RETRY_INTERVAL * cont:
                len: int = self.serialWriter.read(MAX_PAYLOAD_LENGTH + 5)
                self.unpack(len)
                crc = self.computeCRC(self.RXBuffer)
                if self.packet_rx.crc != crc:
                    self.serialWriter.write(self.TXBuffer)
                elif self.packet_rx.command == RxCommands.NACK:
                    time_limit += RETRY_INTERVAL
                    crc = 0
                cont += 1
        if self.packet_rx.crc != crc:
            return ErrorCodes.ACK_TIMEOUT
        return ErrorCodes.OK

    def pack(self, command: int, length: int, buffer: bytearray):
        self.packet_tx.command = command
        self.packet_tx.length = length
        # memmove
        self.packet_tx.payload = buffer
        # bufferizziamo il pacchetto
        self.bufferize(self.packet_tx)

        # manca la parte di calcolo del CRC
        self.packet_tx.crc = self.computeCRC(self.TXBuffer)
        return ErrorCodes.OK

    def bufferize(self, packet: packet_t):
        len: int
        withCRC: bool
        if packet is None:
            return # errore da ritornare
        if packet.crc is None:
            len = 3 + packet.length
            withCRC = False
        else:
            len = 5 + packet.length
            withCRC = True
        """
        In teoria la append dovrebbe funzionare anche con i bytearray
        """
        self.TXBuffer.append(packet.id)
        self.TXBuffer.append(packet.command)
        self.TXBuffer.append(packet.length)
        for i in range(3, 3 + packet.length):
            self.TXBuffer[i] = packet.payload[i - 3]
        
        """
        visto che il crc è composto da due elementi, l'ultimo elemento
        di TXBuffer sarà anch'esso una lista, di questi due elementi
        """
        if withCRC:
            crc = bytearray
            self.TXBuffer[len - 2] = packet.crc >> 8
            self.TXBuffer[len - 1] = packet.crc & 0xFF
            
    def get_timer(self,):
        return time.monotonic()
    
    def read(self, packet: packet_t):
        length: int

        self.RXBuffer = self.serialWriter.read(MAX_PAYLOAD_LENGTH + 5)
        length = len(self.RXBuffer)
        self.unpack(length)

        packet.id = self.packet_rx.id
        packet.command = self.packet_rx.command
        packet.length = length - 5
        packet.payload = self.packet_rx.payload
        packet.crc = self.packet_rx.crc

        # scrivo l'ack se il crc che calcolo io sul buffer è uguale a quello ricevuto nel buffer
        if self.computeCRC(self.RXBuffer, length - 2) == self.packet_rx.crc:
            self.write(RxCommands.ACK, 0, "")
        return ErrorCodes.OK

    def unpack(self,len: int):
        self.packet_rx.id = self.RXBuffer[0]
        self.packet_rx.command = self.RXBuffer[1]
        self.packet_rx.length = self.RXBuffer[2]
        for i in range(3, len):
            self.packet_rx.payload[i - 3] = self.RXBuffer[i]
        self.packet_rx.crc = (self.RXBuffer[len - 2] << 8) | self.RXBuffer[len - 1]

    def computeCRC(self, array: bytearray):
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

    