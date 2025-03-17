#include "UWB.h"

#ifndef TAG_MODE
#define ANCHOR_ADD "83:17:5B:D5:A9:9A:E2:9C"
#else
#define TAG_ADD "7D:00:22:EA:82:60:3B:9C"
#endif

void newRange();
void newBlink(DW1000Device *device);
void inactiveDevice(DW1000Device *device);
void newDevice(DW1000Device *device);

void UWB_setup()
{
    Serial.begin(115200);
    delay(1000);

    SPI.begin(UWB_PIN_SPI_SCK, UWB_PIN_SPI_MISO, UWB_PIN_SPI_MOSI);

    DW1000Ranging.initCommunication(UWB_PIN_SPI_RST, UWB_PIN_SPI_SS, UWB_PIN_SPI_IRQ);

    DW1000Ranging.attachNewRange(newRange);
#ifdef TAG_MODE
    DW1000Ranging.attachNewDevice(newDevice);
#else
    DW1000Ranging.attachBlinkDevice(newBlink);
#endif

    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    // Enable the filter to smooth the distance
    // DW1000Ranging.useRangeFilter(true);
    DW1000.setChannel(5); // Use the same channel for both tag and anchor

#ifdef TAG_MODE
    // we start the module as a tag
    DW1000Ranging.startAsTag(TAG_ADD, DW1000.MODE_LONGDATA_FAST_ACCURACY);
#else
    // we start the module as an anchor
    //  DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    //  DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_LOWPOWER);
    //  DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_FAST_LOWPOWER);
    //  DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_SHORTDATA_FAST_ACCURACY);
    //  DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_FAST_ACCURACY);
    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_FAST_ACCURACY, false);
#endif
}

void UWB_loop()
{
    DW1000Ranging.loop();
}

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

void newBlink(DW1000Device *device)
{
    Serial.print("blink; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void newDevice(DW1000Device *device)
{
    Serial.print("ranging init; 1 device added ! -> ");
    Serial.print(" short:");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
    Serial.print("delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}