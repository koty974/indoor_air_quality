#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "gp2y1010.h"

void uart_init() {
    uint16_t ubrr = 103;           // 9600 baud for 16MHz

    UBRR0H = (ubrr >> 8);          // Set baud high byte
    UBRR0L = (ubrr & 0xFF);        // Set baud low byte

    UCSR0B = (1 << TXEN0);         // Enable TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   // 8-bit, no parity, 1 stop
}

void uart_send(char c) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait for data register empty
    UDR0 = c;
}

void uart_print(const char *s) {
    while (*s) uart_send(*s++);
}

GP2Y1010 dust = {
    .ledPin = 2,     // PD2
    .analogPin = 1   // ADC0
};

int main() {
    uart_init();
    gp2y1010_init(&dust);

    char buffer[64];

    while (1) {
        uint16_t raw = gp2y1010_read_raw(&dust);
        float voltage = gp2y1010_adc_to_voltage(raw);
        float dust_u = gp2y1010_voltage_to_density(voltage);

        snprintf(buffer, sizeof(buffer),
                 "RAW=%u  V=%.3f  Dust=%.1f ug/m3\r\n",
                 raw, voltage, dust_u);

        uart_print(buffer);

        _delay_ms(500);
    }
}
