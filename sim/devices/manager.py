import json
from device import Device
from ..comunication_protocol.parse import parse_packet

devices = {}

def manager(args: str) -> None:
    data = json.loads(args)
    global devices
    reciever_id = data.get("reciever_id", "default")
    payload = data.get("payload", "")

    if reciever_id in devices:
        reciever = devices[reciever_id]
    else:
        reciever = Device()
        devices[reciever_id] = reciever

    parse_packet(payload, reciever)
    