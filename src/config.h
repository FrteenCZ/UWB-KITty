#ifndef CONFIG_H
#define CONFIG_H

const int onboardledPin = 2;


// Define the pin configuration for DWM1000
#define UWB_PIN_SPI_MISO  12    // SPI MISO
#define UWB_PIN_SPI_MOSI  13    // SPI MOSI
#define UWB_PIN_SPI_SCK   14    // SPI Clock
#define UWB_PIN_SPI_SS    15    // Chip Select
#define UWB_PIN_SPI_RST   16    // Reset
#define UWB_PIN_SPI_IRQ   17    // Interrupt

// Define the default antenna delay
#define DEFAULT_ANTENNA_DELAY 16150 // Default antenna delay in picoseconds

#endif