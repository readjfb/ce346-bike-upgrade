#pragma once

#include "nrf_twi_mngr.h"

// Chip addresses for OLED_Display
// 2b'0111100
static const uint8_t SSD1327_I2C_ADDR_CMD = 0x3d;
// 2b'0111101
static const uint8_t SSD1327_I2C_ADDR_DATA = 0x3c;


// Function prototypes

// Initialize and configure the LSM303AGR accelerometer/magnetometer
//
// i2c - pointer to already initialized and enabled twim instance
void ssd1327_init(const nrf_twi_mngr_t *i2c);

void ssd1327_clear(void);

void ssd1327_solid(void);

void ssd1327_normal(void);

void ssd1327_gradient(void);