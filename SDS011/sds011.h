#ifndef SDS011_H
#define SDS011_H

#include <stdint.h>
#include <stdbool.h>

// Structure to hold the measured PM2.5 and PM10 values
typedef struct {
    float pm25;   // PM2.5 value in µg/m³
    float pm10;   // PM10 value in µg/m³
} SDS011_Data;

// Structure representing an SDS011 sensor device
typedef struct {
    int uart_fd;  // File descriptor for UART communication
} SDS011;

// === Function prototypes ===

// Initialize the SDS011 structure with a UART file descriptor
bool sds011_init(SDS011 *sensor, int uart_fd);

// Read a 10-byte data frame from the sensor and parse PM2.5 and PM10 values
bool sds011_read(SDS011 *sensor, SDS011_Data *data);

// Send a command to the sensor requesting new measurement data
bool sds011_request_data(SDS011 *sensor);

// Check if a received frame has a valid checksum
bool sds011_checksum_valid(const uint8_t *buf);

#endif // SDS011_H