/*
 * I2C/TWI library for AVR-GCC.
 * (c) 2018-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes ---------------------------------------------
#include <twi.h>   // Header file with macros and function declarations


// -- Functions --------------------------------------------

/*
 * Function: twi_init()
 * Purpose:  Initialize TWI unit, enable internal pull-ups, and set SCL
 *           frequency.
 * Returns:  none
 */
void twi_init(void)
{
    /* Enable internal pull-up resistors on SDA and SCL lines.
       This ensures the I2C bus stays in an idle HIGH state when not driven. */
    DDR(TWI_PORT) &= ~((1<<TWI_SDA_PIN) | (1<<TWI_SCL_PIN));  // Set SDA & SCL as inputs
    TWI_PORT |= (1<<TWI_SDA_PIN) | (1<<TWI_SCL_PIN);          // Enable pull-ups

    /* Set SCL frequency:
       Formula: SCL_freq = F_CPU / (16 + 2*TWBR*Prescaler)
       Here we clear prescaler bits and set TWBR based on desired speed. */
    TWSR &= ~((1<<TWPS1) | (1<<TWPS0));   // Prescaler = 1
    TWBR = TWI_BIT_RATE_REG;               // Set bit rate register
}


/*
 * Function: twi_start()
 * Purpose:  Generate a START condition on the I2C bus.
 *           Used to signal the beginning of a communication sequence.
 */
void twi_start(void)
{
    /* TWINT = 1: clear interrupt flag
       TWSTA = 1: send START condition
       TWEN  = 1: enable TWI */
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

    /* Wait until the START condition has been transmitted */
    while ((TWCR & (1<<TWINT)) == 0);
}


/*
 * Function: twi_write()
 * Purpose:  Send one byte (address or data) on the I2C bus.
 * Input:    data - byte to transmit
 * Returns:  0 if ACK received (success)
 *           1 if NACK received (failure)
 */
uint8_t twi_write(uint8_t data)
{
    uint8_t twi_status;

    /* Load data (address or data byte) into the data register */
    TWDR = data;

    /* Start transmission */
    TWCR = (1<<TWINT) | (1<<TWEN);

    /* Wait until transmission completes */
    while ((TWCR & (1<<TWINT)) == 0);

    /* Mask prescaler bits to read TWI status code (upper 5 bits only) */
    twi_status = TWSR & 0xf8;

    /* Check if ACK was received:
         0x18 = SLA+W transmitted, ACK received
         0x28 = Data byte transmitted, ACK received
         0x40 = SLA+R transmitted, ACK received */
    if (twi_status == 0x18 || twi_status == 0x28 || twi_status == 0x40)
        return 0;   // ACK received
    else
        return 1;   // NACK received
}


/*
 * Function: twi_read()
 * Purpose:  Read one byte from I2C bus and send ACK or NACK after reception.
 * Input:    ack - determines whether to acknowledge (ACK) or not (NACK)
 * Returns:  The received data byte.
 */
uint8_t twi_read(uint8_t ack)
{
    if (ack == TWI_ACK)
        // Send ACK after receiving the byte (want to continue reading)
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    else
        // Send NACK after receiving the byte (last byte)
        TWCR = (1<<TWINT) | (1<<TWEN);

    // Wait for reception to complete
    while ((TWCR & (1<<TWINT)) == 0);

    return (TWDR);  // Return received byte
}


/*
 * Function: twi_stop()
 * Purpose:  Generate a STOP condition to release the I2C bus.
 * Returns:  none
 */
void twi_stop(void)
{
    /* TWSTO = 1: send STOP
       TWINT = 1: clear interrupt flag
       TWEN  = 1: keep TWI enabled */
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}


/*
 * Function: twi_test_address()
 * Purpose:  Check if a device at given I2C address responds with ACK.
 * Input:    addr - 7-bit I2C slave address
 * Returns:  0 if ACK received (device present)
 *           1 if NACK received (device not responding)
 */
uint8_t twi_test_address(uint8_t addr)
{
    uint8_t ack;  // Holds the ACK result

    twi_start();                            // Begin communication
    ack = twi_write((addr<<1) | TWI_WRITE); // Send SLA+W (address + write bit)
    twi_stop();                             // Stop communication

    return ack;
}


/*
 * Function: twi_readfrom_mem_into()
 * Purpose:  Read multiple bytes starting from a memory address of a slave device.
 * Input:    addr    - 7-bit I2C slave address
 *           memaddr - internal memory/register address to start from
 *           buf     - pointer to buffer to store received bytes
 *           nbytes  - number of bytes to read
 * Returns:  none
 */
void twi_readfrom_mem_into(uint8_t addr, uint8_t memaddr, volatile uint8_t *buf, uint8_t nbytes)
{
    twi_start();
    if (twi_write((addr<<1) | TWI_WRITE) == 0)  // Send slave address with Write
    {
        // Tell slave which internal memory/register address we want to read from
        twi_write(memaddr);
        twi_stop();

        // Generate a repeated start to switch from write mode to read mode
        twi_start();
        twi_write((addr<<1) | TWI_READ);  // Send slave address with Read

        // If reading multiple bytes, ACK each except the last
        if (nbytes >= 2)
        {
            for (uint8_t i = 0; i < (nbytes - 1); i++)
            {
                *buf++ = twi_read(TWI_ACK);  // Read byte and acknowledge
            }
        }

        // Read last byte and send NACK to signal end of transmission
        *buf = twi_read(TWI_NACK);
        twi_stop();
    }
    else
    {
        // If device didn't acknowledge, end communication
        twi_stop();
    }
}
