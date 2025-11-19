#include "gp2y1010.h"

// ============================================================================
// Initialization
// ============================================================================
void gp2y_init(gp2y_t *dev,
               gp2y_hal_t hal,
               float vcc,
               float adc_max,
               float voc_initial)
{
    dev->hal = hal;
    dev->vcc = vcc;
    dev->adc_max = adc_max;
    dev->voc = voc_initial;    // ~0.6V typical
}

// ============================================================================
// Read voltage output of sensor
// ============================================================================
float gp2y_read_voltage(gp2y_t *dev)
{
    // LED ON
    dev->hal.led_on();
    dev->hal.delay_us(GP2Y1010_SAMPLE_DELAY_US);

    // Read ADC
    uint16_t raw = dev->hal.adc_read();

    // LED OFF
    dev->hal.led_off();

    // Complete 10ms cycle
    uint32_t cycle_used =
        GP2Y1010_SAMPLE_DELAY_US + GP2Y1010_LED_ON_TIME_US;

    if (cycle_used < GP2Y1010_PERIOD_US)
        dev->hal.delay_us(GP2Y1010_PERIOD_US - cycle_used);

    return (raw / dev->adc_max) * dev->vcc;
}

// ============================================================================
// Auto-update Voc baseline
// ============================================================================
void gp2y_update_voc(gp2y_t *dev, float vo)
{
    // If output is lower → new true baseline
    if (vo < dev->voc)
        dev->voc = vo;

    // If output held HIGH long, you may store updated value
    // (optional: averaging filter etc.)
}

// ============================================================================
// Convert reading to dust density (µg/m³)
// ============================================================================
float gp2y_read_dust_ugm3(gp2y_t *dev)
{
    float vo = gp2y_read_voltage(dev);

    // Auto calibration
    gp2y_update_voc(dev, vo);

    float dV = vo - dev->voc;
    if (dV < 0) dV = 0;

    // Conversion from datasheet graph:
    // dust [µg/m³] = 0.17 * ΔV * 1000
    return 0.17f * dV * 1000.0f;
}
