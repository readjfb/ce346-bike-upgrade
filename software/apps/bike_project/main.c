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
// #include "screen_driver.h"

static void neopixel_demo(void)
{
    neopixel_driver_set_all(0, 0, 0);
    neopixel_driver_send();
    for (int i = 0; i < NEOPIXEL_DRIVER_NUM_LEDS; i++)
    {
        neopixel_driver_set_range(0, i, 20, 20, 20);
        neopixel_driver_set_led(i, 50, 0, 0);
        neopixel_driver_send();
        nrf_delay_ms(1000 / NEOPIXEL_DRIVER_NUM_LEDS);
    }
}

int main(void)
{
    printf("\n");
    printf("Board started!\n");

    // NEOPIXEL INITIALIZATION STARTS HERE
    neopixel_driver_init();

    printf("Neopixels Done!\n");

    // screen_init();

    while (1) {
        // Do nothing.
        neopixel_demo();


    }

    // python -m serial.tools.miniterm /dev/cu.usbmodem0007820214021 38400
}