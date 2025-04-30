#include "serial_control.h"

bool is2D = true;     // Default assumption
bool modeSet = false; // Flag to lock mode after first input

trilateration trilat;

void handleSerialInput()
{
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        // Serial.println("Received: " + input);

        // LED control
        if (input == "LED ON")
        {
            digitalWrite(onboardledPin, HIGH);
            Serial.println("LED is ON");
        }
        else if (input == "LED OFF")
        {
            digitalWrite(onboardledPin, LOW);
            Serial.println("LED is OFF");
        }

        // Handle trilateration input
        else if (input.startsWith("cords["))
        {
            float x, y, z, d;
            // First, determine if the input is 2D or 3D
            if (!modeSet && sscanf(input.c_str(), "cords[%f,%f,%f],%f", &x, &y, &z, &d) == 4 ||
                sscanf(input.c_str(), "cords[%f,%f,%f]", &x, &y, &z) == 3)
            {
                is2D = false;
                modeSet = true;
                Serial.println("3D mode set.");
                trilat = trilateration(3); // Initialize 3D trilateration
            }
            else if (!modeSet && sscanf(input.c_str(), "cords[%f,%f],%f", &x, &y, &d) == 3 ||
                     sscanf(input.c_str(), "cords[%f,%f]", &x, &y) == 2)
            {
                is2D = true;
                modeSet = true;
                Serial.println("2D mode set.");
                trilat = trilateration(2); // Initialize 2D trilateration
            }

            // After mode is determined, parse using the right pattern
            if (sscanf(input.c_str(), "cords[%f,%f,%f],%f", &x, &y, &z, &d) == 4 ||
                sscanf(input.c_str(), "cords[%f,%f,%f]", &x, &y, &z) == 3)
            {
                if (sscanf(input.c_str(), "cords[%f,%f,%f]", &x, &y, &z) == 3) {
                    d = distance; // Use the own measured distance
                }
                if (is2D)
                {
                    Serial.println("Warning: Input is 3D but mode is set to 2D. Ignoring z-coordinate.");
                    trilat.update({x, y, d});
                }
                else
                {
                    trilat.update({x, y, z, d});
                }
            }
            else if (sscanf(input.c_str(), "cords[%f,%f],%f", &x, &y, &d) == 3 ||
                     sscanf(input.c_str(), "cords[%f,%f]", &x, &y) == 2)
            {
                if (sscanf(input.c_str(), "cords[%f,%f]", &x, &y) == 2) {
                    d = distance; // Use the own measured distance
                }
                if (!is2D)
                {
                    Serial.println("Warning: Input is 2D but mode is set to 3D. Providing default z=0.");
                    z = 0;
                    trilat.update({x, y, z, d});
                }
                else
                {
                    trilat.update({x, y, d});
                }
            }
            else
            {
                Serial.println("Invalid input format. Expected format: cords[x,y,z],d or cords[x,y],d or cords[x,y,z] or cords[x,y]");
            }
        }
        else if (input == "getState")
        {
            Matrix state = trilat.getState();
            Serial.print("Current state: ");
            for (int i = 0; i < state.rows(); i++)
            {
                for (int j = 0; j < state.cols(); j++)
                {
                    Serial.print(state[i][j]);
                    Serial.print(" ");
                }
                Serial.println();
            }
        }
        else if (input == "printBuffer")
        {
            trilat.printBuffer();
        }

        // WiFi control
        else if (input == "WiFi auto")
        {
            connect_to_wifi();
        }
        else if (input == "WiFi AP")
        {
            start_AP("ESP32-AP", "12345678");
        }
        else if (input.startsWith("WiFi connect to ")) // Example input: "WiFi connect to MySSID MyPassword"
        {
            String ssid = input.substring(16, input.indexOf(' ', 16));
            String password = input.substring(input.indexOf(' ', 16) + 1);
            connect_to_wifi(1, 5, ssid.c_str(), password.c_str());
        }
        else if (input == "WiFi scan")
        {
            scan_wifi();
        }

        // WiFi location control
        else if (input == "WiFi location")
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

        // UWB control
        else if (input == "UWB start")
        {
            Serial.println("Starting UWB...");
            UWB_start();
        }
        else if (input == "UWB stop")
        {
            Serial.println("Stopping UWB...");
            UWB_stop();
        }
        else if (input == "UWB status")
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
        else if (input == "UWB switch mode")
        {
            Serial.println("Switching UWB mode...");
            UWB_switchMode();
        }

        // help command
        else if (input == "help")
        {
            Serial.println("<----Available commands---->");
            Serial.println("LED ON");
            Serial.println("LED OFF");
            Serial.println("cords[x,y,z],d or cords[x,y],d or cords[x,y,z] or cords[x,y]");
            Serial.println("getState");
            Serial.println("printBuffer");
            Serial.println("WiFi auto");
            Serial.println("WiFi AP");
            Serial.println("WiFi connect to SSID PASSWORD");
            Serial.println("WiFi scan");
            Serial.println("WiFi location");
            Serial.println("UWB start");
            Serial.println("UWB stop");
            Serial.println("UWB status");
            Serial.println("UWB switch mode");
        }

        // Unknown command
        else
        {
            Serial.println("Unknown command: " + input);
            Serial.println("Type 'help' for a list of available commands.");
        }
    }
}
