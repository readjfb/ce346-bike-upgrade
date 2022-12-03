// OLED Display driver for Microbit_v2
// Based on LSM303AGR driver for Microbit_v2
//
// Initializes sensor and communicates over I2C
// Capable of reading temperature, acceleration, and magnetic field strength
// EDGE_P19 (SCL) and EDGE_P20 (SDA)
// I2C address: 0x78, 0x7A (Default: 0x78)

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "oled_display_driver.h"
#include "nrf_delay.h"
#include "app_timer.h"

// Pointer to an initialized I2C instance to use for transactions
static const nrf_twi_mngr_t* i2c_manager = NULL;
APP_TIMER_DEF(timer1);



// Helper function to perform a 1-byte I2C read of a given register
//
// i2c_addr - address of the device to read from
// reg_addr - address of the register within the device to read
//
// returns 8-bit read value
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr) {
  uint8_t rx_buf = 0;
  nrf_twi_mngr_transfer_t const read_transfer[] = {
		       NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),
                       NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);

  return rx_buf;
}

// Helper function to perform a 1-byte I2C write of a given register
//
// i2c_addr - address of the device to write to
// reg_addr - address of the register within the device to write
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
  //TODO: implement me
  //Note: there should only be a single two-byte transfer to be performed
  uint8_t arr[] = {reg_addr, data}; 
  nrf_twi_mngr_transfer_t const write_transfer[] = {
			    NRF_TWI_MNGR_WRITE(i2c_addr, arr, 2, 0),
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
}

float calculate_tilt(lsm303agr_measurement_t m) {
  float phi_rad = (float)atan((double)(sqrt((double)((m.x_axis*m.x_axis) + (m.y_axis*m.y_axis))) / m.z_axis));
  float phi = (int32_t)(phi_rad * 180.0/3.14) ;//% 360;
  printf("TILT: %f\n", phi);
  return phi;
}

void read_sensors() {
  lsm303agr_read_temperature();
  lsm303agr_read_magnetometer();
  lsm303agr_measurement_t m = lsm303agr_read_accelerometer();
  calculate_tilt(m);
  printf("================================\n");
  
}

// Initialize and configure the LSM303AGR accelerometer/magnetometer
//
// i2c - pointer to already initialized and enabled twim instance
void lsm303agr_init(const nrf_twi_mngr_t* i2c) {
  i2c_manager = i2c;

 // ---Initialize Accelerometer---

  // Reboot acclerometer
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG5, 0x80);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG4, 0x80);

  // Configure accelerometer at 100Hz, normal mode (10-bit)
  // Enable x, y and z axes
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG1, 0x57);

  // Read WHO AM I register
  // Always returns the same value if working
  uint8_t result = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_WHO_AM_I_REG);
  //TODO: check the result of the Accelerometer WHO AM I register
  //printf("Accelerometer: %p\n", result);
  // ---Initialize Magnetometer---

  // Reboot magnetometer
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_A, 0x40);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_C, 0x10);

  // Configure magnetometer at 100Hz, continuous mode
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_A, 0x0C);

  // Read WHO AM I register
  result = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_WHO_AM_I_REG);
  //TODO: check the result of the Magnetometer WHO AM I register
  //printf("Magnetomer: %p\n", result);

  // ---Initialize Temperature---


  // Enable temperature sensor
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_CFG_REG, 0xC0);
  app_timer_init();
  app_timer_create(&timer1, APP_TIMER_MODE_REPEATED, read_sensors);
  app_timer_start(timer1, 32700, NULL);
}



int16_t combine_msb_lsb(uint8_t device, uint8_t msb_reg, uint8_t lsb_reg) {
  uint8_t lsb_result  = i2c_reg_read(device, lsb_reg);
  uint8_t msb_result  = i2c_reg_read(device, msb_reg);
  uint16_t result = ((uint16_t)msb_result << 8) | (lsb_result);
  int16_t signed_result = (int16_t) result;
  return signed_result;
}

// Read the internal temperature sensor
//
// Return measurement as floating point value in degrees C
float lsm303agr_read_temperature(void) {
  int16_t signed_result = combine_msb_lsb(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_H, LSM303AGR_ACC_TEMP_L);
  float output = (float)signed_result*(1.0/256.0)+25.0;
  printf("TEMP: %f\n\n", output);
  return output;
}


lsm303agr_measurement_t lsm303agr_read_accelerometer(void) {
  int16_t x_value = combine_msb_lsb(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_X_H, LSM303AGR_ACC_OUT_X_L);
  int16_t y_value = combine_msb_lsb(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Y_H, LSM303AGR_ACC_OUT_Y_L);
  int16_t z_value = combine_msb_lsb(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Z_H, LSM303AGR_ACC_OUT_Z_L);

  lsm303agr_measurement_t measurement = {
					 (float)(x_value>>6) * 3.9 / 1000.0,
					 (float)(y_value>>6) * 3.9 / 1000.0,
					 (float)(z_value>>6) * 3.9 / 1000.0
  };
  printf("ACCEL X: %f\n", measurement.x_axis);
  printf("ACCEL Y: %f\n", measurement.y_axis);
  printf("ACCEL Z: %f\n\n", measurement.z_axis);
  return measurement;
}

lsm303agr_measurement_t lsm303agr_read_magnetometer(void) {
  int16_t x_value = combine_msb_lsb(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_X_H_REG, LSM303AGR_MAG_OUT_X_L_REG);
  int16_t y_value = combine_msb_lsb(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_Y_H_REG, LSM303AGR_MAG_OUT_Y_L_REG);
  int16_t z_value = combine_msb_lsb(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_Z_H_REG, LSM303AGR_MAG_OUT_Z_L_REG);

  lsm303agr_measurement_t measurement = {
					 (float)x_value * 1.5 / 10.0,
					 (float)y_value * 1.5 / 10.0,
					 (float)z_value * 1.5 / 10.0
  };
  printf("MAGNET X: %f\n", measurement.x_axis);
  printf("MAGNET Y: %f\n", measurement.y_axis);
  printf("MAGNET Z: %f\n\n", measurement.z_axis);
  return measurement;
}
