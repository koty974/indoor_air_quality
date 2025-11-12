#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include "dht.h"

// Convert integer (0-255) to decimal string and send via UART
void uart_put_uint8(uint8_t value)
{
    char hundreds = '0', tens = '0', units = '0';

    if (value >= 100)
    {
        hundreds = '0' + value / 100;
        value %= 100;
    }

    if (value >= 10)
    {
        tens = '0' + value / 10;
        value %= 10;
    }

    units = '0' + value;

    if (hundreds != '0') uart_putc(hundreds);
    if (hundreds != '0' || tens != '0') uart_putc(tens);
    uart_putc(units);
}

// Send string via UART
void uart_puts_ln(const char *s)
{
    while (*s) uart_putc(*s++);
    uart_puts("\r\n");
}

int main(void)
{
    uint8_t temperature = 0;
    uint8_t humidity = 0;

    // Initialize UART and DHT sensor
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    dht_init();

    // Enable global interrupts
    sei();

    while (1)
    {
        if (dht_read(&temperature, &humidity) == DHT_OK)
        {
            uart_puts("Temp: ");
            uart_put_uint8(temperature);
            uart_puts(" C, Hum: ");
            uart_put_uint8(humidity);
            uart_puts_ln(" %");
        }
        else
        {
            uart_puts_ln("DHT read error!");
        }

        _delay_ms(2000);
    }

    return 0;
}

