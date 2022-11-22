#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"

#include "neopixel_driver.h"

#include "microbit_v2.h"

static const nrfx_pwm_t PWM_INST = NRFX_PWM_INSTANCE(0);

// Holds duty cycle values to trigger PWM toggle
nrf_pwm_values_common_t pwm_bits[1] = {6};

// create three arrays to hold the RGB values for each LED
uint8_t neopixel_driver_red[NEOPIXEL_DRIVER_NUM_LEDS];
uint8_t neopixel_driver_green[NEOPIXEL_DRIVER_NUM_LEDS];
uint8_t neopixel_driver_blue[NEOPIXEL_DRIVER_NUM_LEDS];

static void pwm_init(void)
{
    // Initialize the PWM
    // SPEAKER_OUT is the output pin, mark the others as NRFX_PWM_PIN_NOT_USED
    // Set the clock to 500 kHz, count mode to Up, and load mode to Common
    // The Countertop value doesn't matter for now. We'll set it in play_tone()
    // TODO
    nrfx_pwm_config_t config;
    config.output_pins[0] = NEOPIXEL_DRIVER_PIN;
    config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
    config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;

    // set clock to be as fast as possible
    config.base_clock = NRF_PWM_CLK_16MHz;
    config.count_mode = NRF_PWM_MODE_UP;
    config.top_value = 20;
    config.load_mode = NRF_PWM_LOAD_COMMON;
    config.step_mode = NRF_PWM_STEP_AUTO;
    nrfx_pwm_init(&PWM_INST, &config, NULL);

    // use NRFX_PWM_PIN_INVERTED to invert the output


    // clock is 16 MHz, so 1/16 MHz = 62.5 ns
    // Each click of the counter is 62.5 ns

}

// initialize the rgb arrays to empty
void neopixel_driver_init(void) {
    pwm_init();

    neopixel_driver_set_all(0, 0, 0);
}

// create a function to set the RGB values for a single LED
void neopixel_driver_set_led(uint8_t led, uint8_t red, uint8_t green, uint8_t blue) {
    if (led < NEOPIXEL_DRIVER_NUM_LEDS) {
        neopixel_driver_red[led] = red;
        neopixel_driver_green[led] = green;
        neopixel_driver_blue[led] = blue;
    }
}

// create a function to set the RGB values for all LEDs
void neopixel_driver_set_all(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS; i++) {
        neopixel_driver_red[i] = red;
        neopixel_driver_green[i] = green;
        neopixel_driver_blue[i] = blue;
    }
}

// Sends the RGB values to the LEDs
void neopixel_driver_send(void) {
    // First, create a buffer to hold the data to send
    uint8_t buffer[NEOPIXEL_DRIVER_NUM_BYTES];

    // Next, fill the buffer with the data to send
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS; i++) {
        buffer[i * 3] = neopixel_driver_green[i];
        buffer[i * 3 + 1] = neopixel_driver_red[i];
        buffer[i * 3 + 2] = neopixel_driver_blue[i];
    }

    // send the data over pwm
    // iterate over the buffer, converting each byte to a sequence of bits sent
    // over pwm.
    // If bit is 1, send a 1 for 900 ns, else send a 1 for 350 ns

    // clock = 16 MHz
    // 1/16 MHz = 62.5 ns
    // each click of the counter is 62.5 ns
    // For a low bit, send a 1 for 350 ns -> 350/62.5 = 5.6 clicks, round up to 6
    // For a high bit, send a 1 for 900 ns -> 900/62.5 = 14.4 clicks, round up to 15

    // iterate over the buffer
    int bit_index = 0;
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS * 3; i++) {
        // iterate over the bits in the byte
        for (int j = 7; j >= 0; j--) {
            // if the bit is 1, send a 1 for 900 ns
            if (buffer[i] & (1 << j)) {
                pwm_bits[bit_index] = 15;
            }
            // else send a 1 for 350 ns
            else {
                pwm_bits[bit_index] = 6;
            }
            bit_index++;
        }
    }

    nrf_pwm_sequence_t pwm_sequence = {
        .values.p_common = pwm_bits,
        .length = NEOPIXEL_DRIVER_NUM_BITS,
        .repeats = 0,
        .end_delay = 0,
    };

    nrfx_pwm_simple_playback(&PWM_INST, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
}