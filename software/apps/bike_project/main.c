// PWM Square wave tone app
//
// Use PWM to play a tone over the speaker using a square wave

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "microbit_v2.h"
#include "neopixel_driver.h"

int main(void)
{
    printf("Board started!\n");

    neopixel_driver_init();

    neopixel_driver_set_all(100, 0, 0);

    neopixel_driver_send();
}