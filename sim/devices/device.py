from ..comunication_protocol.protocol import Message
import time


class Device:
    def __init__(self, device_id, blender_object=None, role="NONE"):
        self.id = device_id
        self.obj = blender_object
        self.role = role

        self.null_space = None
        self.alpha = None
        self.trilateration = None
        self.kalman = None
        self.last_data_update = None

        self.pos = [0.0, 0.0, 0.0]  # position from trilateration
        self.vel = [0.0, 0.0, 0.0]  # velocity from kalman
        self.epos = [0.0, 0.0, 0.0]  # extrapolated position from kalman

        self._outbox = []  # intents to be sent

    # called by DeviceManager
    def on_message(self, message: Message):
        print("Device", self.id, "received message:", message.type, message.payload)
        if message.type == "data:":
            self.null_space = message.payload["null_space"]
            self.alpha = message.payload["alpha"]
            self.trilateration = message.payload["trilateration"][0]
            self.kalman = message.payload["kalman"][0]

            self.last_data_update = time.time()

        elif message.type == "PING":
            self._outbox.append(
                {"type": "PONG",
                 "sender": self.id,
                 "target": "esp"}
            )

    # called by DeviceManager
    def produce_messages(self):
        msgs = self._outbox
        self._outbox = []
        return msgs

    # called by DeviceManager
    def update_state(self):
        if self.last_data_update is not None:
            self.pos = self.trilateration
            self.vel = self.kalman[3:]

            dt = time.time() - self.last_data_update
            self.epos = [
                self.kalman[0] + self.vel[0] * dt,
                self.kalman[1] + self.vel[1] * dt,
                self.kalman[2] + self.vel[2] * dt,
            ]

        print("Updated device", self.id)
        print(" Position:", self.pos)
        print(" Velocity:", self.vel)
        print(" Extrapolated Position:", self.epos)
        
        print("Null Space:", self.null_space)
        print("Alpha:", self.alpha)
        print("Trilateration:", self.trilateration)
        print("Kalman:", self.kalman)

    def send_distances(self, targets=[]):
        points = []
        if not self.obj or len(targets) == 0:
            return points

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

        message = Message(
            type="points",
            sender=self.id,
            target="esp",
            payload={"distances": points}
        )

        self._outbox.append(message)
