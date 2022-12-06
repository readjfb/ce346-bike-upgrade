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
#include "basic_timing.h"
#include "gpio.h"

static int step = 0;

// static int max_vel = 0;

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
    // Neopixel demo happens once

    // SCREEN INITIALIZATION STARTS HERE
    screen_init();
    clear_screen();

    // Hall init
    timing_init();

    // Button init
    gpio_config(14,0);
    int distance = 0;
    while (1) {
        float vel = get_velocity()*2;
        uint8_t velocity = (uint8_t)vel;
     

        neopixel_driver_set_all(0, 0, 0);
        neopixel_driver_set_color_range((int)(velocity*1.5)%24);
        neopixel_driver_send();

        // if (max_vel < (int)(velocity*1.5)%24) {
        //     max_vel = (int)(velocity*1.5)%24;
        // }
    
        //neopixel_demo();

        // draw the entire screen
        ssd1327_draw_14x10_char(0, 0, velocity, distance);

        // increment the distance
        distance += velocity;
        if (!(gpio_read(14))) 
        { // Button A is pressed -> turn light on
	        distance=0;
            //max_vel=0;
        }	 
        nrf_delay_ms(1000);
    }

    // python -m serial.tools.miniterm /dev/cu.usbmodem0007820214021 38400
}