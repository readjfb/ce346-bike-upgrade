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

#include "screen_driver.h"

NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

static nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;

// initialize the I2C bus
static void initialize_i2c(void) {
    // i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    i2c_config.scl = EDGE_P19;
    i2c_config.sda = EDGE_P20;
    i2c_config.frequency = NRF_TWIM_FREQ_100K;
    i2c_config.interrupt_priority = 0;
    ret_code_t result1 = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

    printf("I2C init result: %d\n", result1);
}

static void initialize_display(const nrf_twi_mngr_t *i2c)
{
    ssd1327_init(i2c);
}

// initialization
void screen_init(void)
{
    printf("Initializing screen...\n");

    // Initialize I2C peripheral and driver
    initialize_i2c();

    printf("I2C initialized!\n");

    // Initialize the display
    initialize_display(&twi_mngr_instance);

    printf("Display initialized!\n");
}