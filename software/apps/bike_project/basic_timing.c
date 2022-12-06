#include "basic_timing.h"

#include <stdlib.h>

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_timer.h"
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "nrfx_timer.h"
#include "microbit_v2.h"

APP_TIMER_DEF(update_timer);

// Counter to keep track of pulses within the interval
static float revolutions_since_last = 0;

// Buffer to hold recent velocity values that are used to calculate velocity
static float velocity_buffer[VELOCITY_QUEUE_SIZE] = {0};
static int velocity_buffer_index = 0;
static float velocity_buffer_sum = 0;

void hall_sensor_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    printf("Hall sensor triggered!\n");

    revolutions_since_last++;
}

float get_velocity(void) {
    // get the average velocity from the queue
    
    return velocity_buffer_sum / VELOCITY_QUEUE_SIZE;
}

static float get_meters_traveled(void) {
    float meters_traveled = revolutions_since_last * WHEEL_CIRCUMFERENCE;
    revolutions_since_last = 0;
    return meters_traveled;
}

// Gets called by app_timer every ms
static void update_velocity(void) {
    float meters_traveled = get_meters_traveled();
    printf("Meters traveled: %f\n",meters_traveled);
    printf("buffsum %f\n", velocity_buffer_sum);
    float velocity = (meters_traveled*1000) / UPDATE_INTERVAL_MS; // m/s

    // update the velocity buffer
    velocity_buffer_sum -= velocity_buffer[velocity_buffer_index];
    velocity_buffer_sum += velocity;
    velocity_buffer[velocity_buffer_index] = velocity;
    velocity_buffer_index = (velocity_buffer_index + 1) % VELOCITY_QUEUE_SIZE;
}

void timing_init(void) {
    // Initialize the hall sensor pin
    nrf_gpio_cfg_input(HALL_SENSOR_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_pin_set(HALL_SENSOR_PIN);

    // Initialize the gpiote driver
    nrf_drv_gpiote_init();

    // Configure the hall sensor pin to trigger on rising edge
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    config.pull = NRF_GPIO_PIN_NOPULL;
    nrf_drv_gpiote_in_init(HALL_SENSOR_PIN, &config, hall_sensor_handler);

    // Enable the hall sensor pin
    nrf_drv_gpiote_in_event_enable(HALL_SENSOR_PIN, true);

    // Initialize the timer
    app_timer_init();
    app_timer_create(&update_timer, APP_TIMER_MODE_REPEATED, update_velocity);
    app_timer_start(update_timer, APP_TIMER_TICKS(UPDATE_INTERVAL_MS), NULL);
}