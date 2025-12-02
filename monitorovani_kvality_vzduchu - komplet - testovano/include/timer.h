#ifndef TIMER_H
# define TIMER_H

/* 
 * Timer library for AVR-GCC.
 * (c) 2019-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and AVR 8-bit Toolchain.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

/**
 * @file 
 * @defgroup fryza_timer Timer Library <timer.h>
 * @code #include <timer.h> @endcode
 *
 * @brief Timer library for AVR-GCC.
 *
 * This header-only library provides a set of macros to configure and
 * control the AVR 8-bit and 16-bit timer/counters (Timer0, Timer1, Timer2).
 * 
 * Macros allow quick setup of timer overflow periods and enable/disable
 * overflow interrupts, useful for periodic operations, time delays, or
 * cooperative task scheduling.
 *
 * @note Based on Microchip Atmel ATmega328P manual.
 *       No separate source (.c) file is required.
 * @copyright (c) 2019-2025 Tomas Fryza, MIT license
 * @{
 */

// -- Includes -------------------------------------------------------
#include <avr/io.h>


// ===================================================================
//  16-bit Timer/Counter1 Configuration
// ===================================================================
/**
 * @name  Timer/Counter1 (16-bit)
 * @note  Overflow time: t_OVF = (1 / F_CPU) * prescaler * 2^16
 *        with F_CPU = 16 MHz
 * @{
 */

/**
 * @brief Stop Timer1 by clearing all clock select bits (CS12:0 = 000).
 */
#define tim1_stop() TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));

/**
 * @brief Set Timer1 overflow period to approximately 4 ms (no prescaler, CS10 = 1).
 */
#define tim1_ovf_4ms() TCCR1B &= ~((1<<CS12) | (1<<CS11)); TCCR1B |= (1<<CS10);

/**
 * @brief Set Timer1 overflow period to approximately 33 ms (prescaler = 8, CS11 = 1).
 */
#define tim1_ovf_33ms() TCCR1B &= ~((1<<CS12) | (1<<CS10)); TCCR1B |= (1<<CS11);

/**
 * @brief Set Timer1 overflow period to approximately 262 ms (prescaler = 64).
 */
#define tim1_ovf_262ms() TCCR1B &= ~(1<<CS12); TCCR1B |= (1<<CS11) | (1<<CS10);

/**
 * @brief Set Timer1 overflow period to approximately 1 second (prescaler = 256).
 */
#define tim1_ovf_1sec() TCCR1B &= ~((1<<CS11) | (1<<CS10)); TCCR1B |= (1<<CS12);

/**
 * @brief Set Timer1 overflow period to approximately 4 seconds (prescaler = 1024).
 */
#define tim1_ovf_4sec() TCCR1B &= ~(1<<CS11); TCCR1B |= (1<<CS12) | (1<<CS10);

/**
 * @brief Enable Timer1 overflow interrupt (TOIE1 = 1).
 */
#define tim1_ovf_enable() TIMSK1 |= (1<<TOIE1);

/**
 * @brief Disable Timer1 overflow interrupt (TOIE1 = 0).
 */
#define tim1_ovf_disable() TIMSK1 &= ~(1<<TOIE1);

/** @} */



// ===================================================================
//  8-bit Timer/Counter0 Configuration
// ===================================================================
/**
 * @name  Timer/Counter0 (8-bit)
 * @note  Overflow time: t_OVF = (1 / F_CPU) * prescaler * 2^8
 *        with F_CPU = 16 MHz
 * @{
 */

/**
 * @brief Stop Timer0 by clearing all clock select bits (CS02:0 = 000).
 */
#define tim0_stop() TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));

/**
 * @brief Set Timer0 overflow period to approximately 16 µs (no prescaler).
 */
#define tim0_ovf_16us() TCCR0B &= ~((1<<CS02) | (1<<CS01)); TCCR0B |= (1<<CS00);

/**
 * @brief Set Timer0 overflow period to approximately 128 µs (prescaler = 8).
 */
#define tim0_ovf_128us() TCCR0B &= ~((1<<CS02) | (1<<CS00)); TCCR0B |= (1<<CS01);

/**
 * @brief Set Timer0 overflow period to approximately 1 ms (prescaler = 64).
 */
#define tim0_ovf_1ms() TCCR0B &= ~(1<<CS02); TCCR0B |= (1<<CS01) | (1<<CS00);

/**
 * @brief Set Timer0 overflow period to approximately 4 ms (prescaler = 256).
 */
#define tim0_ovf_4ms() TCCR0B &= ~((1<<CS01) | (1<<CS00)); TCCR0B |= (1<<CS02);

/**
 * @brief Set Timer0 overflow period to approximately 16 ms (prescaler = 1024).
 */
#define tim0_ovf_16ms() TCCR0B &= ~(1<<CS01); TCCR0B |= (1<<CS02) | (1<<CS00);

/**
 * @brief Enable Timer0 overflow interrupt (TOIE0 = 1).
 */
#define tim0_ovf_enable() TIMSK0 |= (1<<TOIE0);

/**
 * @brief Disable Timer0 overflow interrupt (TOIE0 = 0).
 */
#define tim0_ovf_disable() TIMSK0 &= ~(1<<TOIE0);

/** @} */



// ===================================================================
//  8-bit Timer/Counter2 Configuration
// ===================================================================
/**
 * @name  Timer/Counter2 (8-bit)
 * @note  Overflow time: t_OVF = (1 / F_CPU) * prescaler * 2^8
 *        with F_CPU = 16 MHz
 * @{
 */

/**
 * @brief Stop Timer2 by clearing all clock select bits (CS22:0 = 000).
 */
#define tim2_stop() TCCR2B &= ~((1<<CS22) | (1<<CS21) | (1<<CS20));

/**
 * @brief Set Timer2 overflow period to approximately 16 ms (prescaler = 1024).
 */
#define tim2_ovf_16ms() TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);

/**
 * @brief Enable Timer2 overflow interrupt (TOIE2 = 1).
 */
#define tim2_ovf_enable() TIMSK2 |= (1<<TOIE2);

/**
 * @brief Disable Timer2 overflow interrupt (TOIE2 = 0).
 */
#define tim2_ovf_disable() TIMSK2 &= ~(1<<TOIE2);

/** @} */

/** @} */

#endif
