#ifndef GP2Y1010_H
#define GP2Y1010_H

#include <avr/io.h>
#include <util/delay.h>

typedef struct {
    uint8_t ledPin;      // PORTD pin number
    uint8_t analogPin;   // ADC channel number 0..7
} GP2Y1010;

void gp2y1010_init(GP2Y1010 *s);
uint16_t gp2y1010_read_raw(GP2Y1010 *s);
float gp2y1010_adc_to_voltage(uint16_t adc);
float gp2y1010_voltage_to_density(float volt);

#endif
