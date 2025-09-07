import threading
import time
import json
import sys

sys.path.append(
    '/home/frtanta/Dokumenty/UWB-KITty/sim/.venv/lib/python3.13/site-packages')


try:
    import serial
except ImportError:
    print("Could not import pyserial!")


class SerialThread(threading.Thread):
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        super().__init__()
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.keep_running = True
        self.latest_line = ""

        self.null_space = None
        self.alpha = None
        self.trilateration_cords = None
        self.kalman_cords = None

    def run(self):
        while self.keep_running:
            if self.ser.in_waiting:
                line = self.ser.readline().decode().strip()
                print(f"[ESP] {line}")
                self.latest_line = line
                self.process_latest_line()
            else:
                time.sleep(0.1)

    def stop(self):
        self.keep_running = False
        self.ser.close()

    def send_command(self, command, silent=False):
        if self.ser.is_open:
            if not silent:
                print(f"Sending command: {command}")
            self.ser.write(f"{command}\n".encode())
        else:
            print("Serial port is not open.")

    def send_trilateration(self, points):
        if points:
            data = json.dumps(points)
            self.send_command(f"points {data}", silent=True)
            print(f"Sent points: {len(points)} targets")

    def process_latest_line(self):
        if self.latest_line.startswith("data:"):
            try:
                data = self.latest_line.split("data:")[1].strip()
                data = json.loads(data)
                self.null_space = data.get("null_space", None)
                self.alpha = data.get("alpha", None)
                self.trilateration_cords = data.get("trilateration", None)
                self.kalman_cords = data.get("kalman", None)

                print(f"Parsed data: {data}")
            except json.JSONDecodeError as e:
                print(f"Error decoding JSON data: {e}")

        elif self.latest_line.startswith("Error:"):
            print(f"Received error from ESP: {self.latest_line}")

        else:
            print(f"Unrecognized line format: {self.latest_line}")
