import json

class Message:
    def __init__(self, type, sender=None, target=None, payload=None):
        self.type = type
        self.sender = sender
        self.target = target
        self.payload = payload or {}


class Protocol:
    @staticmethod
    def encode(message: Message):
        # packet = {
        #     "type": message.type,
        #     "sender": message.sender,
        #     "target": message.target,
        #     "payload": message.payload,
        # }
        # return (json.dumps(packet) + "\n").encode("utf-8")

        # Placeholder until I implement the protocol on the ESP side
        return (f"{message.type} {json.dumps(message.payload)}\n").encode("utf-8")

    @staticmethod
    def decode(raw: bytes) -> Message:
        """Parse and execute a command"""
        print(f"Raw message received: {raw}")
        # try:
        #     data = json.loads(raw)
        #     return Message(
        #         type=data["type"],
        #         sender=data.get("sender"),
        #         target=data.get("target"),
        #         payload=data.get("payload", {}),
        #     )
        # except json.JSONDecodeError as e:
        #     print(f"Error decoding JSON: {e}")
        #     return None
        # except KeyError as e:
        #     print(f"Missing key in message: {e}")
        #     return None

        # Placeholder until I implement the protocol on the ESP side
        try:
            parts = raw.decode("utf-8").strip().split(" ", 1)
            msg_type = parts[0]
            payload = json.loads(parts[1]) if len(parts) > 1 else {}
            return Message(type=msg_type, payload=payload, sender=0, target="device")
        
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON payload: {e}")
            return None
        except Exception as e:
            print(f"Error decoding message: {e}")
            return None
