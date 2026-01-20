import json
from typing import Callable, Dict, List

# Define command handler type
CommandFn = Callable[[str], None]


class Command:
    def __init__(self, name: str, fn: CommandFn):
        self.name = name
        self.fn = fn


def cmd_trilateration(args: str) -> None:
    """Handle trilateration command"""
    try:
        data = json.loads(args)
        null_space = data.get("null_space", [])
        alpha = data.get("alpha", 0.0)
        trilateration = data.get("trilateration", [])
        kalman = data.get("kalman", [])
        # You need to store these values in device

    except json.JSONDecodeError:
        print(json.dumps({"error": "invalid_json"}))


# Command registry
COMMANDS: List[Command] = [
    Command("data", cmd_trilateration),
]


def parse_packet(data: str) -> None:
    """Parse and execute a command"""
    parts = data.strip().split(None, 1)  # Split on first whitespace
    if not parts:
        return

    cmd = parts[0].lower()
    args = parts[1] if len(parts) > 1 else ""

    # Find and execute command
    for command in COMMANDS:
        if command.name.lower() == cmd:
            command.fn(args)
            return

    # Command not found
    RuntimeError(f"Unknown command: {cmd}")
