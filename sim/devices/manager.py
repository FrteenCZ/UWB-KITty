import json
from .device import Device
from ..comunication_protocol.protocol import Protocol
from .device import Device
from ..utils.ESPcom import SerialThread


# def manager(args: str) -> None:
#     data = json.loads(args)
#     global devices
#     reciever_id = data.get("reciever_id", "default")
#     payload = data.get("payload", "")

#     if reciever_id in devices:
#         reciever = devices[reciever_id]
#     else:
#         reciever = Device()
#         devices[reciever_id] = reciever

#     parse_packet(payload, reciever)

    
class DeviceManager:
    def __init__(self):
        self.devices: dict[str, Device] = {}
        self.serial_links: dict[str, SerialThread] = {}  # port → SerialOperator

    def add_device(self, device, port):
        self.devices[device.id] = device
        self.serial_links[device.id] = port

    def update(self):
        # 1. Read incoming messages
        for serial in set(self.serial_links.values()):
            for raw in serial.read_messages():
                msg = Protocol.decode(raw)
                self._dispatch_incoming(msg)

        # 2. Collect outgoing messages
        for device in self.devices.values():
            for msg in device.produce_messages():
                self._send_message(msg)

        # 3. Update device states
        for device in self.devices.values():
            device.update_state()

    def _dispatch_incoming(self, msg):
        if msg.target == "device":
            device = self.devices.get(msg.sender)
            if device:
                device.on_message(msg)

        elif msg.target == "system":
            self.handle_system_message(msg)

    def _send_message(self, msg):
        port = self.serial_links.get(msg.sender)
        if port:
            raw = Protocol.encode(msg)
            port.send_command(raw, False)

    def handle_system_message(self, msg):
        print("System message:", msg.type)

manager = DeviceManager()