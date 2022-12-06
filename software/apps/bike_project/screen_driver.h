#pragma once


#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"

#include "microbit_v2.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "nrf_delay.h"
#include "nrf_twi_mngr.h"

#include "microbit_v2.h"

// Creates a screen driver for the microbit v2 to communicate with the
// Zio Qwiic OLED Display
// Communicates via the I2C bus
// The OLED display is a 128x128 pixel display
// SSD1327

static void initialize_i2c(void);

static void initialize_display(const nrf_twi_mngr_t *i2c);

// initialization
void screen_init(void);

// set entire screen to off
void clear_screen(void);

// set screen to all on
void set_screen_solid(void);

// set screen to a gradient for testing
void set_screen_gradient(void);