from ..comunication_protocol.protocol import Message
import time
from ..scene.visualizer import Visualizer


class Device:
    def __init__(self, device_id, blender_object=None, role="NONE"):
        self.id = device_id
        self.obj = blender_object
        self.role = role

        # Data from the device
        self.null_space = None
        self.alpha = None
        self.trilateration = None
        self.kalman = None
        self.last_data_update = None

        # Calculated state
        self.pos = [0.0, 0.0, 0.0]  # position from trilateration
        self.vel = [0.0, 0.0, 0.0]  # velocity from kalman
        self.epos = [0.0, 0.0, 0.0]  # extrapolated position from kalman

        self._outbox = []
        self.visualizer = Visualizer(self)

    def on_message(self, message: Message):
        try:
            if message.type == "data:":
                self.null_space = message.payload.get("null_space")
                self.alpha = message.payload.get("alpha")
                self.trilateration = message.payload.get("trilateration")[0]
                self.kalman = message.payload.get("kalman")[0]
                self.last_data_update = time.time()

            elif message.type == "PING":
                self._outbox.append(
                    Message(type="PONG", sender=self.id, target="esp")
                )

        except Exception as e:
            print(f"Error processing message for device {self.id}: {e}")

    def produce_messages(self):
        msgs = self._outbox
        self._outbox = []
        return msgs

    def update_state(self):
        if self.last_data_update is not None:
            # Update state from new data
            self.pos = self.trilateration
            if self.kalman:
                self.vel = self.kalman[3:]
                dt = time.time() - self.last_data_update
                self.epos = [
                    self.kalman[0] + self.vel[0] * dt,
                    self.kalman[1] + self.vel[1] * dt,
                    self.kalman[2] + self.vel[2] * dt,
                ]

        # Update the scene
        if self.role == "TAG":
            self.visualizer.update()

    def send_distances(self, targets=[]):
        if not self.obj or not targets:
            return

        points = []
        for target in targets:
            if target == self.obj:
                continue

            points.append({
                "name": target.name,
                "location": {
                    "x": target.location.x,
                    "y": target.location.y,
                    "z": target.location.z,
                },
                "distance": (target.location - self.obj.location).length
            })

        if points:
            message = Message(
                type="points",
                sender=self.id,
                target="esp",
                payload={"distances": points}
            )
            self._outbox.append(message)
