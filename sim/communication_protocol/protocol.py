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
        try:
            text = raw.decode("utf-8").strip()
            if not text:
                return None
                
            parts = text.split(" ", 2)
            if not parts:
                return None
                
            tag = parts[0]
            
            if tag == "[DAT]":
                if len(parts) >= 3 and parts[1] == "DATA":
                    payload_str = parts[2]
                    try:
                        data = json.loads(payload_str)
                        # Sender will be inferred by DeviceManager if not present
                        sender_id = data.get("tag_id")
                        return Message(type="data:", payload=data, sender=sender_id, target="device")
                    except json.JSONDecodeError as e:
                        print(f"Error decoding JSON payload: {e}")
                        return None
                elif len(parts) >= 3 and parts[1] == "LOC":
                    coords = [float(x) for x in parts[2].split(',')]
                    return Message(type="loc", payload=coords, sender=None, target="system")
                elif len(parts) >= 3 and parts[1] == "DIST":
                    dist = float(parts[2])
                    return Message(type="dist", payload=dist, sender=None, target="system")
                else:
                    return Message(type="system", payload=text)
                    
            elif tag == "[MSG]":
                return Message(type="msg", payload=parts[1] + (" " + parts[2] if len(parts) > 2 else ""))
            elif tag == "[ACK]":
                return Message(type="ack", payload=parts[1] + (" " + parts[2] if len(parts) > 2 else ""))
            elif tag == "[ERR]":
                return Message(type="err", payload=parts[1] + (" " + parts[2] if len(parts) > 2 else ""))
                
            else:
                # Fallback to older format or generic commands
                msg_type = parts[0]
                payload = parts[1] if len(parts) > 1 else ""
                if len(parts) > 2:
                    payload = parts[1] + " " + parts[2]
                return Message(type=msg_type, payload=payload, sender=None, target="device")
        
        except Exception as e:
            print(f"Error decoding message: {e}")
            return None
