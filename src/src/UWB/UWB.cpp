#include "UWB.h"

const int measurementBufferSize = 10;
float measurementBuffer[measurementBufferSize];
int measurementBufferIndex;

float distance = 0.0;
float avgDistance = 0.0;

bool isRanging = false;
bool isAnchor = false;

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
    DW1000Ranging.startAsTag("7D:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);
    StatusLED_setColor(0, 50, 0);
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
    DW1000Ranging.startAsAnchor("82:17:5B:D5:A9:9A:E2:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);
    StatusLED_setColor(0, 0, 50);
}

/**
 * @brief Initialize the UWB module
 *
 * This function initializes the UWB module and starts it as either a tag or an anchor.
 */
void UWB_setup()
{
    // Load preferences
    preferences.begin("UWB", true);
    isRanging = preferences.getBool("isRanging", false);
    isAnchor = preferences.getBool("isAnchor", false);
    preferences.end();

    // Initialize measurement buffer
    for (int i = 0; i < measurementBufferSize; i++)
    {
        measurementBuffer[i] = 0.0;
    }
    measurementBufferIndex = 0;

    SPI.begin(UWB_PIN_SPI_SCK, UWB_PIN_SPI_MISO, UWB_PIN_SPI_MOSI);

    // init the configuration
    DW1000Ranging.initCommunication(UWB_PIN_SPI_RST, UWB_PIN_SPI_SS, UWB_PIN_SPI_IRQ);
    
    // Wait for DW1000 to stabilize
    delay(100);
    
    // Enable range filter to smooth noisy measurements
    DW1000Ranging.useRangeFilter(true);
    DW1000Ranging.setRangeFilterValue(15);  // Exponential moving average, 15 samples

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
    
    // Wait for ranging to settle
    delay(500);
    
    // Set antenna delay AFTER starting ranging
    DW1000.setAntennaDelay(16384);
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
void UWB_switchMode()
{
    Serial.println("Switching mode...");

    // This is a workaround to "stop" current ranging
    detachInterrupt(digitalPinToInterrupt(UWB_PIN_SPI_IRQ));

    // Re-init SPI communication
    DW1000Ranging.initCommunication(UWB_PIN_SPI_RST, UWB_PIN_SPI_SS, UWB_PIN_SPI_IRQ);
    
    delay(100);

    if (isAnchor)
    {
        isAnchor = false;
        preferences.begin("UWB", false);
        preferences.putBool("isAnchor", isAnchor);
        preferences.end();

        startAsTag();
    }
    else
    {
        isAnchor = true;
        preferences.begin("UWB", false);
        preferences.putBool("isAnchor", isAnchor);
        preferences.end();

        startAsAnchor();
    }

    delay(500);
    DW1000.setAntennaDelay(16384);
    
    // Enable range filter
    DW1000Ranging.useRangeFilter(true);
    DW1000Ranging.setRangeFilterValue(15);
}

void UWB_start()
{
    Serial.println("Starting UWB...");
    isRanging = true;
    preferences.begin("UWB", false);
    preferences.putBool("isRanging", isRanging);
    preferences.end();

    if (isAnchor)
    {
        startAsAnchor();
    }
    else
    {
        startAsTag();
    }
}

void UWB_stop()
{
    Serial.println("Stopping UWB...");
    isRanging = false;
    preferences.begin("UWB", false);
    preferences.putBool("isRanging", isRanging);
    preferences.end();
    StatusLED_setColor(50, 0, 0);
}