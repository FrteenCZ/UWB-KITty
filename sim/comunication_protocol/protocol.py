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
        return f"{message.type} {json.dumps(message.payload)}\n".encode("utf-8")
        # return f"{message.type} {message.payload}\n".encode("utf-8")

        # packet = {
        #     "type": message.type,
        #     "sender": message.sender,
        #     "target": message.target,
        #     "payload": message.payload,
        # }
        # return (json.dumps(packet) + "\n").encode("utf-8")

    @staticmethod
    def decode(raw: str) -> Message:
        print("Decoding raw:", raw)
        """Parse and execute a command"""
        parts = raw.decode("utf-8").strip().split(None, 1)  # Split on first whitespace
        if not parts:
            return

        type = parts[0].lower()
        data = parts[1] if len(parts) > 1 else ""
        try:
            payload = json.loads(data)
        except:
            payload = data

        print("Decoded type:", type)
        print("Decoded payload:", payload)
        # data = json.loads(raw)
        return Message(
            # type=data["type"],
            # sender=data.get("sender"),
            # target=data.get("target"),
            # payload=data.get("payload", {}),
            type = type,
            sender = "default",
            target = "device",
            payload=payload
        )
