import json
from typing import Callable, Dict, List
from ..devices.device import Device
from ..devices.manager import manager

# Define command handler type
CommandFn = Callable[[str, Device], None]


class Command:
    def __init__(self, name: str, fn: CommandFn):
        self.name = name
        self.fn = fn


def cmd_trilateration(args: str, device: Device) -> None:
    """Handle trilateration command"""
    if not device:
        return

    try:
        data = json.loads(args)
    except json.JSONDecodeError:
        print(json.dumps({"error": "invalid_json"}))

    device.null_space = data.get("null_space", [])
    device.alpha = data.get("alpha", 0.0)
    device.trilateration = data.get("trilateration", [])
    device.kalman = data.get("kalman", [])



def cmd_manager(args: str, device: Device) -> None:
    """Reroute to specific device using manager"""
    manager(args)


# Command registry
COMMANDS: List[Command] = [
    Command("data", cmd_trilateration),
    Command("manager", cmd_manager),
]


def parse_packet(data: str, device: Device = None) -> None:
    """Parse and execute a command"""
    parts = data.strip().split(None, 1)  # Split on first whitespace
    if not parts:
        return

    cmd = parts[0].lower()
    args = parts[1] if len(parts) > 1 else ""

    # Find and execute command
    for command in COMMANDS:
        if command.name.lower() == cmd:
            command.fn(args, device)
            return

    # Command not found
    RuntimeError(f"Unknown command: {cmd}")
