#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"

#include "neopixel_driver.h"

#include "microbit_v2.h"

// https://github.com/NordicPlayground/nrf52-ble-multi-link-multi-role/tree/master/common/drv_ws2812

static nrfx_pwm_t PWM_INST = NRFX_PWM_INSTANCE(0);

// Holds duty cycle values to trigger PWM toggle
nrf_pwm_values_individual_t pwm_duty_cycle_values[NEOPIXEL_DRIVER_NUM_BITS] = {6 | (1 << 15)};

volatile bool pwm_done = true;

void pwm_handler(nrfx_pwm_evt_type_t event_type)
{
    switch (event_type)
    {
    case NRFX_PWM_EVT_FINISHED:
        pwm_done = true;
        break;
    default:
        break;
    }
}

static nrf_pwm_sequence_t pwm_sequence =
{
    .values.p_individual = pwm_duty_cycle_values,
    .length = (sizeof(pwm_duty_cycle_values) / sizeof(uint16_t)),
    .repeats = 0,
    .end_delay = 0
};

// create three arrays to hold the RGB values for each LED
uint8_t neopixel_driver_red[NEOPIXEL_DRIVER_NUM_LEDS];
uint8_t neopixel_driver_green[NEOPIXEL_DRIVER_NUM_LEDS];
uint8_t neopixel_driver_blue[NEOPIXEL_DRIVER_NUM_LEDS];

static void pwm_init(void)
{
    // https://github.com/NordicPlayground/nrf52-ble-multi-link-multi-role/blob/master/common/drv_ws2812/drv_ws2812.c
    // Initialize the PWM
    // SPEAKER_OUT is the output pin, mark the others as NRFX_PWM_PIN_NOT_USED

    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[1] = NEOPIXEL_DRIVER_PIN;
    // pwm_config.output_pins[1] = NEOPIXEL_DRIVER_PIN;
    pwm_config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    // WS2812 protocol requires a 800 kHz PWM frequency. PWM Top value = 20 and Base Clock = 16 MHz achieves this
    pwm_config.top_value = 20;
    pwm_config.base_clock = NRF_PWM_CLK_16MHz;

    nrfx_pwm_init(&PWM_INST, &pwm_config, pwm_handler);

    // use NRFX_PWM_PIN_INVERTED to invert the output


    // clock is 16 MHz, so 1/16 MHz = 62.5 ns
    // Each click of the counter is 62.5 ns

}

// initialize the rgb arrays to empty
void neopixel_driver_init(void) {
    memset(neopixel_driver_red, 0, sizeof(neopixel_driver_red));
    memset(neopixel_driver_green, 0, sizeof(neopixel_driver_green));
    memset(neopixel_driver_blue, 0, sizeof(neopixel_driver_blue));

    pwm_init();

    neopixel_driver_set_all(0, 0, 0);

    neopixel_driver_send();
}

// create a function to set the RGB values for a single LED
void neopixel_driver_set_led(uint8_t led, uint8_t red, uint8_t green, uint8_t blue) {
    if (led < NEOPIXEL_DRIVER_NUM_LEDS) {
        neopixel_driver_red[led] = red;
        neopixel_driver_green[led] = green;
        neopixel_driver_blue[led] = blue;
    }
}

// create a function to set the RGB values for all LEDs
void neopixel_driver_set_all(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS; i++) {
        neopixel_driver_red[i] = red;
        neopixel_driver_green[i] = green;
        neopixel_driver_blue[i] = blue;
    }
}

static void convert_rgb_to_pwm_sequence(void)
{
    uint8_t buffer[NEOPIXEL_DRIVER_NUM_LEDS * 3];

    // Next, fill the buffer with the data to send
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS; i++)
    {
        buffer[i * 3] = neopixel_driver_green[i];
        buffer[i * 3 + 1] = neopixel_driver_red[i];
        buffer[i * 3 + 2] = neopixel_driver_blue[i];
    }

    // send the data over pwm
    // iterate over the buffer, converting each byte to a sequence of bits sent
    // over pwm.
    // If bit is 1, send a 1 for 900 ns, else send a 1 for 350 ns

    // clock = 16 MHz
    // 1/16 MHz = 62.5 ns
    // each click of the counter is 62.5 ns
    // For a low bit, send a 1 for 350 ns -> 350/62.5 = 5.6 clicks, round up to 6
    // For a high bit, send a 1 for 900 ns -> 900/62.5 = 14.4 clicks, round up to 15

    // iterate over the buffer
    int bit_index = 0;
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_BYTES; i++)
    // for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS * 3; i++)
    {
        // if the byte is in range of the number of LEDs, then send the data
        // otherwise, send LOW for the remaining bytes
        if (i >= NEOPIXEL_DRIVER_NUM_LEDS * 3) {
            continue;
        }

        // iterate over the bits in the byte
        for (int bit = 7; bit >= 0; bit--)
        {
            // if the bit is 1, send a 1 for 900 ns
            uint8_t b = (buffer[i] >> bit) & 0x01;
            uint16_t pwm = 0;

            if (b == 1)
            {
                pwm = PWM_LOGIC_HIGH;
            }
            // else send a 1 for 350 ns
            else
            {
                pwm = PWM_LOGIC_LOW;
            }
            pwm_duty_cycle_values[bit_index++].channel_1 = pwm;
        }
    }
}

// Sends the RGB values to the LEDs
void neopixel_driver_send(void) {
    convert_rgb_to_pwm_sequence();

    if (!pwm_done)
    {
        return;
    }

    pwm_done = false;
    uint32_t err_code = nrfx_pwm_simple_playback(&PWM_INST, &pwm_sequence, 1, NRFX_PWM_FLAG_STOP);

    printf("err_code: %d\n", err_code);

    return;
}

void neopixel_driver_set_range(uint8_t start, uint8_t end, uint8_t red, uint8_t green, uint8_t blue) {
    if (start > end) {
        printf("ERROR IN NEOPIXEL DRIVER SETRANGE: start > end\n");
        return;
    }
    if (start < 0) {
        printf("ERROR IN NEOPIXEL DRIVER SETRANGE: start < 0\n");
    }
    if (end > NEOPIXEL_DRIVER_NUM_LEDS) {
        printf("ERROR IN NEOPIXEL DRIVER SETRANGE: end > NEOPIXEL_DRIVER_NUM_LEDS\n");
    }

    for (int i = start; i <= end; i++) {
        neopixel_driver_set_led(i, red, green, blue);
    }

    for (int i = start; i < end; i++) {
        neopixel_driver_red[i] = red;
        neopixel_driver_green[i] = green;
        neopixel_driver_blue[i] = blue;
    }
}

// create a function to set the RGB values for all LEDs to a single color, the speed of setting increasing based on input speed
// note that speed should be inverted, so a higher speed value will result in a slower change in LEDs with the current implementation
void neopixel_driver_set_all_speed(uint8_t red, uint8_t green, uint8_t blue, uint8_t speed) {
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS; i++) {
        // create a fade effect by turning off the LED speed indices ago
        for (int j = 1; j < speed; j++) {
            if (i-j >= 0) {
                neopixel_driver_red[i - j] = red / j;
                neopixel_driver_green[i - j] = green / j;
                neopixel_driver_blue[i - j] = blue / j;
            }
        }

        neopixel_driver_red[i] = red;
        neopixel_driver_green[i] = green;
        neopixel_driver_blue[i] = blue;
        neopixel_driver_send();
        nrf_delay_ms(speed);
    }
}
