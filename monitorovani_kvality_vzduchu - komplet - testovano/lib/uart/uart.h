#ifndef UART_H
# define UART_H

/************************************************************************
*  Title:    Interrupt UART library with receive/transmit circular buffers
*  Author:   Peter Fleury <pfleury@gmx.ch>  http://tinyurl.com/peterfleury
*  File:     $Id: uart.h,v 1.13 2015/01/11 13:53:25 peter Exp $
*  Software: AVR-GCC 4.x, AVR Libc 1.4 or higher
*  Hardware: any AVR with built-in UART/USART
*
*  DESCRIPTION:
*   This is the header file for an interrupt-driven UART communication library.
*   It provides function declarations and macros to send/receive data through
*   the AVR’s built-in USART (serial port).
*
*   Reception and transmission are buffered using *circular buffers*, meaning:
*   - Data can be received in the background (via interrupts)
*   - The main program can process or send data asynchronously
*
*   Works on most AVR MCUs with a UART, e.g. ATmega328P.
*
************************************************************************/

#include <avr/pgmspace.h>   // Allows reading strings stored in Flash (PROGMEM)

// -----------------------------------------------------------------------------
//  Compiler version check
// -----------------------------------------------------------------------------
#if (__GNUC__ * 100 + __GNUC_MINOR__) < 405
# error "This library requires AVR-GCC 4.5 or later."
#endif


// -----------------------------------------------------------------------------
//  UART Baud Rate Macros
// -----------------------------------------------------------------------------
/**
 * @brief Calculate value for UBRR register (normal mode)
 *
 * This macro computes the baud rate setting for the UART.
 *
 * @param baudRate Desired baud rate (bps)
 * @param xtalCpu  CPU clock frequency (Hz), e.g. 16000000UL for 16 MHz
 *
 * Example:
 * @code
 * uart_init(UART_BAUD_SELECT(9600, F_CPU));
 * @endcode
 */
#define UART_BAUD_SELECT(baudRate, xtalCpu) \
    (((xtalCpu) + 8UL * (baudRate)) / (16UL * (baudRate)) - 1UL)

/**
 * @brief Calculate value for UBRR register (double speed mode, U2X)
 *
 * Double speed mode uses 8× the baud clock instead of 16× for finer adjustment.
 *
 * The return value has bit 15 set (0x8000) to indicate that double-speed mode
 * should be enabled inside uart_init().
 */
#define UART_BAUD_SELECT_DOUBLE_SPEED(baudRate, xtalCpu) \
    ((((xtalCpu) + 4UL * (baudRate)) / (8UL * (baudRate)) - 1UL) | 0x8000)


// -----------------------------------------------------------------------------
//  Buffer Sizes (must be powers of 2)
// -----------------------------------------------------------------------------
#ifndef UART_RX_BUFFER_SIZE
# define UART_RX_BUFFER_SIZE 64   /**< Default RX buffer size */
#endif

#ifndef UART_TX_BUFFER_SIZE
# define UART_TX_BUFFER_SIZE 64   /**< Default TX buffer size */
#endif

// Verify that buffer sizes fit within SRAM limits
#if ((UART_RX_BUFFER_SIZE + UART_TX_BUFFER_SIZE) >= (RAMEND - 0x60))
# error "UART buffer sizes too large for available SRAM."
#endif


// -----------------------------------------------------------------------------
//  Error Codes (returned in high byte of uart_getc())
// -----------------------------------------------------------------------------
#define UART_FRAME_ERROR     0x1000 /**< Framing error by UART       */
#define UART_OVERRUN_ERROR   0x0800 /**< Overrun condition (data lost) */
#define UART_PARITY_ERROR    0x0400 /**< Parity error by UART        */
#define UART_BUFFER_OVERFLOW 0x0200 /**< Receive ring buffer overflow */
#define UART_NO_DATA         0x0100 /**< No receive data available   */


// -----------------------------------------------------------------------------
//  Function Prototypes
// -----------------------------------------------------------------------------

/**
 * @brief Initialize UART and set baud rate.
 *
 * Example:
 * @code
 * uart_init(UART_BAUD_SELECT(9600, F_CPU));
 * @endcode
 */
extern void uart_init(unsigned int baudrate);

/**
 * @brief Receive one byte from the UART receive buffer.
 *
 * @return 16-bit value:
 *   - low byte: received character
 *   - high byte: error code (0 = OK)
 *
 * Example:
 * @code
 * unsigned int rx = uart_getc();
 * if (!(rx & 0xFF00)) {   // no error
 *     char c = rx & 0xFF;
 * }
 * @endcode
 */
extern unsigned int uart_getc(void);

/**
 * @brief Transmit one byte through UART.
 * @param data Character to send.
 */
extern void uart_putc(unsigned char data);

/**
 * @brief Transmit a null-terminated string from RAM.
 * @param s Pointer to string (in RAM)
 *
 * Example:
 * @code
 * uart_puts("Hello World!\r\n");
 * @endcode
 */
extern void uart_puts(const char *s);

/**
 * @brief Transmit a string stored in program memory (PROGMEM).
 * @param s Pointer to string stored in Flash.
 *
 * Example:
 * @code
 * uart_puts_p(PSTR("Flash message\r\n"));
 * @endcode
 */
extern void uart_puts_p(const char *s);

/**
 * @brief Macro to automatically place string constant into program memory.
 *
 * This lets you write `uart_puts_P("Hello")` instead of using PSTR manually.
 */
#define uart_puts_P(__s) uart_puts_p(PSTR(__s))


// -----------------------------------------------------------------------------
//  Secondary UART (USART1) – for MCUs with multiple UARTs (e.g. ATmega128)
// -----------------------------------------------------------------------------
extern void uart1_init(unsigned int baudrate);
extern unsigned int uart1_getc(void);
extern void uart1_putc(unsigned char data);
extern void uart1_puts(const char *s);
extern void uart1_puts_p(const char *s);
#define uart1_puts_P(__s) uart1_puts_p(PSTR(__s))


#endif // UART_H
