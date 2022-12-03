// LED Matrix Driver
// Displays characters on the LED matrix

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"

uint32_t rows[5] = {LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5};
uint32_t cols[5] = {LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5};

bool led_states[5][5] = {false};
int row = 0;

APP_TIMER_DEF(timer1);

void callback(void *context){
  nrf_gpio_pin_clear(rows[row]);
  row = (++row) % 5;
  nrf_gpio_pin_set(rows[row]);
  for(int i = 0; i < 5; i++){
    if(led_states[row][i])
      nrf_gpio_pin_clear(cols[i]);
    else
      nrf_gpio_pin_set(cols[i]);
  }
}

void clear_matrix(){
  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 5; j++){
      led_states[i][j] = false;
    }
  }
}

void set_x(void){
  clear_matrix();
  led_states[0][0] = true;
  led_states[1][1] = true;
  led_states[2][2] = true;
  led_states[3][3] = true;
  led_states[4][4] = true;
  led_states[0][4] = true;
  led_states[1][3] = true;
  led_states[3][1] = true;
  led_states[4][0] = true;
}

void set_char(char c){
  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 5; j++){
      led_states[i][j] = font[c][i] & (1<<j);
    }
  }
}

void led_matrix_init(void) {
  // initialize row pin
  nrf_gpio_pin_dir_set(LED_ROW1,1);
  nrf_gpio_pin_dir_set(LED_ROW2,1);
  nrf_gpio_pin_dir_set(LED_ROW3,1);
  nrf_gpio_pin_dir_set(LED_ROW4,1);
  nrf_gpio_pin_dir_set(LED_ROW5,1);

  // initialize col pins
  nrf_gpio_pin_dir_set(LED_COL1,1);
  nrf_gpio_pin_dir_set(LED_COL2,1);
  nrf_gpio_pin_dir_set(LED_COL3,1);
  nrf_gpio_pin_dir_set(LED_COL4,1);
  nrf_gpio_pin_dir_set(LED_COL5,1);

  // set default values for pins
  for(int i = 0; i < 5; i++){
    nrf_gpio_pin_clear(rows[i]);
    nrf_gpio_pin_clear(cols[i]);
  }

  // nrf_gpio_pin_set(LED_ROW1);
  // nrf_gpio_pin_set(LED_COL1);
  // set_x();
  set_char('S');
  // initialize timer(s) (Part 3 and onwards)
  app_timer_init();
  app_timer_create(&timer1, APP_TIMER_MODE_REPEATED, callback);
  app_timer_start(timer1, 100, NULL);
  // set default state for the LED display (Part 4 and onwards)
}