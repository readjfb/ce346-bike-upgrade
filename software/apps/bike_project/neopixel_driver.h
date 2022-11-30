# pragma once

#include "nrf.h"
#include "microbit_v2.h"

# define NEOPIXEL_DRIVER_PIN  EDGE_P13
// # define NEOPIXEL_DRIVER_PIN  13

# define NEOPIXEL_DRIVER_NUM_LEDS  24

# define NEOPIXEL_DRIVER_NUM_BYTES  (NEOPIXEL_DRIVER_NUM_LEDS * 3) + 200
// just pad a lot of bytes to the end of the buffer
# define NEOPIXEL_DRIVER_NUM_BITS  (NEOPIXEL_DRIVER_NUM_BYTES * 8)

// inits are in nano seconds
# define PWM_LOGIC_0_HIGH_NS 350
# define PWM_LOGIC_1_HIGH_NS 900
# define PWM_LOGIC_PERIOD_NS 1250

#define PWM_LOGIC_HIGH 14 | 0x8000
#define PWM_LOGIC_LOW 6 | 0x8000

static void pwm_init(void);

void neopixel_driver_init(void);

// create a function to set the RGB values for a single LED
void neopixel_driver_set_led(uint8_t led, uint8_t red, uint8_t green, uint8_t blue);

// create a function to set the RGB values for all LEDs
void neopixel_driver_set_all(uint8_t red, uint8_t green, uint8_t blue);

// Sends the RGB values to the LEDs
void neopixel_driver_send(void);

// Sets color for a range of consecutive LEDs (inclusive)
void neopixel_driver_set_range(uint8_t start, uint8_t end, uint8_t red, uint8_t green, uint8_t blue);
