/*
 * CalSTAR Avionics Ground Station Test: Receiver
 *
 * file: main.cpp
 *
 * Prints anything it receives over radio to UART.
 * Use to verify that the Ground Station can receive properly.
 */

#include "RFM69/RFM69.hpp"
#include "mbed.h"

#define SPI1_MOSI (PA_7)
#define SPI1_MISO (PA_6)
#define SPI1_SCLK (PA_5)
#define SPI1_SSEL (PA_4)
#define RADIO_RST (PC_1)

#define ENCRYPT_KEY "CALSTARENCRYPTKE"

#define BAUDRATE (115200)

#define UART6_RX (PC_7)
#define UART6_TX (PC_6)

// #define UART6_RX (PA_3)
// #define UART6_TX (PA_2)


int main() {
  Serial pc(UART6_TX, UART6_RX);
  pc.baud(BAUDRATE);
  pc.set_blocking(false);

  RFM69 radio(SPI1_MOSI, SPI1_MISO, SPI1_SCLK, SPI1_SSEL, RADIO_RST, true);
  radio.reset();
  pc.printf("radio reset\r\n");

  radio.init();
  pc.printf("radio init'd\r\n");

  radio.setAESEncryption(ENCRYPT_KEY, strlen(ENCRYPT_KEY));
  radio.sleep();

  radio.setPowerDBm(10);
  pc.printf("radio high powered\r\n");

  radio.setCSMA(true);

  char rx[64];
  while (true) {
    int bytes_rxd = radio.receive(rx, sizeof(rx));
    if (bytes_rxd > 1) {
      rx[bytes_rxd] = '\0';
      char *data = &rx[1];
      pc.printf("[rssi: %d, bytes received: %d] %s\r\n", radio.getRSSI(), bytes_rxd - 1, data);
    }
  }
  return 0;
}
