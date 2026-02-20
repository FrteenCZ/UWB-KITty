# Serial Communication Protocol (v1.0)

## 1. Overview

This protocol uses a **Header-Payload** architecture over a standard Serial (UART) link. It is designed to be typed by a human in a terminal and parsed programmatically by Python (Blender) or VS Code extensions.

## 2. General Rules

* **Encoding:** UTF-8 / ASCII.
* **Line Terminator:** Every message must end with a Newline (`\n`).
* **Delimiter:** The first **space** character separates the Header/Command from the Payload.


---

## 3. Outgoing: Host  Module (Commands)

The Host (PC/Blender) sends commands in a simple `command <args>` format.

| Command | Args Example | Description |
| --- | --- | --- |
| `ping` | *(none)* | Check if module is alive. |
| `LED` | `red`, `off` | Changes onboard status light. |
| `UWB` | `start`, `stop` | Toggles localization radio. |
| `sim_update` | `{"tag_id": 100, "measurements": [{"id": 1, "d": 5.0}, {"id": 2, "d": -2.5}, {"id": 3, "d": 8}]}` | Sends simulation state to module via JSON. |
| `config_anchor` | `{"id": 3, "x": 3.0, "y": 4.0, "z": 2.0}` | Configures an anchor's position. |
| `help` | *(none)* | Returns list of available commands. |

---

## 4. Incoming: Module  Host (Responses)

All messages from the module must start with a **Type Tag** in brackets. This allows the receiver to filter data from debug messages.

### Response Tags

* **`[ACK]` (Acknowledge):** Confirms a command was received and processed.
* *Example:* `[ACK] UWB start`


* **`[DAT]` (Data/Telemetry):** High-frequency data for the simulation.
* *Format:* `[DAT] <TYPE> <VALUE1>,<VALUE2>,...`
* *Example:* `[DAT] POS 10.2,5.1,0.0`


* **`[MSG]` (Message):** General information intended for the human user.
* *Example:* `[MSG] Searching for UWB Anchors...`


* **`[ERR]` (Error):** Indicates a failure or invalid command.
* *Example:* `[ERR] Invalid JSON in sim_update`
