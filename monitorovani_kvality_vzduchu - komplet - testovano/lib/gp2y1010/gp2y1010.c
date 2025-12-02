
#include "gp2y1010.h"
#include "timer.h"             // Timer0 macros
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t last_raw = 0;

// State machine timing
// Timer0 overflow period: 16 µs
// GP2Y1010 timing (datasheet):
// LED ON 280 µs → 18 ticks
// Extra 40 µs → 2 ticks
// LED OFF 9.7 ms → 605 ticks
volatile uint8_t state = 0;
volatile uint16_t ticks = 0;

void gp2y1010_init(GP2Y1010 *s) {
    // Configure LED pin as output
    DDRD |= (1 << s->ledPin);
    
    ADMUX = (1 << REFS0);


    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

}

// Timer0 Overflow Interrupt Service Routine
ISR(TIMER0_OVF_vect) {
    ticks++;

    switch (state) {

    case 0: // LED ON for 280 µs
        PORTD |= ~(1 << PD2);
        

        if (ticks >= 18) {
            ticks = 0;
            state = 1;
        }
        break;

    case 1: // LED still ON for 40 µs, sample ADC
        if (ticks == 1) {
            // Select ADC channel 1
            ADMUX = (ADMUX & 0xF0) | 1;

            // Start ADC conversion
            ADCSRA |= (1 << ADSC);
            while (ADCSRA & (1 << ADSC));

            last_raw = ADC;
        }

        if (ticks >= 2) {
            PORTD &= (1 << PD2); // Turn LED OFF
            ticks = 0;
            state = 2;
        }
        break;

    case 2: // LED OFF ~9.7 ms
        if (ticks >= 605) {
            ticks = 0;
            state = 0;
        }
        break;
    }
}

uint16_t gp2y1010_read_raw(GP2Y1010 *s) {
    (void)s; // unused
    return last_raw;
}

float gp2y1010_adc_to_voltage(uint16_t raw) {
    return (raw * 5.0f) / 1023.0f;
}

float gp2y1010_voltage_to_density(float v) {
    float v0 = 0.1f;  // change value based on calibration
    if (v < v0) return 0.0f;
    return (v - v0) / 0.005f;   // 0.005V ~ 1 ug/m3
}

