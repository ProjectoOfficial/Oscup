from pyOscup import Oscup
from time import sleep
from pyOscup import ErrorCodes
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
        print("payload: ")
        print(payload)
        print("crc: {}".format(crc))
        print("\n\n")
    