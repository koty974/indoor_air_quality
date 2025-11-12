#ifndef DHT_H
#define DHT_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/**
 * DHT11 sensor interface
 * Uses a single-wire digital protocol
 *
 * Example usage:
 *   dht_init();
 *   if(dht_read(&temperature, &humidity) == 0) {
 *       // successful read
 *   }
 */

// Select pin: change these according to your wiring
#define DHT_PORT PORTD
#define DHT_PIN  PIND
#define DHT_DDR  DDRD
#define DHT_BIT  PD2  // Using digital pin 2

// Return codes
#define DHT_OK             0
#define DHT_ERROR_TIMEOUT -1
#define DHT_ERROR_CHECKSUM -2

// Function prototypes
void dht_init(void);
int dht_read(uint8_t* temperature, uint8_t* humidity);

#endif
