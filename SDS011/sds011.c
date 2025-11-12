#include "sds011.h"
#include <string.h>
#include <unistd.h>  // for read() and write()

// === SDS011 protocol constants ===
#define SDS011_FRAME_LEN 10   // Data frame is always 10 bytes long
#define SDS011_HEADER 0xAA    // Start byte
#define SDS011_CMD_ID 0xC0    // Data frame identifier
#define SDS011_TAIL 0xAB      // End byte

// Initialize SDS011 structure
bool sds011_init(SDS011 *sensor, int uart_fd) {
    if (!sensor) return false;
    sensor->uart_fd = uart_fd;
    return true;
}

// Verify checksum (sum of bytes 2–7 must equal byte 8)
bool sds011_checksum_valid(const uint8_t *buf) {
    uint8_t sum = 0;
    for (int i = 2; i < 8; i++)
        sum += buf[i];
    return sum == buf[8];
}

// Read one 10-byte frame and extract PM2.5 / PM10 values
bool sds011_read(SDS011 *sensor, SDS011_Data *data) {
    uint8_t buf[SDS011_FRAME_LEN];

    // Read 10 bytes from the serial port
    if (read(sensor->uart_fd, buf, SDS011_FRAME_LEN) != SDS011_FRAME_LEN)
        return false;

    // Validate header, command ID, and tail
    if (buf[0] != SDS011_HEADER || buf[1] != SDS011_CMD_ID || buf[9] != SDS011_TAIL)
        return false;

    // Validate checksum
    if (!sds011_checksum_valid(buf))
        return false;

    // Parse raw 16-bit values (little endian)
    uint16_t pm25_raw = (buf[3] << 8) | buf[2];
    uint16_t pm10_raw = (buf[5] << 8) | buf[4];

    // Convert to micrograms per cubic meter
    data->pm25 = pm25_raw / 10.0f;
    data->pm10 = pm10_raw / 10.0f;

    return true;
}

// Send "request data" command (active query mode)
bool sds011_request_data(SDS011 *sensor) {
    // Command frame: 19 bytes total (AA B4 ... AB)
    // 0x04 = command ID for requesting measurement
    uint8_t cmd[19] = {
        0xAA, 0xB4, 0x04,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0xFF, 0xFF, 0x02, 0xAB
    };

    // Send command to UART — returns true if all bytes were written
    return write(sensor->uart_fd, cmd, sizeof(cmd)) == sizeof(cmd);
}