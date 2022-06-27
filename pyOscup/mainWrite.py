'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2022 Daniel Rossi 
Version: 1.2.4
'''

from pyOscup import Oscup
from pyOscup import ErrorCodes
from pyOscup import packet_t
from struct import unpack
from time import time, sleep

from pyOscup import TxCommands
id = 0x1C
baudrate = 115200
port = "COM3"
oscup = Oscup(id, baudrate, port)

while True:
    val = 1234

    valbyte = val.to_bytes(8, 'little')
    error = oscup.write(TxCommands.SHARE, len(valbyte), valbyte)

    error = ErrorCodes.NO_DATA
    start = time()
    while error != ErrorCodes.OK and time() - start < 0.2:
        error, packet = oscup.read()

        if error:
            if error != ErrorCodes.NO_DATA:
                print("Error: {}".format(error))

    if error == ErrorCodes.OK:
        print("Data incoming:")
        id, command, length, payload, crc = packet.getParams()
        print("id: {} - command: {} - length: {}".format(hex(id), command, length))
        print("payload:")
        print(packet_t.translate_bytes(payload))
        if command == TxCommands.CONFIRM or command == 3:
            print(len(valbyte))
            print("long long value: {}".format(unpack('Q', bytearray(payload[:8]))))
            print("packet crc: {} --> calculated crc: {}".format(crc, packet_t.computeCRC(packet.getBuff()[:-2])))
            
        print("\n\n")
    
    sleep(0.01)
    