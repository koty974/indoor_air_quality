/* 
 * Read values from I2C (TWI) temperature/humidity sensor and send
 * them to UART.
 * (c) 2017-2024 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and AVR 8-bit Toolchain 3.6.2.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes -------------------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions (registers, I/O ports, etc.)
#include <avr/interrupt.h>  // Provides macros for enabling/disabling interrupts
#include "timer.h"          // Custom timer library (used to configure Timer1)
#include <twi.h>            // I2C/TWI (Two-Wire Interface) communication library
#include <uart.h>           // Peter Fleury's UART library for serial communication
#include <stdio.h>          // Standard C library (used here for sprintf formatting)


// -- Defines --------------------------------------------------------
#define DHT_ADR 0x5c      // I2C address of the DHT12 sensor
#define DHT_HUM_MEM 0     // Memory register address for humidity data
#define DHT_TEMP_MEM 2    // Memory register address for temperature data

//#define RTC_ADR 0x68     // (Commented) I2C address for Real-Time Clock (if used)
//#define RTC_SEC_MEM 0    // (Commented) Memory address for seconds register


// -- Global variables -----------------------------------------------
volatile uint8_t flag_update_uart = 0;  // Flag indicating that new sensor data is ready to send over UART
volatile uint8_t dht12_values[5];       // Array to store 5 bytes of data read from the DHT12 sensor


// -- Function definitions -------------------------------------------

/*
 * Function: timer1_init
 * Purpose:  Initialize Timer1 to overflow every 1 second and enable its interrupt.
 */
void timer1_init(void)
{
    tim1_ovf_1sec();      // Configure Timer1 to overflow every 1 second
    tim1_ovf_enable();    // Enable the Timer1 overflow interrupt
}



/*
 * Function: main
 * Purpose:  Main program loop — initializes peripherals, waits for new sensor data,
 *           and sends it via UART when ready.
 */
int main(void)
{
    char uart_msg[10];    // Temporary buffer for formatted text sent to UART

    // Initialize I2C (TWI) and UART communication
    twi_init();
    uart_init(UART_BAUD_SELECT(115200, F_CPU));  // 115200 baud rate

    sei();  // Enable global interrupts (necessary for UART and Timer ISR)

    // Check if the DHT12 sensor is connected and responding on the I2C bus
    if (twi_test_address(DHT_ADR) != 0)
    {
        // Print an error message in red color using ANSI escape codes
        uart_puts("[\x1b[31;1mERROR\x1b[0m] I2C device not detected\r\n");
        while (1);  // Stop program execution if sensor is not found
    }

    // Initialize the timer to generate periodic interrupts
    timer1_init();

    // Main program loop
    while (1)
    {
        // If the flag is set (data ready from sensor)
        if (flag_update_uart == 1)
        {
            // --- Display temperature ---
            // dht12_values[2] = integer part, [3] = decimal part of temperature
            sprintf(uart_msg, "%u.%u °C\t\t", dht12_values[2], dht12_values[3]);
            uart_puts(uart_msg);

            // --- Display humidity ---
            // dht12_values[0] = integer part, [1] = decimal part of humidity
            sprintf(uart_msg, "%u.%u %%\t\t", dht12_values[0], dht12_values[1]);
            uart_puts(uart_msg);

            // --- Display checksum ---
            // dht12_values[4] = checksum for verifying data integrity
            sprintf(uart_msg, "%u\r\n", dht12_values[4]);
            uart_puts(uart_msg);

            // Reset flag until new data is read again
            flag_update_uart = 0;
        }
    }

    // The program will never reach this point
    return 0;
}


// -- Interrupt service routines -------------------------------------

/*
 * ISR: Timer1 Overflow Interrupt
 * Purpose:  Executes every time Timer1 overflows (1 sec interval).
 *            Every 5 overflows (≈5 seconds), reads new data from the DHT12 sensor.
 */
ISR(TIMER1_OVF_vect)
{
    static uint8_t n_ovfs = 0;  // Counter for the number of overflows

    n_ovfs++;
    // Read sensor data every 5 seconds
    if (n_ovfs >= 5)
    {
        n_ovfs = 0;  // Reset overflow counter

        // Read 5 bytes starting from DHT_HUM_MEM (humidity register)
        // Data format:
        // [0] Humidity integer part
        // [1] Humidity decimal part
        // [2] Temperature integer part
        // [3] Temperature decimal part
        // [4] Checksum
        twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 5);

        // Set flag to notify main loop that data is ready to send
        flag_update_uart = 1;
    }
}



/*
 * The following commented-out sections are examples of how other I2C devices 
 * (like BME280 or MPU6050) could be used in a similar way.
 */

/*
    // Example: Reading chip ID from BME280 sensor (I2C address 0x76)
    sla = 0x76;
    twi_start();
    ack = twi_write((sla<<1) | TWI_WRITE);
    if (ack == 0) {       // Device responded
        twi_write(0xd0);  // Chip ID register
        twi_stop();
        twi_start();
        twi_write((sla<<1) | TWI_READ);
        uint8_t value = twi_read(TWI_NACK);
        twi_stop();

        // Send chip ID value to UART (in hex)
        itoa(value, string, 16);
        uart_puts(string);
        uart_puts("\t");
    }
*/

/*
    // Example: Resetting MPU-6050 accelerometer/gyroscope
    // twi_start();
    // twi_write((0x68<<1) | TWI_WRITE);
    // twi_write(0x6b);  // Power management register
    // twi_write(0x00);  // Clear sleep bit to wake the device
    // twi_stop();
*/
