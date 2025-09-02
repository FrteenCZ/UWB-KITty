import threading
import time
import bpy
import json
import sys
import mathutils

sys.path.append(
    '/home/frtanta/Dokumenty/UWB-KITty/sim/.venv/lib/python3.13/site-packages')


try:
    import serial
except ImportError:
    print("Could not import pyserial!")


def ensure_arrow(name, start=(0, 0, 0), end=(0, 0, 0)):
    """Create or update an arrow (mesh line) between start and end."""
    obj = bpy.data.objects.get(name)
    if obj is None:
        # Create a mesh line
        mesh = bpy.data.meshes.new(name + "Mesh")
        obj = bpy.data.objects.new(name, mesh)
        bpy.context.collection.objects.link(obj)
    # Update geometry
    mesh = obj.data
    mesh.clear_geometry()
    mesh.from_pydata([start, end], [(0, 1)], [])
    mesh.update()
    return obj


def draw_eigenvectors(eigenvalues, eigenvectors, centroid=(0, 0, 0)):
    """Draw eigenvectors as arrows starting from centroid, scaled by eigenvalues."""
    centroid = mathutils.Vector(centroid)
    for i, (eigval, eigvec) in enumerate(zip(eigenvalues, eigenvectors)):
        eigvec = mathutils.Vector(eigvec)
        end = centroid + eigvec * eigval
        ensure_arrow(f"EigenVector_{i}", centroid, end)


def draw_decentered_solution(u, last_eigvec, alpha):
    """Draw correction vector starting at u, direction = last_eigvec, length = alpha."""
    start = mathutils.Vector(u)
    end = start + mathutils.Vector(last_eigvec) * alpha
    ensure_arrow("DecenteredSolution", start, end)


def draw_kalman_velocity(position, velocity):
    """Draw velocity vector at Kalman filter position."""
    start = mathutils.Vector(position)
    end = start + mathutils.Vector(velocity)
    ensure_arrow("KalmanVelocity", start, end)


class SerialThread(threading.Thread):
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        super().__init__()
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.keep_running = True
        self.latest_line = ""

        self.trilateration_cords = None
        self.kalman_cords = None
        self.eigenvectors = None

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
                print(f"[BLENDER] Sending command: {command}")
            self.ser.write(f"{command}\n".encode())
        else:
            print("[ESP] Serial port is not open.")

    def send_trilateration(self, context):
        points = []
        obj = bpy.data.objects["Cube"]
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

    def process_latest_line(self):
        if self.latest_line.startswith("Trilateration Solution JSON:"):
            try:
                data = self.latest_line.split(":")[1].strip()
                self.trilateration_cords = json.loads(data)
                [x, y, z] = self.trilateration_cords[0]

                obj = bpy.data.objects.get("Trilateration")
                if obj:
                    obj.location = (x, y, z)

            except json.JSONDecodeError as e:
                print(f"[BLENDER] Error decoding trilateration data: {e}")

        elif self.latest_line.startswith("Kalman Filter State JSON:"):
            try:
                data = self.latest_line.split(":")[1].strip()
                self.kalman_cords = json.loads(data)
                [x, y, z, vx, vy, vz] = self.kalman_cords[0]

                obj = bpy.data.objects.get("Kalman")
                if obj:
                    obj.location = (x, y, z)

                obj = bpy.data.objects.get("KalmanVelocityDirection")
                if obj:
                    obj.location = (x + vx, y + vy, z + vz)

                    scale = (vx**2 + vy**2 + vz**2)**0.5 * 0.1
                    obj.scale = (scale, scale, scale)

            except json.JSONDecodeError as e:
                print(f"[BLENDER] Error decoding Kalman data: {e}")

        elif self.latest_line.startswith("Alpha:"):
            try:
                alpha = float(self.latest_line.split(":")[1].strip())
                if self.trilateration_cords:
                    u = self.trilateration_cords[0]
                    eigenVector = [
                        self.eigenvectors[0][-1],
                        self.eigenvectors[1][-1],
                        self.eigenvectors[2][-1]
                    ]
                    ensure_arrow("SolutionCorrection1", u,
                                 (u[0] + eigenVector[0] * alpha,
                                  u[1] + eigenVector[1] * alpha,
                                  u[2] + eigenVector[2] * alpha))
                    ensure_arrow("SolutionCorrection2", u,
                                 (u[0] - eigenVector[0] * alpha,
                                  u[1] - eigenVector[1] * alpha,
                                  u[2] - eigenVector[2] * alpha))
            except ValueError as e:
                print(f"[BLENDER] Error parsing alpha value: {e}")

        elif self.latest_line.startswith("Eigenvectors (E):"):
            try:
                data = self.latest_line.split(":")[1].strip()
                self.eigenvectors = json.loads(data)
            except json.JSONDecodeError as e:
                print(f"[BLENDER] Error decoding eigenvectors data: {e}")
