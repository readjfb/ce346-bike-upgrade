#pragma once

#include "nrf_twi_mngr.h"

// Chip addresses for OLED_Display
// 2b'0111100
static const uint8_t SSD1327_I2C_ADDR_CMD = 0x3D;
// 2b'0111101
static const uint8_t SSD1327_I2C_ADDR_DATA = 0x3C;


// Function prototypes

// Initialize and configure the LSM303AGR accelerometer/magnetometer
//
// i2c - pointer to already initialized and enabled twim instance
void ssd1327_init(const nrf_twi_mngr_t *i2c);