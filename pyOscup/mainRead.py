'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2021 Daniel Rossi & Riccardo Salami
Version: ALPHA 1.2.0
'''

from pyOscup import Oscup
from pyOscup import ErrorCodes
from struct import unpack
id = 0x1C
baudrate = 115200
port = "COM3"
oscup = Oscup(id, baudrate, port)

while True:
    error, packet = oscup.read()

    if error:
        if error != ErrorCodes.NO_DATA:
            print("Error: {}".format(error))
    else:
        id, command, length, payload, crc = packet.getParams()
        print("id: {} - command: {} - length: {}".format(hex(id), command, length))
        print("payload: {}".format(payload))
        print("long long value: {}".format(unpack('Q', bytearray(payload[:8]))))
        print("crc: {}".format(crc))
        print("\n\n")
    