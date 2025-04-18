#include "UWB.h"

#ifdef ANCHORMODE
bool isAnchor = true;
#else
bool isAnchor = false;
#endif

#ifdef STARTRANGING
bool isRanging = true;
#else
bool isRanging = false;
#endif

const int measurementBufferSize = 3;
float measurementBuffer[measurementBufferSize];
int measurementBufferIndex;

float distance = 0.0;
float avgDistance = 0.0;

// Calibration variables
bool isCalibrating = false;
int minDelay = 15600;
int maxDelay = 16700;
int bestDelay = 0;
float calibrationTarget = 0.0;
float tolerance = 0.05;

/**
 * @brief Calibrate the UWB module
 *
 * This function calibrates the UWB module by adjusting the antenna delay based on the measured distance.
 */
void calibrate()
{
    if (abs(calibrationTarget - distance) < tolerance)
    {
        Serial.print("Calibration successful");
        Serial.print(" (delay: ");
        Serial.print(bestDelay);
        Serial.println(")");
        isCalibrating = false;
        return;
    }

    if (maxDelay - minDelay <= 1)
    {
        Serial.println("Calibration converged (delay range too small)");
        isCalibrating = false;
        return;
    }

    int midDelay = (minDelay + maxDelay) / 2;

    if (avgDistance < calibrationTarget)
    {
        Serial.println("Decreasing delay");
        maxDelay = midDelay;
        bestDelay = midDelay;
    }
    else
    {
        Serial.println("Increasing delay");
        minDelay = midDelay;
        bestDelay = midDelay;
    }

    Serial.printf("Current delay: %d, Distance: %.2f, Target: %.2f\n", bestDelay, avgDistance, calibrationTarget);
    Serial.printf("Min delay: %d, Max delay: %d\n", minDelay, maxDelay);

    DW1000.setAntennaDelay(bestDelay);
}

/**
 * @brief Callback function to be called when a new range is available
 *
 * This function prints the short address of the distant device, the range, and the RX power.
 */
void newRange()
{
    distance = DW1000Ranging.getDistantDevice()->getRange();

    measurementBuffer[measurementBufferIndex] = distance;
    measurementBufferIndex = (measurementBufferIndex + 1) % measurementBufferSize;
    float sum = 0;
    for (int i = 0; i < measurementBufferSize; i++)
    {
        sum += measurementBuffer[i];
    }
    avgDistance = sum / measurementBufferSize;

    Serial.print("from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(avgDistance);
    Serial.print(" m");
    Serial.printf(" (%0.2f m)", distance);
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");

    if (isCalibrating)
    {
        if (measurementBufferIndex == 0)
        {
            calibrate();
        }
    }
}

/**
 * @brief Callback function to be called when a new device is added
 *
 * @param device Pointer to the new device
 */
void newDevice(DW1000Device *device)
{
    Serial.print("New device added -> Short: ");
    Serial.println(device->getShortAddress(), HEX);
}

/**
 * @brief Callback function to be called when a new blink device is added
 *
 * @param device Pointer to the new blink device
 */
void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

/**
 * @brief Callback function to be called when a device is inactive
 *
 * @param device Pointer to the inactive device
 */
void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}

/**
 * @brief Start the UWB module as a tag
 *
 * This function initializes the UWB module as a tag device.
 */
void startAsTag()
{
    Serial.println("Starting as TAG...");
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    DW1000Ranging.startAsTag("7D:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY);
}

/**
 * @brief Start the UWB module as an anchor
 *
 * This function initializes the UWB module as an anchor device.
 */
void startAsAnchor()
{
    Serial.println("Starting as ANCHOR...");
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachBlinkDevice(newBlink);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    DW1000Ranging.startAsAnchor("82:17:5B:D5:A9:9A:E2:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY);
}

/**
 * @brief Handle the UWB root path
 *
 * This function serves the UWB HTML page when the root path is accessed.
 */
void handleUwbRoot()
{
    File file = SPIFFS.open("/UWB.html", "r");
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }
    String html = file.readString();
    file.close();
    server.send(200, "text/html", html);
}

/**
 * @brief Handle the UWB start ranging request
 *
 * This function starts the UWB ranging process when the corresponding endpoint is accessed.
 */
void handleUwbStartRanging()
{
    Serial.println("Starting ranging...");
    isRanging = true;

    if (isAnchor)
    {
        startAsAnchor();
    }
    else
    {
        startAsTag();
    }

    server.send(200, "text/plain", "Ranging started");
}

/**
 * @brief Handle the UWB stop ranging request
 *
 * This function stops the UWB ranging process when the corresponding endpoint is accessed.
 */
void handleUwbStopRanging()
{
    Serial.println("Stopping ranging...");
    isRanging = false;
    server.send(200, "text/plain", "Ranging stopped");
}

/**
 * @brief Handle the UWB switch mode request
 *
 * This function switches the UWB module mode between tag and anchor when the corresponding endpoint is accessed.
 */
void handleUwbSwitchMode()
{
    Serial.println("Switching mode...");
    switchMode();
    server.send(200, "text/plain", "Mode switched");
}

/**
 * @brief Handle the UWB status request
 *
 * This function returns the current status of the UWB module as a JSON response.
 */
void handleUwbStatus()
{
    StaticJsonDocument<256> status;

    status["isRanging"] = isRanging;
    status["isAnchor"] = isAnchor;
    status["distance"] = avgDistance;
    status["RXPower"] = DW1000Ranging.getDistantDevice()->getRXPower();

    // String shortAddr = String(DW1000Ranging.getCurrentShortAddress(), HEX);
    // shortAddr.toUpperCase();
    // status["deviceAddress"] = shortAddr;

    String otherAddr = String(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    otherAddr.toUpperCase();
    status["otherDeviceAddress"] = otherAddr;

    String json;
    serializeJson(status, json);
    server.send(200, "application/json", json);
}

/**
 * @brief Handle the UWB calibration request
 *
 * This function starts the UWB calibration process when the corresponding endpoint is accessed.
 */
void handleUwbCalibrate()
{
    if (!server.hasArg("value"))
    {
        server.send(400, "text/plain", "Missing 'value' parameter");
        return;
    }

    float newTarget = server.arg("value").toFloat();
    if (newTarget <= 0)
    {
        server.send(400, "text/plain", "Invalid distance");
        return;
    }

    minDelay = 15600;
    maxDelay = 16700;
    bestDelay = 0;

    calibrationTarget = newTarget;
    isCalibrating = true;

    server.send(200, "text/plain", "Calibration started with target: " + String(calibrationTarget));
}

/**
 * @brief Initialize the UWB module
 *
 * This function initializes the UWB module and starts it as either a tag or an anchor.
 */
void UWB_setup()
{
    SPI.begin(14, 12, 13); // SCK, MISO, MOSI
    // init the configuration
    DW1000Ranging.initCommunication(UWB_PIN_SPI_RST, UWB_PIN_SPI_SS, UWB_PIN_SPI_IRQ); // Reset, CS, IRQ pin

    if (isRanging)
    {
        if (isAnchor)
        {
            startAsAnchor();
        }
        else
        {
            startAsTag();
        }
    }

    // Set the antenna delay
    DW1000.setAntennaDelay(16150);

    // Setup web server routes
    server.on("/UWB.html", handleUwbRoot);
    server.on("/UWB", handleUwbRoot);
    server.on("/UWB/startRanging", handleUwbStartRanging);
    server.on("/UWB/stopRanging", handleUwbStopRanging);
    server.on("/UWB/switchMode", handleUwbSwitchMode);
    server.on("/UWB/status", handleUwbStatus);
    server.on("/UWB/calibrate", handleUwbCalibrate);
}

/**
 * @brief Loop function for the UWB module
 *
 * This function is called in the main loop to handle UWB communication.
 */
void UWB_loop()
{
    if (isRanging)
    {
        DW1000Ranging.loop();
    }
}

/**
 * @brief Switch the mode of the UWB module between tag and anchor
 *
 * This function switches the mode of the UWB module and reinitializes it.
 */
void switchMode()
{
    Serial.println("Switching mode...");

    // This is a workaround to "stop" current ranging
    detachInterrupt(digitalPinToInterrupt(UWB_PIN_SPI_IRQ));

    // Re-init SPI communication
    DW1000Ranging.initCommunication(UWB_PIN_SPI_RST, UWB_PIN_SPI_SS, UWB_PIN_SPI_IRQ);

    if (isAnchor)
    {
        isAnchor = false;
        startAsTag();
    }
    else
    {
        isAnchor = true;
        startAsAnchor();
    }
}