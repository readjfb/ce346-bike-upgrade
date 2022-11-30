#pragma once

#include "nrf.h"
#include "stdbool.h"
#include "microbit_v2.h"

#define HALL_SENSOR_PIN EDGE_P8
#define WHEEL_RADIUS 0.3 // meters
#define WHEEL_CIRCUMFERENCE 2 * 3.1415 * WHEEL_RADIUS // meters
#define UPDATE_INTERVAL_MS 250 // ms

#define VELOCITY_QUEUE_SIZE 10

float get_velocity(void);

void timing_init(void);