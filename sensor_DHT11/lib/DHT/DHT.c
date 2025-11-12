#include "dht.h"

/**
 * @brief Initialize DHT11 pin
 */
void dht_init(void)
{
    // Set pin as input (high by default)
    DHT_DDR &= ~(1 << DHT_BIT);
    DHT_PORT |= (1 << DHT_BIT);  // pull-up
}

/**
 * @brief Read data from DHT11
 * @param temperature Pointer to store temperature (°C)
 * @param humidity Pointer to store humidity (%)
 * @return DHT_OK on success, negative on error
 */
int dht_read(uint8_t* temperature, uint8_t* humidity)
{
    uint8_t data[5] = {0,0,0,0,0};
    uint8_t i, j;

    // ---- Send start signal ----
    DHT_DDR |= (1 << DHT_BIT);   // output
    DHT_PORT &= ~(1 << DHT_BIT); // pull low
    _delay_ms(20);               // 18-20ms minimum
    DHT_PORT |= (1 << DHT_BIT);  // release
    _delay_us(40);               // wait 20-40us
    DHT_DDR &= ~(1 << DHT_BIT);  // input

    // ---- Wait for DHT response ----
    uint16_t count = 0;
    while((DHT_PIN & (1 << DHT_BIT)) && count++ < 1000) _delay_us(1);
    if(count >= 1000) return DHT_ERROR_TIMEOUT;

    count = 0;
    while(!(DHT_PIN & (1 << DHT_BIT)) && count++ < 1000) _delay_us(1);
    if(count >= 1000) return DHT_ERROR_TIMEOUT;

    count = 0;
    while((DHT_PIN & (1 << DHT_BIT)) && count++ < 1000) _delay_us(1);
    if(count >= 1000) return DHT_ERROR_TIMEOUT;

    // ---- Read 40 bits (5 bytes) ----
    for (j=0; j<5; j++)
    {
        for (i=0; i<8; i++)
        {
            // wait for pin to go high
            count = 0;
            while(!(DHT_PIN & (1 << DHT_BIT)) && count++ < 1000) _delay_us(1);
            if(count >= 1000) return DHT_ERROR_TIMEOUT;

            // measure high pulse width
            _delay_us(30);  // 26-28us = 0, 70us = 1
            if(DHT_PIN & (1 << DHT_BIT)) data[j] |= (1 << (7-i));

            // wait for pin to go low
            count = 0;
            while((DHT_PIN & (1 << DHT_BIT)) && count++ < 1000) _delay_us(1);
            if(count >= 1000) return DHT_ERROR_TIMEOUT;
        }
    }

    // ---- Verify checksum ----
    if(data[0] + data[1] + data[2] + data[3] != data[4])
        return DHT_ERROR_CHECKSUM;

    *humidity    = data[0];
    *temperature = data[2];

    return DHT_OK;
}
