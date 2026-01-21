import cmd
from typing import Callable
from threading import Thread, Event
import time
import sys

# Append venv from source directory, because of pyserial
sys.path.append(
    '/home/frtanta/Dokumenty/UWB-KITty/sim/.venv/lib/python3.13/site-packages')

try:
    from serial import Serial
except ImportError:
    print("Could not import pyserial!")


class SerialThread(Thread):
    def __init__(
        self,
        port: str = "/dev/ttyUSB0",
        baudrate: int = 115200,
        print_all: bool = True,
    ):
        super().__init__(daemon=True)
        self.ser = Serial(port, baudrate, timeout=1)
        self.stop_event = Event()
        self.latest_line = ""
        self.print_all = print_all

        self._rx_queue = []

    def run(self):
        try:
            while not self.stop_event.is_set():
                line = self.ser.readline()
                if not line:
                    continue

                self._rx_queue.append(line)

        finally:
            self.ser.close()

    def stop(self):
        self.stop_event.set()

    def send_command(self, cmd: str | list[str], silent: bool = True):
        if not self.ser.is_open:
            raise RuntimeError("Serial port is not open")

        if isinstance(cmd, list):
            cmd = " ".join(cmd)

        if not silent:
            print(f"Sending command: {cmd}")

        for i in range(0, len(cmd), 64):
            self.ser.write(cmd[i:i+64])
            time.sleep(0.001) # brief pause to allow buffer to clear
        self.ser.flush()

    def read_messages(self):
        msgs = self._rx_queue
        self._rx_queue = []
        return msgs
