#include "serial_control.h"
#include "../VirtualMachnies/VirtualMachnies.h"

using CommandFn = void (*)(const String &args);

struct Command
{
    const char *name;
    const char *alias;
    CommandFn fn;
    const char *help;
};

// Helper functions for protocol responses
void sendACK(const String &cmd, const String &msg = "")
{
    Serial.print("[ACK] ");
    Serial.print(cmd);
    if (msg.length() > 0)
    {
        Serial.print(" ");
        Serial.print(msg);
    }
    Serial.println();
}

void sendERR(const String &msg)
{
    Serial.print("[ERR] ");
    Serial.println(msg);
}

void sendMSG(const String &msg)
{
    Serial.print("[MSG] ");
    Serial.println(msg);
}

void sendDAT(const String &type, const String &data)
{
    Serial.print("[DAT] ");
    Serial.print(type);
    Serial.print(" ");
    Serial.println(data);
}

void cmd_help(const String &args);
void cmd_ping(const String &args);
void cmd_status_LED(const String &args);
void cmd_wifi(const String &args);
void cmd_uwb(const String &args);
void cmd_config_anchor(const String &args);
void cmd_sim_update(const String &args);

Command commands[] = {
    {"help", "h", cmd_help, "List all commands"},
    {"ping", "p", cmd_ping, "ping -> check connection"},
    {"LED", "l", cmd_status_LED, "LED <red|green|blue|off> -> control onboard LED"},
    {"wifi", "w", cmd_wifi, "wifi <auto|AP|connect to SSID PASSWORD|scan|location> -> WiFi control"},
    {"UWB", "u", cmd_uwb, "UWB <start|stop|status|switch> -> UWB control"},
    {"config_anchor", "ca", cmd_config_anchor, "config_anchor <json> -> register anchor position"},
    {"sim_update", "su", cmd_sim_update, "sim_update <json> -> update virtual tag"},
};
const size_t COMMAND_COUNT = sizeof(commands) / sizeof(commands[0]);

void handleCommand(const String &line)
{
    int space = line.indexOf(' ');
    String cmd = (space == -1) ? line : line.substring(0, space);
    String args = (space == -1) ? "" : line.substring(space + 1);

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (cmd.equalsIgnoreCase(commands[i].name) || cmd.equalsIgnoreCase(commands[i].alias))
        {
            commands[i].fn(args);
            return;
        }
    }

    sendERR("Command not found: " + cmd);
    sendMSG("Type 'help' for a list of available commands.");
}

String buffer;
bool echo = true;
bool waiting = true;

void serialTask()
{
    while (Serial.available())
    {
        char c = Serial.read();
        buffer += c;

        if (waiting)
        {
            waiting = false;
            if (c == '-')
            {
                echo = false;
                buffer = "";
            }
        }

        if (echo)
        {
            Serial.print(c); // Echo back the received character
        }

        if (c == '\n')
        {
            echo = true;
            waiting = true;

            buffer.trim();
            if (buffer.length() > 0)
            {
                handleCommand(buffer);
            }
            buffer = "";
        }
    }
}

// =====================================================================================================
// ====================================== Command Implementations ======================================
// =====================================================================================================

// List all available commands
void cmd_help(const String &)
{
    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        String msg = String(commands[i].name) + " - " + String(commands[i].help);
        sendMSG(msg);
    }
}

// Basic ping command to check connection
void cmd_ping(const String &)
{
    sendACK("ping");
}

// Control the status LED
void cmd_status_LED(const String &args)
{
    if (args == "red")
    {
        StatusLED_setColor(255, 0, 0);
    }
    else if (args == "green")
    {
        StatusLED_setColor(0, 255, 0);
    }
    else if (args == "blue")
    {
        StatusLED_setColor(0, 0, 255);
    }
    else if (args == "off")
    {
        StatusLED_setColor(0, 0, 0);
    }
    else
    {
        sendERR("invalid_mode");
        return;
    }

    sendACK("LED", args);
}

// WiFi command handler
void cmd_wifi(const String &args)
{
    if (args == "auto")
    {
        sendMSG("Connecting to saved WiFi...");
        connect_to_wifi();
    }
    else if (args == "AP")
    {
        sendMSG("Starting Access Point...");
        start_AP("ESP32-AP", "12345678");
    }
    else if (args.startsWith("connect to "))
    { // Example input: "connect to MySSID MyPassword"
        String ssid = args.substring(11, args.indexOf(' ', 11));
        String password = args.substring(args.indexOf(' ', 11) + 1);
        sendMSG("Connecting to " + ssid + "...");
        connect_to_wifi(1, 5, ssid.c_str(), password.c_str());
    }
    else if (args == "scan")
    {
        sendMSG("Scanning WiFi...");
        scan_wifi();
    }
    else if (args == "location")
    {
        sendMSG("Scanning for location...");
        scan_wifi();

        // Load stored locations
        File file = SPIFFS.open("/networks.json", "r");
        if (!file)
        {
            sendERR("Failed to open networks.json");
            return;
        }

        // Read file into a string
        String content = file.readString();
        file.close();

        // Parse the string into a JSON document
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, content);
        if (error)
        {
            sendERR("Failed to parse JSON: " + String(error.c_str()));
            return;
        }

        // Extract the JSON object
        JsonObject root = doc.as<JsonObject>();

        // Find the matching location
        findMatchingLocation(root);
        sendMSG("Best match: " + String(bestMatch.name));

        String locData = String(bestMatch.location[0]) + "," + String(bestMatch.location[1]) + "," + String(bestMatch.location[2]);
        sendDAT("LOC", locData);
    }
    else
    {
        sendERR("invalid_wifi_command");
        return;
    }
    sendACK("wifi", args);
}

// UWB command handler
void cmd_uwb(const String &args)
{
    if (args == "start")
    {
        sendMSG("Starting UWB...");
        UWB_start();
    }
    else if (args == "stop")
    {
        sendMSG("Stopping UWB...");
        UWB_stop();
    }
    else if (args == "status")
    {
        String status = "UWB status: ";
        if (isRanging)
        {
            status += "Ranging Active, ";
        }
        else
        {
            status += "Ranging Inactive, ";
        }

        if (isAnchor)
        {
            status += "Mode: Anchor";
        }
        else
        {
            status += "Mode: Tag";
        }
        sendMSG(status);
        sendDAT("DIST", String(distance));
    }
    else if (args == "switch")
    {
        sendMSG("Switching UWB mode...");
        UWB_switchMode();
    }
    else
    {
        sendERR("invalid_uwb_command");
        return;
    }
    sendACK("UWB", args);
}

// VM: Register Anchor
// Input: {"id": 1, "x": 1.0, "y": 2.0, "z": 0.0}
void cmd_config_anchor(const String &args)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, args);
    if (err)
    {
        sendERR("Failed to parse JSON: " + String(err.c_str()));
        return;
    }

    uint16_t id = doc["id"];
    float x = doc["x"];
    float y = doc["y"];
    float z = doc["z"];

    VirtualMachine *vm = VMManager::getInstance().addVM(id, VM_ANCHOR);
    vm->setPosition(x, y, z);
    sendACK("config_anchor", "id=" + String(id));
}

// VM: Update Simulation
// Input: {"tag_id": 100, "measurements": [{"id": 1, "d": 5.0}, ...]}
void cmd_sim_update(const String &args)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, args);
    if (err)
    {
        sendERR("Failed to parse JSON: " + String(err.c_str()));
        return;
    }

    uint16_t tag_id = doc["tag_id"];
    JsonArray measurementsJson = doc["measurements"];

    std::vector<Measurement> measurements;
    for (JsonObject m : measurementsJson)
    {
        Measurement sm;
        sm.anchor_id = m["id"];
        sm.distance = m["d"];
        measurements.push_back(sm);
    }

    VirtualMachine *tag = VMManager::getInstance().processMeasurements(tag_id, measurements);

    if (tag)
    {
        trilateration t = tag->getTrilateration();

        String trilatStr = "{\"null_space\": " + String(t.null_space.transpose().toString().c_str()) + ", ";
        trilatStr += "\"alpha\": " + String(t.alpha) + ", ";
        trilatStr += "\"trilateration\": " + String(t.trilatSolution.toString().c_str()) + ", ";
        trilatStr += "\"kalman\": " + String(t.getState().transpose().toString().c_str()) + "}\n";

        sendDAT("DATA", trilatStr);
    }
    else
    {
        sendERR("Failed to process measurements for tag_id: " + String(tag_id));
    }
}