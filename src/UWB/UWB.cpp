#include "UWB.h"

#ifdef ANCHORMODE
bool isAnchor = true;
#else
bool isAnchor = false;
#endif

void newRange()
{
    Serial.print("from: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m");
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newDevice(DW1000Device *device)
{
    Serial.print("New device added -> Short: ");
    Serial.println(device->getShortAddress(), HEX);
}

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}

void startAsTag()
{
    Serial.println("Starting as TAG...");
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    DW1000Ranging.startAsTag("7D:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY);
}

void startAsAnchor()
{
    Serial.println("Starting as ANCHOR...");
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachBlinkDevice(newBlink);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);
    DW1000Ranging.startAsAnchor("82:17:5B:D5:A9:9A:E2:9C", DW1000.MODE_LONGDATA_RANGE_ACCURACY);
}


void UWB_setup()
{
    Serial.begin(115200);
    delay(1000);
    SPI.begin(14, 12, 13); // SCK, MISO, MOSI
    // init the configuration
    DW1000Ranging.initCommunication(UWB_PIN_SPI_RST, UWB_PIN_SPI_SS, UWB_PIN_SPI_IRQ); // Reset, CS, IRQ pin

    if (isAnchor)
    {
        startAsAnchor();
    }
    else
    {
        startAsTag();
    }
}

void UWB_loop()
{
    DW1000Ranging.loop();
}

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