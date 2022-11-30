#include <stdbool.h>
#include <stdint.h>

#include <math.h>
#include "nrf_delay.h"
#include "app_timer.h"
#include "ssd1327.h"

// This person is using an arduino to control the same display
// https://github.com/bitbank2/ssd1327/blob/master/src/ssd1327.cpp#L309

APP_TIMER_DEF(update_timer);

static const nrf_twi_mngr_t *i2c_manager = NULL;

// Helper function to perform a 1-byte I2C read of a given register
//
// i2c_addr - address of the device to read from
// reg_addr - address of the register within the device to read
//
// returns 8-bit read value
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr)
{
    uint8_t rx_buf = 0;
    nrf_twi_mngr_transfer_t const read_transfer[] = {
        // TODO: implement me
        NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP),

        NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)};
    nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL);

    return rx_buf;
}

// Helper function to perform a 1-byte I2C write of a given register
//
// i2c_addr - address of the device to write to
// reg_addr - address of the register within the device to write
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data) {
    // TODO: implement me
    // Note: there should only be a single two-byte transfer to be performed

    uint8_t arr[2] = {reg_addr, data};

    nrf_twi_mngr_transfer_t const write_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, &arr, 2, 0)};


    nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);
}

// Helper function to perform a 1-byte I2C write
static void i2c_write(uint8_t i2c_addr, uint8_t *data, uint8_t len)
{
    nrf_twi_mngr_transfer_t const write_transfer[] = {
        NRF_TWI_MNGR_WRITE(i2c_addr, data, len, 0)};
    ret_code_t result = nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL);

    if (result != NRF_SUCCESS) {
        printf("i2c_write failed: %d\n", result);
    }
}

static uint8_t format_first_byte(uint8_t i2c_addr, bool sa0, bool rw)
{
    // Formatted as
    // [i2c_addr 2..8][SA0 1][R/W 0]

    // If write mode is active, set the R/W bit to 0
    // If sa0 is set to 0, command

    uint8_t first_byte = i2c_addr << 1;

    first_byte &= 0xFF << 2;

    first_byte |= (sa0 << 1);

    first_byte |= rw;

    return first_byte;
}

static void send_1b_cmd(uint8_t cmd)
{
    // uint8_t first_byte = format_first_byte(SSD1327_I2C_ADDR, 0, 0);

    // first_byte = SSD1327_I2C_ADDR;

    // printf("I2C ADDR: %x\n", SSD1327_I2C_ADDR);

    // // print first byte
    // printf("first byte: %x\n", first_byte);

    // first byte, command inducer (0x00), command

    uint8_t data[2] = {0x00, cmd};
    i2c_write(SSD1327_I2C_ADDR_CMD, data, 2);
}

static void send_2b_cmd(uint8_t cmd0, uint8_t cmd1) {
    // uint8_t first_byte = format_first_byte(SSD1327_I2C_ADDR, 0, 0);

    uint8_t data[4] = {0x00, cmd0, cmd1};
    i2c_write(SSD1327_I2C_ADDR_CMD, data, 3);

}

// Initialize the SSD1327 display
//
// i2c - pointer to the I2C manager instance
void ssd1327_init(const nrf_twi_mngr_t *i2c)
{
    i2c_manager = i2c;

    // Initialize the display
    // Write the initialization commands to the display
    send_2b_cmd(0xFD, 0x12);
    send_1b_cmd(0xA4);
    send_1b_cmd(0xAF);
    send_1b_cmd(0xA5);


    // Send the command 0xAF to turn on the display
    // send_1b_cmd(0xAF);

    // // Send the command 0xA4 to turn off the entire display
    // send_1b_cmd(0xA4);

    // // turn on the entire display
    // send_1b_cmd(0xA5);


}