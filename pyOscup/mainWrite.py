'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2021 Daniel Rossi & Riccardo Salami
Version: ALPHA 1.2.0
'''

from pyOscup import Oscup
from pyOscup import ErrorCodes
from struct import unpack
from time import time, sleep

from pyOscup import TxCommands
id = 0x1C
baudrate = 115200
port = "COM8"
oscup = Oscup(id, baudrate, port)

while True:
    val = 1234

    valbyte = val.to_bytes(8, 'big')
    error = oscup.write(TxCommands.SHARE ,4 ,valbyte)

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
        print("payload: {}".format(payload))
        if command == TxCommands.CONFIRM:
            print(len(valbyte))
            print("long long value: {}".format(unpack('Q', bytearray(payload[:8]))))
        print("crc: {}".format(crc))
        print("\n\n")
    
    sleep(0.01)
    