#ifndef GP2Y1010_H
#define GP2Y1010_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Constants
// ============================================================================
#define GP2Y1010_LED_ON_TIME_US   320     // LED pulse width
#define GP2Y1010_SAMPLE_DELAY_US  280     // Wait after LED ON before sampling
#define GP2Y1010_PERIOD_US        10000   // 10ms measurement cycle

// ============================================================================
// Hardware Abstraction Layer (must be implemented by user)
// ============================================================================
typedef struct {
    void (*led_on)(void);        // Pull LED pin LOW
    void (*led_off)(void);       // Pull LED pin HIGH
    uint16_t (*adc_read)(void);  // Return 0–ADC_MAX
    void (*delay_us)(uint32_t);  // Microsecond delay
} gp2y_hal_t;

// ============================================================================
// Sensor instance
// ============================================================================
typedef struct {
    gp2y_hal_t hal;

    float vcc;      // ADC reference voltage (usually 5.0V)
    float adc_max;  // Max ADC value (1023 for 10-bit, 4095 for 12-bit)

    float voc;      // Stored no-dust baseline (typical ~0.6V)
} gp2y_t;

// ============================================================================
// Public API
// ============================================================================

// Initialize sensor instance
void gp2y_init(gp2y_t *dev,
               gp2y_hal_t hal,
               float vcc,
               float adc_max,
               float voc_initial);

// Perform one full measurement cycle (LED pulse + ADC)
float gp2y_read_dust_ugm3(gp2y_t *dev);

// Read raw analog voltage
float gp2y_read_voltage(gp2y_t *dev);

// Auto-calibrate Voc baseline (recommended during startup)
void gp2y_update_voc(gp2y_t *dev, float vo);

#ifdef __cplusplus
}
#endif

#endif // GP2Y1010_H
