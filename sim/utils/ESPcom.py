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
        process_latest_line: Callable[[str], None] = print,
        port: str = "/dev/ttyUSB0",
        baudrate: int = 115200,
        print_all: bool = True,
    ):
        super().__init__(daemon=True)
        self.ser = Serial(port, baudrate, timeout=1)
        self.stop_event = Event()
        self.latest_line = ""
        self.print_all = print_all
        self.process_latest_line = process_latest_line

    def run(self):
        try:
            while not self.stop_event.is_set():
                line = self.ser.readline()
                if not line:
                    continue

                decoded = line.decode(errors="replace").strip()
                self.latest_line = decoded

                if self.print_all:
                    print(f"[ESP] {decoded}")

                self.process_latest_line(decoded)
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

        self.ser.write((cmd + "\n").encode())
