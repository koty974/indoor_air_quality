#include "gp2y1010.h"

void gp2y1010_init(GP2Y1010 *s) {
    // LED pin as output (PORTD)
    DDRD |= (1 << s->ledPin);
    PORTD |= (1 << s->ledPin); // LED OFF (active LOW)

    // ADC setup
    ADMUX = (1 << REFS0); // AVcc reference
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // Prescaler 64
}

uint16_t gp2y1010_read_raw(GP2Y1010 *s) {

    // LED ON (active low)
    PORTD &= ~(1 << s->ledPin);
    _delay_us(280);

    // Select ADC channel
    ADMUX = (ADMUX & 0xF0) | (s->analogPin & 0x0F);

    // Start ADC
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    uint16_t adc = ADC;

    _delay_us(40);

    // LED OFF
    PORTD |= (1 << s->ledPin);
    _delay_us(9680);

    return adc;
}

float gp2y1010_adc_to_voltage(uint16_t adc) {
    return adc * (5.0f / 1023.0f);
}

float gp2y1010_voltage_to_density(float volt) {
    if (volt < 0.1f) return 0.0f;
    return (volt - 0.1f) * 0.5f * 1000.0f;
}
