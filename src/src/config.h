#ifndef CONFIG_H
#define CONFIG_H

#define BUTTON_PIN  0   // User Button
#define LED_PIN     1   // Status LED
#define BOOT_PIN    9   // Boot Mode Pin

// Define the pin configuration for DWM1000
#define UWB_PIN_SPI_IRQ     3   // Interrupt
#define UWB_PIN_SPI_SCK     4   // SPI Clock
#define UWB_PIN_SPI_MISO    5   // SPI MISO
#define UWB_PIN_SPI_MOSI    6   // SPI MOSI
#define UWB_PIN_SPI_SS      7   // Chip Select
#define UWB_PIN_SPI_RST     10  // Reset

// Define the default antenna delay
#define DEFAULT_ANTENNA_DELAY 16150 // Default antenna delay in picoseconds

#endif