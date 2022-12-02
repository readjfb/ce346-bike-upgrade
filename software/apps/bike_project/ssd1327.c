#include <stdbool.h>
#include <stdint.h>

#include <math.h>
#include "nrf_delay.h"
#include "app_timer.h"
#include "ssd1327.h"

// This person is using an arduino to control the same display
// https://github.com/bitbank2/ssd1327/blob/master/src/ssd1327.cpp#L309

APP_TIMER_DEF(update_timer);

static nrf_twi_mngr_t *i2c_manager = NULL;

static uint8_t ucBackBuffer[8192] = {0};

static void set_row(uint8_t row_num) {
    uint8_t ucCommand[2] = {0x75, 0x00, row_num};
    ssd1327_write_command(ucCommand, 2);
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
    else {
        printf("i2c_write succeeded\n");
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

static void send_data_bytes(uint8_t *data, uint8_t len) {
    uint8_t buf[len+2];

    buf[0] = 0x40;

    int i = 1;
    while (i < len+1) {
        buf[i] = data[i-1];
        i++;
    }

    i2c_write(SSD1327_I2C_ADDR_DATA, buf, len+1);
}

static void send_data_byte(uint8_t data) {
    uint8_t buf[2];

    buf[0] = 0x40;
    buf[1] = data;

    i2c_write(SSD1327_I2C_ADDR_DATA, buf, 2);
}


static void send_1b_cmd(uint8_t cmd)
{
    // Send slave address with SA0=0 for command mode and R/W=0 for write
    // then send the control byte with Co=0 for command mode and D/C#=0 for data
    uint8_t data[2] = {0x00, cmd};
    i2c_write(SSD1327_I2C_ADDR_DATA, data, 2);
}

static void send_2b_cmd(uint8_t cmd0, uint8_t cmd1) {
    // This sends a 2b command
    // First sends the control byte (Co D/C# 00000) with Co=1 for continutation mode and D/C#=0 for command
    // Then the command byte is sent

    // The external i2c address should be sent first, as 011110 SAO R/W with SAO=0 for command mode and R/W=0 for write
    // TODO: does this work?
    uint8_t data[3] = {0x00, cmd0, cmd1};
    i2c_write(SSD1327_I2C_ADDR_DATA, data, 3);
}

static void send_3b_cmd(uint8_t cmd0, uint8_t cmd1, uint8_t cmd2) {
    // This sends a 3b command
    // First sends the control byte (Co D/C# 00000) with Co=1 for continutation mode and D/C#=0 for command
    // Then the command byte is sent

    // The external i2c address should be sent first, as 011110 SAO R/W with SAO=0 for command mode and R/W=0 for write

    uint8_t data[4] = {0x00, cmd0, cmd1, cmd2};
    i2c_write(SSD1327_I2C_ADDR_DATA, data, 4);
}

static void ssd1327set_position(int x, int y, int cx, int cy)
{
    unsigned char buf[8];

    buf[0] = 0x00;
    buf[1] = 0x15;

    buf[2] = x / 2;                         // start address
    buf[3] = (uint8_t)(((x + cx) / 2) - 1); // end address
    buf[4] = 0x75;                          // row start/end
    buf[5] = y;                             // start row
    buf[6] = y + cy - 1;                    // end row

    i2c_write(SSD1327_I2C_ADDR_DATA, buf, 7);

    // set display start line
    send_2b_cmd(0xA1, 0x00);
    send_2b_cmd(0xA2, 0x00);
}

static void test_pixel_write(void)
{
    ssd1327set_position(0, 0, 128, 128);
    // Clear buffer
    for (int i = 0; i < 8192; i++)
    {
        ucBackBuffer[i] = 0x00;
    }

    // set the first pixel to 0
    ucBackBuffer[0] = 0x01;

    // send data in blocks of 32 bytes
    int blocksize = 192;

    for (int i = 0; i < 8192 / blocksize; i++)
    {
        // Send the data
        send_data_bytes(ucBackBuffer + (i*blocksize), blocksize);
    }

    // send_data_bytes(&ucBackBuffer, 4000);
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

    // Remap with A0 and 0b0000 0000
    send_2b_cmd(0xa0, 0x14);

    // set multiplex ratio

    // sleep for 250ms, then turn screen off
    nrf_delay_ms(250);
    send_1b_cmd(0xA6);

    // set to normal
    send_1b_cmd(0xA4);

    // send_2b_cmd(0xFD, 0x12);    // Unlock controller
    // send_1b_cmd(0xA4);          // Set entire display off
    // send_2b_cmd(0xb3, 0x91);    // Set display clock divide ratio/oscillator frequency
    // send_2b_cmd(0xca, 0x3f);    // Set multiplex ratio
    // send_2b_cmd(0xa2, 0x00);    // Set display offset
    // send_2b_cmd(0xa1, 0x00);    // Set display start line
    // send_3b_cmd(0xa0, 0x14, 0x11);// Set remap and data format
    // send_2b_cmd(0xb5, 0x00);    // Set GPIO
    // send_2b_cmd(0xb1, 0xe2);     // Set phase length
    // send_2b_cmd(0xd1, 0x82);    // Set display enhancement A
    // send_1b_cmd(0xa6);           // Set normal display mode
}

static void total_screen_refresh(void)
{
    ssd1327set_position(0, 0, 128, 128);
    // int block_size = 192;
    int block_size = 200;

    int top = (8192.0 / block_size) + .50001;

    for (int i = 0; i < top; i++)
    {
        // if it's the last one, dont overflow
        if (i == top - 1)
        {
            block_size = 8192 - (i * block_size) + 1;
        }

        send_data_bytes(ucBackBuffer + (i * block_size), block_size);
    }
}

void ssd1327_clear(void) {
    // turns off all pixels
    // send_1b_cmd(0xA6);

    // Clear buffer
    for (int i = 0; i < 8192; i++)
    {
        ucBackBuffer[i] = 0x00;
    }

    total_screen_refresh();
}

void ssd1327_solid(void)
{
    // turns on the display
    for (int i = 0; i < 8192; i++)
    {
        ucBackBuffer[i] = 0xFF;
    }

    total_screen_refresh();
}

void ssd1327_normal(void)
{
    // Test the display
    send_1b_cmd(0xA4);
}

void ssd1327_gradient(void)
{
    float frac = 8.0 / 8192.0;
    for (int i = 0; i < 8192; i++)
    {
        uint8_t val = (uint8_t)(frac * i);

        ucBackBuffer[i] = val + (val << 4);

        // ucBackBuffer[i] = 0x0f;
    }

    total_screen_refresh();
}