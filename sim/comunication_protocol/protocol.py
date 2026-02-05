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
        packet = {
            "type": message.type,
            "sender": message.sender,
            "target": message.target,
            "payload": message.payload,
        }
        return (json.dumps(packet) + "\n").encode("utf-8")

    @staticmethod
    def decode(raw: str) -> Message:
        """Parse and execute a command"""
        print(f"Decoding message: {raw.strip()}")
        try:
            data = json.loads(raw)
            return Message(
                type=data["type"],
                sender=data.get("sender"),
                target=data.get("target"),
                payload=data.get("payload", {}),
            )
        except json.JSONDecodeError as e:
            print(f"Error decoding JSON: {e}")
            return None
        except KeyError as e:
            print(f"Missing key in message: {e}")
            return None
