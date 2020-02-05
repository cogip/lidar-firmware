#!/usr/bin/env python3

import os
import signal
import sys

from serial import Serial
from textwrap import wrap

def signal_handler(signal, frame):
    dump_file.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

dump_dir = os.path.abspath(os.path.dirname(__file__))
dump_file_path = os.path.join(dump_dir, "dump", "lidar.dump")

print(dump_dir)

dump_file = open(dump_file_path, "w+")
with Serial('/dev/ttyUSB0', 115200, timeout=1) as ser:
    line = []
    while(1):
        char = ser.read()
        dump_file.write("{}\n".format(char.hex()))
