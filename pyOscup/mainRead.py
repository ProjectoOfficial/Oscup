'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2022 Daniel Rossi
Version: 1.2.4
'''

from pyOscup import Oscup
from pyOscup import packet_t
from pyOscup import ErrorCodes, TxCommands
from struct import unpack
id = 0x1C
baudrate = 115200
port = "COM8"
oscup = Oscup(id, baudrate, port)

while True:
    error, packet = oscup.read()

    if error:
        if error != ErrorCodes.NO_DATA:
            print("Error: {}".format(error))
    else:
        print("Data incoming:")
        id, command, length, payload, crc = packet.getParams()
        print("id: {} - command: {} - length: {}".format(hex(id), command, length))
        print("payload: {}".format(packet_t.translate_bytes(payload)))
        if command == TxCommands.SHARE:
            print("long long value: {}".format(unpack('Q', bytearray(payload[:8]))))
        print("crc: {}".format(crc))
        print("\n\n")
    