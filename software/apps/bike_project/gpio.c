#include "gpio.h"
#include <stdio.h>

// FOR BUTTON A TO RESET distance



typedef struct{
  // Step 3:
  uint32_t blank[321];
  uint32_t OUT;
  uint32_t OUTSET;
  uint32_t OUTCLR;
  uint32_t IN;
  uint32_t DIR;
  uint32_t DIRSET;
  uint32_t DIRCLR;
  uint32_t LATCH;
  uint32_t DETECTMODE;
  uint32_t BLANK[118];
  uint32_t PIN_CNF[32];
} gpio_reg_t;


volatile gpio_reg_t* port0 = (gpio_reg_t*)0x50000000;
volatile gpio_reg_t* port1 = (gpio_reg_t*)0x50000300;

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
//  dir - gpio direction (INPUT, OUTPUT)
void gpio_config(uint8_t gpio_num, gpio_direction_t dir) {
  // use bitmask to not change other bits 
  if (gpio_num >= 32) {
	// edit port1 if gpio_num > 32
  	port1->PIN_CNF[gpio_num - 32]  = (port1->PIN_CNF[gpio_num - 32] & ~3) | ((dir + dir << 1) & 3);
  }
  else {
  	port0->PIN_CNF[gpio_num]  = (port1->PIN_CNF[gpio_num] & ~3) | (dir + dir << 1 & 3);
  }
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
void gpio_set(uint8_t gpio_num) {
  if (gpio_num >= 32) {
    port1->OUTSET = (port1->OUTSET & ~(1 << (gpio_num - 32)) | (1 << (gpio_num - 32)));
  }
  else {
    port0->OUTSET = (port1->OUTSET & ~(1 << (gpio_num)) | (1 << (gpio_num)));
  }
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
void gpio_clear(uint8_t gpio_num) {
  if (gpio_num >= 32) {
    port1->OUTCLR = 1 << (gpio_num - 32);
  }
  else {
    port0->OUTCLR = 1 << gpio_num;
  }
}

// Inputs: 
//  gpio_num - gpio number 0-31 OR (32 + gpio number)
// Output:
//  bool - pin state (true == high)
bool gpio_read(uint8_t gpio_num) {
  if (gpio_num >= 32) {
    return !!((port1->IN) & (1 << gpio_num));
  }
  else {
    return !!((port0->IN) & (1 << gpio_num));
  }
}

// prints out some information about the GPIO driver. Can be called from main()
void gpio_print(void) {
}
