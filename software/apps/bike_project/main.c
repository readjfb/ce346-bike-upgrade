// PWM Square wave tone app
//
// Use PWM to play a tone over the speaker using a square wave

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "nrf_delay.h"
#include "nrf_twi_mngr.h"
#include "microbit_v2.h"
#include "neopixel_driver.h"
#include "screen_driver.h"

static int step = 0;

static void neopixel_demo(void)
{

    if (step == 0) {
        neopixel_driver_set_all(0, 0, 0);
        neopixel_driver_send();
        step++;
        return;
    }

    neopixel_driver_set_range(0, step, 20, 20, 20);
    neopixel_driver_set_led(step, 50, 0, 0);
    neopixel_driver_send();
    step++;
    // nrf_delay_ms(1000 / NEOPIXEL_DRIVER_NUM_LEDS);

    step = step % NEOPIXEL_DRIVER_NUM_LEDS;

}

int main(void)
{
    printf("\n");
    printf("Board started!\n");

    // NEOPIXEL INITIALIZATION STARTS HERE
    neopixel_driver_init();

    printf("Neopixels Done!\n");

    // SCREEN INITIALIZATION STARTS HERE
    screen_init();
    clear_screen();
    while (1) {
        neopixel_demo();

        // set_screen_solid();

        // nrf_delay_ms(1000);

        //clear_screen();

        // nrf_delay_ms(1000);

        //set_screen_gradient();

        ssd1327_draw_14x10_char(0, 0, 'j');
        // ssd1327_draw_14x10_char(25, 25, '13');

        nrf_delay_ms(1000);
    }

    // python -m serial.tools.miniterm /dev/cu.usbmodem0007820214021 38400
}