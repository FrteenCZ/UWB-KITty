#include "serial_control.h"

trilateration trilat;

using CommandFn = void (*)(const String &args);

struct Command
{
    const char *name;
    CommandFn fn;
    const char *help;
};

void cmd_help(const String &args);
void cmd_ping(const String &args);
void cmd_status_LED(const String &args);
void cmd_points(const String &args);
void cmd_wifi(const String &args);
void cmd_uwb(const String &args);

Command commands[] = {
    {"help", cmd_help, "List all commands"},
    {"ping", cmd_ping, "ping → check connection"},
    {"LED", cmd_status_LED, "LED <red|green|blue|off> → control onboard LED"},
    {"points", cmd_points, "points <json> → process trilateration points"},
    {"wifi", cmd_wifi, "wifi <auto|AP|connect to SSID PASSWORD|scan|location> → WiFi control"},
    {"UWB", cmd_uwb, "UWB <start|stop|status|switch mode> → UWB control"},
};
const size_t COMMAND_COUNT = sizeof(commands) / sizeof(commands[0]);

void handleCommand(const String &line)
{
    int space = line.indexOf(' ');
    String cmd = (space == -1) ? line : line.substring(0, space);
    String args = (space == -1) ? "" : line.substring(space + 1);

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (cmd.equalsIgnoreCase(commands[i].name))
        {
            commands[i].fn(args);
            return;
        }
    }

    Serial.println("{\"error\":\"unknown_command\"}");
    Serial.println("Type 'help' for a list of available commands.");
}

String inputLine;
void serialTask()
{
    while (Serial.available())
    {
        String inputLine = Serial.readStringUntil('\n');
        inputLine.trim();
        if (inputLine.length() > 0)
        {
            handleCommand(inputLine);
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
        Serial.print(commands[i].name);
        Serial.print(" - ");
        Serial.println(commands[i].help);
    }
}

// Basic ping command to check connection
void cmd_ping(const String &)
{
    Serial.println("pong");
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
        Serial.println("{\"error\":\"invalid_mode\"}");
        return;
    }

    Serial.println("{\"status\":\"ok\"}");
}

// Process trilateration points
void cmd_points(const String &args)
{
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, args);

    if (err)
    {
        Serial.println("{\"error\":\"invalid_json\"}");
        return;
    }

    // Get points array
    JsonArray arr = doc["distances"].as<JsonArray>();
    int count = arr.size();
    int numOfDimensions = 3; // Assuming 3D by default

    Matrix cords(count, numOfDimensions);
    Matrix distances(count, 1);

    // Dimension check
    if (arr[0]["location"].size() != 3)
    {
        Serial.println("{\"error\":\"invalid_dimensions\"}");
    }

    // Parse array
    for (int i = 0; i < count; i++)
    {
        JsonObject obj = arr[i];
        DataPoint p;
        p.x = obj["location"]["x"].as<float>();
        p.y = obj["location"]["y"].as<float>();
        p.z = obj["location"]["z"].as<float>();
        p.d = obj["distance"].as<float>();

        cords[i][0] = p.x;
        cords[i][1] = p.y;
        cords[i][2] = p.z;
        distances[i][0] = p.d;
    }

    // Update correct trilateration instance
    trilat.update(cords, distances, millis());

    // Print output
    Serial.printf(
        "data: "
        "{\"null_space\": %s, "
        "\"alpha\": %.6f, "
        "\"trilateration\": %s, "
        "\"kalman\": %s}\n",
        trilat.null_space.transpose().toString().c_str(),
        trilat.alpha,
        trilat.trilatSolution.toString().c_str(),
        trilat.getState().transpose().toString().c_str());
}

// WiFi command handler
void cmd_wifi(const String &args)
{
    if (args == "auto")
    {
        connect_to_wifi();
    }
    else if (args == "AP")
    {
        start_AP("ESP32-AP", "12345678");
    }
    else if (args.startsWith("connect to ")) // Example input: "connect to MySSID MyPassword"
    {
        String ssid = args.substring(11, args.indexOf(' ', 11));
        String password = args.substring(args.indexOf(' ', 11) + 1);
        connect_to_wifi(1, 5, ssid.c_str(), password.c_str());
    }
    else if (args == "scan")
    {
        scan_wifi();
    }
    else if (args == "location")
    {
        scan_wifi();

        // Load stored locations
        File file = SPIFFS.open("/networks.json", "r");
        if (!file)
        {
            Serial.println("Failed to open file for reading");
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
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.c_str());
            return;
        }

        // Extract the JSON object
        JsonObject root = doc.as<JsonObject>();

        // Find the matching location
        findMatchingLocation(root);
        Serial.println(bestMatch.name);
        Serial.printf("Location: %f, %f, %f\n", bestMatch.location[0], bestMatch.location[1], bestMatch.location[2]);
    }
    else
    {
        Serial.println("{\"error\":\"invalid_wifi_command\"}");
        return;
    }
    Serial.println("{\"status\":\"ok\"}");
}

// UWB command handler
void cmd_uwb(const String &args)
{
    if (args == "start")
    {
        Serial.println("Starting UWB...");
        UWB_start();
    }
    else if (args == "stop")
    {
        Serial.println("Stopping UWB...");
        UWB_stop();
    }
    else if (args == "status")
    {
        Serial.println("UWB status: ...");
        if (isRanging)
        {
            Serial.println("Ranging is active");
        }
        if (isAnchor)
        {
            Serial.println("Device is in anchor mode");
        }
        else
        {
            Serial.println("Device is in tag mode");
        }
        Serial.print("Distance: ");
        Serial.println(distance);
    }
    else if (args == "switch mode")
    {
        Serial.println("Switching UWB mode...");
        UWB_switchMode();
    }
    else
    {
        Serial.println("{\"error\":\"invalid_uwb_command\"}");
        return;
    }
    Serial.println("{\"status\":\"ok\"}");
}
