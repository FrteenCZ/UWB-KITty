import threading
import time
import bpy
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

    def run(self):
        while self.keep_running:
            if self.ser.in_waiting:
                line = self.ser.readline().decode().strip()
                print(f"[ESP] {line}")
                self.latest_line = line
            else:
                time.sleep(0.1)

    def stop(self):
        self.keep_running = False
        self.ser.close()

    def send_command(self, command, silent=False):
        if self.ser.is_open:
            if not silent:
                print(f"[BLENDER] Sending command: {command}")
            self.ser.write(f"{command}\n".encode())
        else:
            print("[ESP] Serial port is not open.")

    def send_trilateration(self, context):
        points = []
        obj = context.active_object
        if obj:
            collection_name = "DistanceTargets"

            for target in bpy.data.collections.get(collection_name, []).objects:
                if target == obj:
                    continue

                points.append({
                    "name": target.name,
                    "location": {
                        "x": target.location.x,
                        "y": target.location.y,
                        "z": target.location.z
                    },
                    "distance": (target.location - obj.location).length
                })

            if points:
                data = json.dumps(points)
                self.send_command(f"points {data}", silent=True)
                print(f"[BLENDER] Sent points")