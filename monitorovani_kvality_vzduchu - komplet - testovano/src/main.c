// -- Includes -------------------------------------------------------
// Basic AVR libraries
#include <avr/io.h>         // Definitions of registers, ports and bits for AVR
#include <avr/interrupt.h>  // Macros for enabling/disabling interrupts
#include "timer.h"          // Custom library for timer configuration
#include <twi.h>            // I2C (TWI) communication
#include <uart.h>           // UART communication (Peter Fleury)
#include <stdio.h>          // sprintf and snprintf for formatted text
#include <oled.h>           // OLED display library
#include "gp2y1010.h"       // Sharp GP2Y1010 dust sensor library

// -- Defines --------------------------------------------------------
// I2C address of DHT12 sensor
#define DHT_ADR      0x5c
#define DHT_HUM_MEM  0      // Memory address for humidity
#define DHT_TEMP_MEM 2      // Memory address for temperature

// ADC pin for MQ135 gas sensor
#define MQ135_PIN 0         // ADC0 on Arduino Uno

// -- Global variables -----------------------------------------------
// Flag indicating that UART data needs to be updated
volatile uint8_t flag_update_uart = 0;

// Flag indicating that OLED display needs to be updated
volatile uint8_t flag_update_oled = 0;

// Array for storing DHT12 sensor values
volatile uint8_t dht12_values[5]; 
// [0] = humidity integer, [1] = humidity decimal
// [2] = temperature integer, [3] = temperature decimal
// [4] = checksum

// MQ135 sensor values
volatile uint16_t mq135_value = 0;    // Raw ADC value


// GP2Y1010 dust sensor values
volatile uint16_t dust_raw = 0;       // Raw ADC value
volatile float dust_voltage = 0;      // Converted voltage
volatile float dust_density = 0;      // Converted dust density in ug/m3

// GP2Y1010 sensor configuration structure
GP2Y1010 dust = {
    .ledPin = 2,    // PD2 digital pin for sensor LED control
    .analogPin = 1  // ADC1 for sensor analog output
};

// ------------------ OLED SETUP ------------------
void oled_setup(void)
{
    oled_init(OLED_DISP_ON);  // Initialize OLED and turn it on
    oled_clrscr();            // Clear display

    // Display heading in double size
    oled_charMode(DOUBLESIZE);
    oled_puts("INDOOR AIR Q");
    oled_charMode(NORMALSIZE); // Return to normal size

    // Labels for each measurement
    oled_gotoxy(0, 2);
    oled_puts("MQ135 ADC:");
    oled_gotoxy(0, 3);
    oled_puts("Level:");
    oled_gotoxy(0, 4);
    oled_puts("Temp [°C]:");
    oled_gotoxy(0, 5);
    oled_puts("Humidity [%]:");  // %% prints %
    oled_gotoxy(0, 6);
    oled_puts("Dust [ug/m3]:");

    oled_display();  // Transfer buffer to OLED RAM
}


// -------- ADC INITIALIZATION for MQ135 ---------
//void mq135_adc_init(void)
//{
//     Set ADC reference voltage to AVcc (5V)
//    ADMUX = (1 << REFS0);
//
//     Enable ADC and set prescaler to 128 (16 MHz / 128 = 125 kHz)
//    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
//}

// -------- ADC READ for MQ135 ---------
uint16_t mq135_read(void)
{
    // Select ADC channel 0 (A0)
    ADMUX = (ADMUX & 0xF0) | 0;  // 0 = ADC0

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait until conversion is complete
    while (ADCSRA & (1 << ADSC));

    // Return ADC value
    return ADC;
}


// Text description of MQ135 air quality
const char* get_quality_level(uint16_t value)
{
    if (value < 300) return "EXCELLENT";
    if (value < 500) return "GOOD";
    if (value < 700) return "MEDIUM";
    if (value < 900) return "BAD";
    return "VERY BAD";
}

// -------- TIMER1 (1s) INITIALIZATION --------
void timer1_init(void)
{
    tim1_ovf_1sec();    // Timer1 overflow every 1 second
    tim1_ovf_enable();  // Enable Timer1 overflow interrupt
}

// ------------------------ MAIN ------------------------
int main(void)
{
    // Buffers for formatted text
    char uart_msg[10];    
    char oled_msg[10];    

    // Initialize peripherals
    twi_init();                                   // I2C
    uart_init(UART_BAUD_SELECT(115200, F_CPU));  // UART 115200 baud
    //mq135_adc_init();                                   // mq135ADC
    oled_setup();                                 // OLED
    gp2y1010_init(&dust);                         // GP2Y1010 dust sensor

    // Timer0 for controlling GP2Y1010 LED (overflow every 16µs)
    tim0_ovf_16us();
    tim0_ovf_enable();

    sei();               // Enable global interrupts
    timer1_init();       // Start Timer1 for periodic measurements

    while (1)
    {
        // ---------------- UPDATE OLED ----------------
        if (flag_update_oled == 1)
        {
            

            // Display raw MQ135 ADC value
            oled_gotoxy(12,2);
            oled_puts("     ");                     
            oled_gotoxy(12,2);
            sprintf(oled_msg, "%4d", mq135_value);
            oled_puts(oled_msg);

            // Display textual quality level
            oled_gotoxy(8,3);
            oled_puts("             ");
            oled_gotoxy(8,3);
            oled_puts(get_quality_level(mq135_value));

            // Temperature from DHT12
            oled_gotoxy(13,4);
            oled_puts("        "); 
            oled_gotoxy(13,4);
            sprintf(oled_msg, "%u.%u C", dht12_values[2], dht12_values[3]);
            oled_puts(oled_msg);

            // Humidity from DHT12
            oled_gotoxy(13,5);
            oled_puts("        ");
            oled_gotoxy(13,5);
            sprintf(oled_msg, "%u.%u %% ", dht12_values[0], dht12_values[1]);
            oled_puts(oled_msg);

            // Dust concentration GP2Y1010
            oled_gotoxy(14,6);
            oled_puts("        ");
            oled_gotoxy(14,6);

            uint16_t dust_int = (uint16_t)(dust_density);
            uint16_t dust_dec = (uint16_t)((dust_density - dust_int) * 10);

            sprintf(oled_msg, "%u.%u", dust_int, dust_dec);
            oled_puts(oled_msg);

            oled_display();  // Refresh OLED content

            flag_update_oled = 0; // Reset flag
        }

        // ---------------- UPDATE UART ----------------
        if (flag_update_uart == 1)
        {
            // Temperature
            sprintf(uart_msg, "Temp: %u.%u C\r\n", dht12_values[2], dht12_values[3]);
            uart_puts(uart_msg);

            // Humidity
            sprintf(uart_msg, "Humidity: %u.%u %%\r\n", dht12_values[0], dht12_values[1]);
            uart_puts(uart_msg);

            // MQ135 raw and quality
            sprintf(uart_msg, "MQ135 raw=%u\r\n", mq135_value);
            uart_puts(uart_msg);

            // Dust value
            uint16_t dust_int = (uint16_t)dust_voltage;
            uint16_t dust_dec = (uint16_t)((dust_voltage - dust_int) * 10);
            sprintf(uart_msg, "Dust: %u.%u ug/m3\r\n\r\n", dust_int, dust_dec);
            uart_puts(uart_msg);

            flag_update_uart = 0; // Reset flag
        }
    }

    return 0; // Program never reaches this point
}

// ------------------------ ISR TIMER1 ------------------------
// Timer1 interrupt every 1 second
ISR(TIMER1_OVF_vect)
{
    static uint8_t counter = 0;  // Overflow counter
    counter++;

    // Perform measurements every 5 seconds
    if (counter >= 5)
    {
        counter = 0;

        // Read MQ135 sensor
        mq135_value = mq135_read();     ///MQ135_PIN

        // Read DHT12 via I2C (5 bytes)
        twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 5);

        // Read GP2Y1010 dust sensor
        dust_raw = gp2y1010_read_raw(&dust);
        dust_voltage = gp2y1010_adc_to_voltage(dust_raw);
        dust_density = gp2y1010_voltage_to_density(dust_voltage);

        // Request updates in main loop
        flag_update_oled = 1;
        flag_update_uart = 1;
    }
}







