import bpy # type: ignore
import time
from .device import Device
from ..comunication_protocol.protocol import Protocol
from ..utils.ESPcom import SerialThread


class DeviceManager:
    def __init__(self):
        self.devices: dict[str, Device] = {}
        self.serial_links: dict[str, SerialThread] = {}
        self._last_send_time = 0.0

    def load_devices_from_properties(self, context, serial_port):
        self.clear_devices()
        scene_props = context.scene.uwb_kitty_props
        
        if not serial_port or not serial_port.is_alive():
            print("Serial port not running. Cannot load devices.")
            return

        for device_prop in scene_props.devices:
            blender_obj = bpy.data.objects.get(device_prop.blender_object_name)
            if not blender_obj:
                print(f"Object '{device_prop.blender_object_name}' not found for device '{device_prop.id}'. Skipping.")
                continue

            device = Device(
                device_id=device_prop.id,
                blender_object=blender_obj,
                role=device_prop.role,
            )
            self.add_device(device, serial_port)
        
        print(f"Loaded {len(self.devices)} devices from properties.")

    def add_device(self, device, port):
        self.devices[device.id] = device
        self.serial_links[device.id] = port

    def clear_devices(self):
        self.devices.clear()
        self.serial_links.clear()

    def update(self):
        # 1. Read incoming messages
        for serial in set(self.serial_links.values()):
            if not serial.is_alive():
                continue
            for raw in serial.read_messages():
                msg = Protocol.decode(raw)
                if msg:
                    self._dispatch_incoming(msg)

        # 2. Collect outgoing messages
        for device in self.devices.values():
            for msg in device.produce_messages():
                self._send_message(msg)

        # 3. Update device states
        for device in self.devices.values():
            device.update_state()

        # 4. Send distances periodically
        current_time = time.time()
        if current_time - self._last_send_time >= 1/6:
            self._send_distances()
            self._last_send_time = current_time

    def _send_distances(self):
        anchors = [
            dev.obj for dev in self.devices.values() if dev.role == "ANCHOR"
        ]
        tags = [dev for dev in self.devices.values() if dev.role == "TAG"]

        if not anchors or not tags:
            return

        for tag in tags:
            tag.send_distances(anchors)

    def _dispatch_incoming(self, msg):
        if msg.target == "all":
            for device in self.devices.values():
                device.on_message(msg)

        elif msg.target == "device":
            device = self.devices.get(msg.sender)
            if device:
                device.on_message(msg)

        elif msg.target == "system":
            self.handle_system_message(msg)

    def _send_message(self, msg):
        port = self.serial_links.get(msg.sender)
        if port:
            raw = Protocol.encode(msg)
            port.send_command(raw, True)

    def handle_system_message(self, msg):
        print("System message:", msg.type)