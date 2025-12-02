#ifndef TWI_H
#define TWI_H
/*
 * I2C/TWI library for AVR-GCC.
 * (c) 2018–2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno (ATmega328P, 16 MHz).
 *
 * Provides low-level control of the AVR’s hardware TWI (I2C) interface.
 * Implements basic Master Transmit and Master Receive functions.
 */

/**
 * @file 
 * @defgroup fryza_twi TWI Library <twi.h>
 * @code #include <twi.h> @endcode
 *
 * @brief I2C/TWI library for AVR-GCC.
 *
 * This library defines functions for communication between an AVR microcontroller
 * (as a Master) and one or more Slave devices using the TWI/I2C protocol.
 *
 * Only master modes (transmit/receive) are implemented. It relies on
 * the AVR’s internal TWI hardware registers.
 *
 * @note Based on the Microchip (Atmel) ATmega16/ATmega328P datasheets.
 */

#include <avr/io.h>  // Provides register definitions for the specific AVR MCU


// -----------------------------------------------------------------------------
//  Clock and frequency definitions
// -----------------------------------------------------------------------------

/**
 * @name Definition of frequencies 
 */
#ifndef F_CPU
#define F_CPU 16000000 /**< Default CPU frequency in Hz (required for TWI_BIT_RATE_REG) */
#endif

#define F_SCL 100000 /**< I2C/TWI bit rate in Hz (standard mode: 100 kHz).
                          Must be greater than ~31 kHz for proper operation. */

/**
 * Formula for TWI bit rate:
 *     SCL_freq = F_CPU / (16 + 2 * TWBR * Prescaler)
 * TWI_BIT_RATE_REG computes the correct TWBR value given F_CPU and F_SCL.
 */
#define TWI_BIT_RATE_REG ((F_CPU/F_SCL - 16) / 2)


// -----------------------------------------------------------------------------
//  Port and pin configuration for SDA and SCL lines
// -----------------------------------------------------------------------------

/**
 * @name Definition of ports and pins
 */
#define TWI_PORT PORTC  /**< Port connected to the TWI interface (SDA, SCL) */
#define TWI_SDA_PIN 4   /**< SDA (Serial Data) pin */
#define TWI_SCL_PIN 5   /**< SCL (Serial Clock) pin */


// -----------------------------------------------------------------------------
//  Other helpful macros
// -----------------------------------------------------------------------------

/**
 * @name Other definitions
 */
#define TWI_WRITE 0 /**< Mode bit for writing to an I2C device (R/W bit = 0) */
#define TWI_READ 1  /**< Mode bit for reading from an I2C device (R/W bit = 1) */
#define TWI_ACK 0   /**< Send ACK after reading (means “continue reading”) */
#define TWI_NACK 1  /**< Send NACK after reading (means “stop reading”) */

/**
 * AVR trick macros: convert a PORT register name to its corresponding
 * Data Direction (DDR) and PIN registers.
 * 
 * Example:
 *   DDR(PORTC) → DDRC
 *   PIN(PORTC) → PINC
 * 
 * These work because of the way the I/O registers are organized in memory:
 *   DDRx is always located one address below PORTx.
 *   PINx is always located two addresses below PORTx.
 */
#define DDR(_x) (*(&_x - 1))
#define PIN(_x) (*(&_x - 2))


// -----------------------------------------------------------------------------
//  Function prototypes
// -----------------------------------------------------------------------------

/**
 * @brief Initialize the TWI module.
 *
 * - Configures SDA and SCL as inputs with pull-ups.
 * - Sets the bit rate register (TWBR) to generate desired SCL frequency.
 */
void twi_init(void);


/**
 * @brief Send a START condition to begin communication.
 *
 * Places the TWI bus in a busy state and prepares to send an address byte.
 */
void twi_start(void);


/**
 * @brief Send one byte (address or data) on the TWI bus.
 *
 * @param data  The byte to transmit.
 * @return 0 if ACK received, 1 if NACK.
 *
 * @note
 * Returns 0 (ACK) if TWI status register indicates:
 *  - 0x18: SLA+W transmitted, ACK received  
 *  - 0x28: Data byte transmitted, ACK received  
 *  - 0x40: SLA+R transmitted, ACK received
 */
uint8_t twi_write(uint8_t data);


/**
 * @brief Read one byte from the TWI bus.
 *
 * @param ack  Whether to send ACK (0) or NACK (1) after receiving the byte.
 * @return     The received byte.
 */
uint8_t twi_read(uint8_t ack);


/**
 * @brief Generate a STOP condition, releasing the bus.
 */
void twi_stop(void);


/**
 * @brief Check if a device acknowledges its I2C address.
 *
 * @param addr  7-bit slave address.
 * @return 0 if ACK received (device present), 1 if NACK (device missing).
 */
uint8_t twi_test_address(uint8_t addr);


/**
 * @brief Read multiple bytes from a slave device starting at a memory/register address.
 *
 * @param addr     Slave address.
 * @param memaddr  Starting memory/register address.
 * @param buf      Pointer to buffer for received data.
 * @param nbytes   Number of bytes to read.
 *
 * Performs the following sequence:
 *   1. Send START + SLA+W  
 *   2. Send memory address  
 *   3. Send repeated START + SLA+R  
 *   4. Read N bytes (ACK for all but last, NACK for last)  
 *   5. Send STOP  
 */
void twi_readfrom_mem_into(uint8_t addr, uint8_t memaddr, volatile uint8_t *buf, uint8_t nbytes);

/** @} */  // End of doxygen documentation group

#endif
