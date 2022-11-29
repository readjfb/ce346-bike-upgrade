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

int main(void)
{
    printf("\n");
    printf("Board started!\n");

    neopixel_driver_init();

    neopixel_driver_set_all(100, 1, 1);

    neopixel_driver_send();

    printf("Done!\n");

    // screen_init();

    while (1) {
        // Do nothing.
        neopixel_driver_send();
    }

    // python -m serial.tools.miniterm /dev/cu.usbmodem0007820214021 38400
}