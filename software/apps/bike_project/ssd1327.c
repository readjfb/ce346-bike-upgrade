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

//font bitmaps created from DejaVu Mono under open source license
//https://dejavu-fonts.github.io/
//https://dejavu-fonts.github.io/License.html
// anti-aliased 10x14 font
const uint8_t aa_font[][35] = {
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // space
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x55,0x55,0x05,0xaa,0xaa,0xa0,0xa5,0x55,0x55,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // !
{0x40,0x00,0x00,0x0b,0x54,0x00,0x00,0x6a,0x80,0x00,0x00,0xe8,0x00,0x00,0x01,0x40,0x00,0x0d,0x40,0x00,0x00,0xea,0x80,0x00,0x01,0xa8,0x00,0x00,0x01,0x40,0x00,0x00,0x00,0x00,0x00}, // "
{0x52,0x80,0xa0,0x0a,0xe9,0x0a,0x00,0x5e,0xab,0xa4,0x00,0x29,0x7a,0xad,0xd6,0x80,0xa5,0xee,0xa9,0x4a,0x00,0x16,0xaa,0xa5,0x00,0x28,0x5a,0xab,0x02,0x80,0xa1,0x70,0x14,0x0a,0x00}, // #
{0x00,0x00,0x3f,0x40,0x28,0x0e,0xa8,0x0a,0x00,0x91,0xa5,0xa5,0x59,0x5a,0xfa,0xfe,0xbf,0xa0,0xa0,0x24,0x0a,0x0e,0xd6,0x00,0xa0,0x2a,0xb0,0x19,0x00,0x54,0x03,0xc0,0x00,0x00,0x00}, // $
{0x00,0x60,0x6a,0x90,0x02,0x09,0x06,0x00,0x1c,0x80,0x20,0x00,0x8e,0x5b,0x1f,0x4d,0x3a,0xce,0xfb,0x20,0x50,0x90,0x62,0x40,0x09,0x06,0x08,0x00,0xef,0xb0,0x80,0x03,0xac,0x04,0x00}, // %
{0x00,0x02,0xad,0xa0,0x00,0x01,0xa9,0xf0,0x00,0x7a,0xca,0x00,0x1a,0xc9,0xa0,0x0e,0x90,0xaa,0x47,0xa4,0x0a,0xeb,0xa8,0x01,0xa3,0xac,0xb5,0x69,0x00,0x06,0xaa,0xc0,0x00,0x17,0xd0}, // &
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x0b,0x54,0x00,0x00,0x6a,0x80,0x00,0x00,0xe8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // '
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x0e,0xd0,0x00,0x01,0x1a,0xbd,0x5f,0xa0,0x1e,0xaa,0xad,0x00,0x05,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // (
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x00,0x1f,0xaa,0xbd,0x1e,0xaf,0xfe,0xae,0xd4,0x00,0x05,0xd0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // )
{0x10,0x04,0x00,0x03,0xc3,0xc0,0x00,0x08,0x20,0x00,0x00,0x69,0x00,0x00,0xaa,0xaa,0x00,0x05,0x69,0x50,0x00,0x09,0x60,0x00,0x01,0x82,0x40,0x00,0x14,0x0c,0x00,0x00,0x00,0x00,0x00}, // *
{0x00,0x02,0x80,0x00,0x00,0x28,0x00,0x00,0x02,0x80,0x00,0x15,0x69,0x54,0x02,0xaa,0xaa,0x80,0x15,0x69,0x54,0x00,0x02,0x80,0x00,0x00,0x28,0x00,0x00,0x02,0x80,0x00,0x00,0x14,0x00}, // +
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3d,0x00,0x00,0x02,0xa0,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // ,
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // -
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x02,0xa0,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // .
{0xd0,0x00,0x00,0x0a,0xd0,0x00,0x00,0x7a,0xd0,0x00,0x00,0x7a,0xd0,0x00,0x00,0x7a,0xd0,0x00,0x00,0x1a,0x90,0x00,0x00,0x1e,0xb0,0x00,0x00,0x1e,0x00,0x00,0x00,0x10,0x00,0x00,0x00}, // /
{0x05,0xff,0xf5,0x03,0xaa,0xaa,0xa4,0x69,0x40,0x16,0x9a,0x40,0x50,0x1a,0xa0,0x0a,0x00,0xaa,0x00,0xf0,0x0a,0xe9,0x00,0x06,0xb3,0xab,0xfe,0xac,0x07,0xea,0xbd,0x00,0x00,0x00,0x00}, // 0
{0x00,0x00,0x00,0xf0,0x00,0x00,0x0a,0x00,0x00,0x00,0xaa,0xaa,0xaa,0xaa,0xaf,0xff,0xff,0xaa,0x00,0x00,0x0a,0x64,0x00,0x00,0xa2,0x80,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 1
{0x05,0xd0,0x00,0x53,0xaa,0x90,0x0a,0x69,0x7a,0x40,0xaa,0x40,0x69,0x0a,0xa0,0x01,0xa4,0xaa,0x00,0x06,0x8a,0xa0,0x00,0x1b,0xae,0x40,0x00,0x6a,0x3c,0x00,0x01,0xf0,0x00,0x00,0x00}, // 2
{0x07,0x50,0x7f,0x43,0xaa,0x5a,0xa8,0x69,0x6a,0xc3,0xba,0x01,0xa0,0x0a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xa0,0x0f,0x00,0xa6,0x40,0x00,0x0b,0x3c,0x00,0x02,0x80,0x00,0x00,0x14}, // 3
{0x00,0x00,0x28,0x05,0x55,0x56,0x95,0xaa,0xaa,0xaa,0xaa,0xff,0xfe,0xbf,0x79,0x00,0x28,0x00,0x6d,0x02,0x80,0x00,0x6d,0x28,0x00,0x01,0xee,0x80,0x00,0x01,0xa8,0x00,0x00,0x01,0x40}, // 4
{0x00,0x01,0xed,0x0a,0x00,0xea,0xac,0xa0,0x1a,0x46,0x9a,0x02,0x90,0x1a,0xa0,0x28,0x00,0xaa,0x02,0x80,0x0a,0xa5,0x68,0x00,0xaa,0xaa,0x90,0x0a,0x55,0x55,0x02,0x90,0x00,0x00,0x14}, // 5
{0x00,0x07,0xef,0x06,0xc0,0xaa,0xa8,0xa0,0x2b,0x47,0xba,0x02,0x80,0x0a,0xa0,0x28,0x00,0xaa,0x43,0x80,0x0a,0x6b,0x4f,0x03,0xb1,0xaa,0xeb,0xac,0x07,0xea,0xad,0x00,0x00,0x00,0x00}, // 6
{0xb4,0x00,0x00,0x0a,0xb5,0x00,0x00,0xae,0xad,0x00,0x0a,0x16,0xad,0x40,0xa0,0x07,0xab,0x4a,0x00,0x05,0xea,0xa0,0x00,0x01,0xea,0x00,0x00,0x01,0xa0,0x00,0x00,0x05,0x00,0x00,0x00}, // 7
{0x07,0x50,0x7f,0x42,0xaa,0x5a,0xa8,0xed,0x7a,0xc3,0xaa,0x00,0xa0,0x0a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xad,0x7a,0x41,0xa2,0xaa,0x7b,0xe9,0x1f,0xd0,0xea,0x40,0x00,0x00,0x00}, // 8
{0x00,0x55,0x40,0x00,0x7a,0xaa,0xd0,0x2b,0xfb,0xea,0xce,0x40,0x60,0x69,0xa0,0x02,0x81,0xaa,0x00,0x28,0x0a,0xa0,0x02,0x80,0xae,0x95,0xac,0x0a,0x3a,0xaa,0x03,0xc0,0x7f,0x40,0x00}, // 9
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa8,0x02,0xa0,0x0a,0x80,0x2a,0x00,0x54,0x01,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // :
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa8,0x02,0xb0,0x0a,0x80,0x2a,0x00,0x54,0x01,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // ;
{0x03,0xc0,0x0f,0x00,0x38,0x00,0xb0,0x00,0x90,0x18,0x00,0x0a,0x02,0x80,0x00,0x60,0x24,0x00,0x02,0x46,0x00,0x00,0x28,0xa0,0x00,0x00,0x88,0x00,0x00,0x0b,0x80,0x00,0x00,0x64,0x00}, // <
{0x00,0x3c,0x3c,0x00,0x02,0x82,0x80,0x00,0x28,0x28,0x00,0x02,0x82,0x80,0x00,0x28,0x28,0x00,0x02,0x82,0x80,0x00,0x28,0x28,0x00,0x02,0x82,0x80,0x00,0x28,0x28,0x00,0x02,0x82,0x80}, // =
{0x00,0x03,0x00,0x00,0x00,0xa8,0x00,0x00,0x09,0x80,0x00,0x01,0x89,0x00,0x00,0x28,0xa0,0x00,0x02,0x42,0x00,0x00,0xe0,0x2c,0x00,0x0b,0x03,0x80,0x01,0x80,0x09,0x00,0x28,0x00,0xa0}, // >
{0x00,0x00,0x00,0x00,0x74,0x00,0x00,0x2a,0xa0,0x00,0x0a,0xde,0x90,0x00,0xa0,0x1a,0x54,0x5a,0x00,0x6a,0x8a,0xa0,0x01,0x54,0x5e,0x40,0x00,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,0x00}, // ?
{0x07,0xaa,0xaa,0x81,0xb7,0xb5,0xec,0x2c,0x24,0x01,0x82,0x02,0x00,0x08,0x20,0x2d,0x07,0x82,0x01,0xaa,0xa4,0x3c,0x01,0xf4,0x00,0xb0,0x00,0x01,0x06,0xb5,0x55,0xe0,0x0e,0xaa,0xa9}, // @
{0x00,0x00,0x05,0xe0,0x00,0x1f,0xab,0x00,0x7a,0xa9,0x45,0xea,0xb6,0x80,0xab,0x50,0x28,0x0a,0xb4,0x02,0x80,0x7a,0xad,0x68,0x00,0x1e,0xaa,0x90,0x00,0x05,0xea,0xf0,0x00,0x01,0x7a}, // A
{0x00,0x00,0x00,0x00,0x55,0x0e,0xa4,0x3a,0xa7,0xbe,0x9e,0xd6,0xa4,0x1a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // B
{0x00,0x00,0x00,0x06,0xc0,0x00,0x39,0xa0,0x00,0x00,0xaa,0x00,0x00,0x0a,0xa0,0x00,0x00,0xaa,0x00,0x00,0x0a,0xec,0x00,0x03,0xb2,0xb4,0x01,0xe8,0x1e,0xaa,0xab,0x40,0x1f,0xaf,0x40}, // C
{0x00,0x00,0x00,0x00,0x5e,0xab,0x50,0x1a,0xbf,0xea,0x42,0x90,0x00,0x68,0xe4,0x00,0x01,0xba,0x00,0x00,0x0a,0xa0,0x00,0x00,0xaa,0x00,0x00,0x0a,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // D
{0xf0,0x05,0x00,0xfa,0x00,0xa0,0x0a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xa0,0x0a,0x00,0xaa,0xff,0xaf,0xfa,0xaa,0xaa,0xaa,0xa5,0x55,0x55,0x55}, // E
{0x50,0x00,0x00,0x0a,0x00,0xf0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0xaa,0xaa,0xaa,0xff,0xff,0xff,0xf0,0x00,0x00,0x00}, // F
{0x00,0x00,0x00,0x01,0x40,0xaa,0xac,0xe4,0x0a,0xff,0x9a,0x00,0xa0,0x0a,0xa0,0x0f,0x00,0xaa,0x00,0x00,0x0a,0xec,0x00,0x01,0xb2,0xb4,0x01,0xe8,0x1a,0xaa,0xaa,0x40,0x1f,0xaf,0x40}, // G
{0x00,0x00,0x00,0x0a,0xaa,0xaa,0xaa,0xff,0xfa,0xff,0xf0,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // H
{0x00,0x00,0x00,0x05,0x00,0x00,0x05,0xa0,0x00,0x00,0xaa,0x00,0x00,0x0a,0xaf,0xff,0xff,0xaa,0xaa,0xaa,0xaa,0xa5,0x55,0x55,0xaa,0x00,0x00,0x0a,0xa0,0x00,0x00,0xa5,0x00,0x00,0x05}, // I
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xaa,0xaa,0xaf,0x4a,0xaa,0xaa,0xa9,0xa0,0x00,0x01,0xaa,0x00,0x00,0x0a,0xa0,0x00,0x00,0xa5,0x00,0x00,0x0a,0x00,0x00,0x00,0xb0,0x00,0x00,0x28}, // J
{0xc0,0x00,0x00,0x69,0x00,0x00,0x1a,0xec,0x00,0x1e,0x93,0xb0,0x06,0xb4,0x06,0xc7,0xa4,0x00,0x1b,0xad,0x00,0x00,0x69,0x00,0x00,0x01,0xb0,0x00,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // K
{0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xaf,0xff,0xff,0xfa,0xaa,0xaa,0xaa,0xa0,0x00,0x00,0x00}, // L
{0xff,0xff,0xff,0xfa,0xaa,0xaa,0xaa,0xed,0x40,0x00,0x01,0x7b,0x50,0x00,0x00,0x5e,0xc0,0x00,0x05,0xec,0x00,0x17,0xb5,0x00,0x0e,0xd4,0x00,0x00,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // M
{0x00,0x00,0x00,0x0a,0xaa,0xaa,0xaa,0xff,0xff,0xfe,0xa0,0x00,0x01,0xe9,0x00,0x05,0xad,0x00,0x07,0xad,0x00,0x16,0xb4,0x00,0x0e,0xb4,0x00,0x00,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // N
{0x00,0x00,0x00,0x00,0x7e,0xab,0xd0,0x3a,0xbf,0xea,0xce,0x90,0x00,0x6b,0xa0,0x00,0x00,0xaa,0x00,0x00,0x0a,0xa0,0x00,0x00,0xa6,0x94,0x01,0x69,0x3a,0xaa,0xaa,0xc0,0x5f,0xaf,0x50}, // O
{0x0f,0xf0,0x00,0x02,0xaa,0x80,0x00,0xed,0x7b,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0xff,0xaf,0xff,0xaa,0xaa,0xaa,0xa5,0x55,0x55,0x55}, // P
{0x00,0x00,0x00,0x00,0x7e,0xab,0xd0,0x3a,0xbf,0xea,0xce,0x90,0x00,0x6b,0xa0,0x00,0x00,0xaa,0x00,0x00,0x0a,0xa0,0x00,0x00,0xa6,0x94,0x01,0x69,0x3a,0xaa,0xaa,0xc0,0x5f,0xaf,0x50}, // Q
{0x00,0x00,0x01,0xe1,0xeb,0x01,0xab,0x2b,0xec,0xeb,0x4a,0xc1,0xab,0x40,0xa0,0x0a,0x40,0x0a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa}, // R
{0x00,0x00,0x00,0x01,0x40,0x1e,0xb4,0x6c,0x02,0xbe,0x8a,0x00,0xe4,0x3a,0xa0,0x0a,0x00,0xaa,0x00,0xa0,0x0a,0xa0,0x1b,0x00,0xae,0xc2,0x80,0x0a,0x2a,0xac,0x01,0x90,0xff,0x00,0x3c}, // S
{0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xaf,0xff,0xff,0xfa,0xaa,0xaa,0xaa,0xa5,0x55,0x55,0x5a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00}, // T
{0x00,0x00,0x00,0x0a,0xaa,0xaa,0xb4,0xff,0xff,0xfe,0x80,0x00,0x00,0x3b,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x3b,0xaa,0xaa,0xaa,0x8a,0xaa,0xab,0xf0}, // U
{0xad,0x40,0x00,0x0e,0xab,0x50,0x00,0x05,0xea,0xb5,0x00,0x01,0x7a,0xad,0x00,0x00,0x16,0xa0,0x00,0x17,0xaa,0x00,0x7e,0xaf,0x45,0xea,0xb5,0x00,0xaa,0xd0,0x00,0x0d,0x40,0x00,0x00}, // V
{0xaa,0xf5,0x40,0x07,0xea,0xaa,0xfd,0x00,0x55,0xfa,0xa0,0x00,0x5f,0xad,0x00,0xeb,0xd4,0x00,0x0a,0xd4,0x00,0x00,0x7e,0xbd,0x40,0x00,0x07,0xaa,0x15,0x7e,0xaa,0xaa,0xaa,0xbf,0x54}, // W
{0x90,0x00,0x00,0xea,0xc0,0x00,0x6b,0x7a,0x40,0x7a,0x40,0x6b,0x3a,0xd0,0x01,0xea,0x90,0x00,0x1e,0xa9,0x00,0x0e,0xb7,0xad,0x06,0xa4,0x07,0xac,0xac,0x00,0x06,0xa9,0x00,0x00,0x0e}, // X
{0xd0,0x00,0x00,0x0a,0xc0,0x00,0x00,0x6a,0x40,0x00,0x00,0x6b,0x40,0x00,0x01,0xeb,0xff,0xf0,0x02,0xaa,0xaa,0x01,0xa9,0x55,0x51,0xe9,0x00,0x00,0xeb,0x40,0x00,0x0b,0x40,0x00,0x00}, // Y
{0x00,0x00,0x00,0x0f,0x40,0x00,0x0a,0xab,0x00,0x00,0xaa,0xa9,0x00,0x0a,0xa1,0xad,0x00,0xaa,0x07,0xad,0x0a,0xa0,0x07,0xa4,0xaa,0x00,0x06,0xaa,0xa0,0x00,0x1e,0xaf,0x00,0x00,0x1e}, // Z
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x00,0x00,0x0a,0x00,0x00,0x00,0xa5,0x55,0x55,0x5a,0xaa,0xaa,0xaa,0xff,0xff,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // [
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x01,0xeb,0x00,0x01,0xeb,0x40,0x01,0xeb,0x40,0x01,0xeb,0x40,0x01,0xa9,0x40,0x00,0xa9,0x00,0x00,0x0d,0x00,0x00,0x00}, // \'
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x55,0x55,0x55,0xaa,0xaa,0xaa,0xaa,0x55,0x55,0x55,0xa0,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // ]
{0x00,0xc0,0x00,0x00,0x18,0x00,0x00,0x0e,0x40,0x00,0x03,0x90,0x00,0x00,0xa4,0x00,0x00,0x0b,0x00,0x00,0x00,0x6c,0x00,0x00,0x01,0xb0,0x00,0x00,0x06,0xc0,0x00,0x00,0x08,0x00,0x00}, // ^
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // _
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x00,0x06,0xc0,0x00,0x00,0x90,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // `
{0x00,0x01,0x55,0x50,0x03,0xaa,0xaa,0x00,0x6a,0xba,0xf0,0x0a,0x28,0x3c,0x00,0xa2,0x80,0xb0,0x0a,0x28,0x0a,0x00,0xa2,0x80,0xa0,0x0e,0x3b,0x5a,0x00,0x28,0xaa,0x90,0x00,0x07,0xf4}, // a
{0x00,0x00,0x50,0x00,0x01,0xea,0xb4,0x00,0x2a,0xfa,0x80,0x0a,0xc0,0x3a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xe0,0x00,0xb5,0x57,0x95,0x6d,0xaa,0xaa,0xaa,0xaf,0xff,0xff,0xff}, // b
{0x00,0x00,0x00,0x00,0x01,0x40,0x14,0x00,0x64,0x01,0x90,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa4,0x01,0xa0,0x06,0x95,0x69,0x00,0x1a,0xaa,0x40,0x00,0x7f,0xd0}, // c
{0x55,0x55,0x55,0x5a,0xaa,0xaa,0xaa,0xff,0xfa,0xfa,0xf0,0x02,0xc0,0x38,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0e,0x95,0x6b,0x00,0x3a,0xaa,0xc0,0x00,0x7f,0xd0}, // d
{0x00,0x00,0x50,0x00,0x01,0xea,0x3c,0x00,0x2b,0xa1,0xb0,0x0a,0x4a,0x0a,0x00,0xa0,0xa0,0xa0,0x0a,0x0a,0x0a,0x00,0xa0,0xa0,0xa0,0x06,0x9a,0x69,0x00,0x3a,0xaa,0xc0,0x00,0x7f,0xd0}, // e
{0x00,0x00,0x00,0x0a,0x0a,0x00,0x00,0xa0,0xa0,0x00,0x0a,0x0a,0x00,0x00,0xad,0xa5,0x55,0x56,0xaa,0xaa,0xaa,0x05,0xa5,0x55,0x50,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x05,0x00,0x00}, // f
{0x00,0x55,0x55,0x50,0x0a,0xaa,0xaa,0x00,0xfa,0xfa,0xf0,0x02,0xc0,0x3c,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x01,0xa0,0x0e,0x95,0x6b,0x00,0x3a,0xaa,0xc0,0x00,0x7f,0xd0}, // g
{0x00,0x00,0x00,0x00,0x00,0x15,0x55,0x00,0x3a,0xaa,0xa0,0x0e,0xbf,0xff,0x00,0xa4,0x00,0x00,0x0a,0x00,0x00,0x00,0x60,0x00,0x05,0x57,0x95,0x55,0xaa,0xaa,0xaa,0xaf,0xff,0xff,0xff}, // h
{0x00,0x00,0x00,0x50,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0xa0,0xaa,0xaa,0xaa,0x0a,0xaa,0xaa,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xf0,0x00,0xa0,0x00,0x00,0x05}, // i
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x55,0x55,0xa0,0xaa,0xaa,0xa5,0x0a,0x55,0x55,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x00,0x00}, // j
{0x00,0x80,0x00,0x30,0x09,0x00,0x1a,0x00,0xa4,0x06,0xb0,0x06,0x93,0xa4,0x00,0x1a,0xad,0x00,0x00,0x6b,0x00,0x00,0x01,0xa4,0x0a,0xaa,0xaa,0xaa,0xff,0xff,0xff,0xf0,0x00,0x00,0x00}, // k
{0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x55,0x55,0x55,0xaa,0xaa,0xaa,0xa9,0xaf,0xff,0xff,0x4a,0x00,0x00,0x00,0xa0,0x00,0x00,0x05,0x00,0x00,0x00}, // l
{0x00,0x7a,0xaa,0xa0,0x0a,0xbf,0xff,0x00,0xa0,0x00,0x00,0x06,0x55,0x55,0x00,0x7a,0xaa,0xa0,0x0a,0xbf,0xff,0x00,0xa0,0x00,0x00,0x06,0x55,0x55,0x00,0xaa,0xaa,0xa0,0x0f,0xff,0xff}, // m
{0x00,0x00,0x00,0x00,0x00,0x15,0x55,0x00,0x3a,0xaa,0xa0,0x0e,0xbf,0xff,0x00,0xa4,0x00,0x00,0x0a,0x00,0x00,0x00,0x60,0x00,0x00,0x07,0x95,0x55,0x00,0xaa,0xaa,0xa0,0x0f,0xff,0xff}, // n
{0x00,0x00,0x50,0x00,0x01,0xea,0xb4,0x00,0x2a,0xfa,0x80,0x0a,0xc0,0x3a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0e,0x95,0x6b,0x00,0x3a,0xaa,0xc0,0x00,0x7f,0xd0}, // o
{0x00,0x00,0x50,0x00,0x01,0xea,0xb4,0x00,0x2a,0xfa,0x80,0x0a,0xc0,0x3a,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xe0,0x00,0xb0,0x07,0x95,0x6d,0x00,0xaa,0xaa,0xa0,0x0f,0xff,0xff}, // p
{0x00,0x55,0x55,0x50,0x0a,0xaa,0xaa,0x00,0xfa,0xfa,0xf0,0x02,0xc0,0x38,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x00,0xa0,0x0e,0x95,0x6b,0x00,0x3a,0xaa,0xc0,0x00,0x7f,0xd0}, // q
{0x00,0x7c,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0x64,0x00,0x00,0x05,0xb5,0x55,0x00,0xaa,0xaa,0xa0,0x0f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // r
{0x00,0x00,0x00,0x00,0x01,0x41,0x54,0x00,0xe4,0xea,0x80,0x0a,0x0a,0x7a,0x00,0xa0,0xa0,0xa0,0x0a,0x19,0x0a,0x00,0xa6,0x80,0xa0,0x06,0xa8,0x0a,0x00,0x3b,0x42,0x80,0x00,0x00,0x00}, // s
{0x00,0x00,0x00,0x00,0x0f,0x00,0x0f,0x00,0xa0,0x00,0xa0,0x0a,0x00,0x0a,0x00,0xa0,0x01,0xa3,0xfa,0xff,0xeb,0x2a,0xaa,0xaa,0xc0,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0f,0x00,0x00}, // t
{0x00,0x00,0x00,0x00,0x05,0x55,0x55,0x00,0xaa,0xaa,0xa0,0x0f,0xff,0xef,0x00,0x00,0x01,0x80,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x05,0x55,0x7a,0x00,0xaa,0xaa,0x90,0x0f,0xff,0xd4}, // u
{0x00,0xd0,0x00,0x00,0x0a,0x94,0x00,0x00,0x7a,0xb5,0x00,0x00,0x1e,0xad,0x00,0x00,0x16,0xa0,0x00,0x01,0xea,0x00,0x05,0xea,0xd0,0x07,0xab,0x40,0x00,0xad,0x40,0x00,0x0d,0x00,0x00}, // v
{0x00,0xaf,0x50,0x00,0x0f,0xaa,0xf5,0x00,0x05,0x7a,0xa0,0x00,0x01,0x6b,0x00,0x03,0xed,0x40,0x00,0x29,0x40,0x00,0x01,0x7b,0x50,0x00,0x01,0x6a,0x00,0x17,0xaa,0xb0,0x0a,0xaf,0x50}, // w
{0x00,0xc0,0x00,0x60,0x0b,0x00,0x1a,0x00,0xe9,0x06,0x90,0x03,0xa7,0xa4,0x00,0x06,0xa8,0x00,0x01,0xae,0xb0,0x00,0x6b,0x1a,0xc0,0x0a,0x40,0x3a,0x00,0x90,0x00,0x60,0x04,0x00,0x03}, // x
{0x00,0xd0,0x00,0x00,0x0a,0x94,0x00,0x00,0x7a,0xb4,0x00,0x00,0x1e,0xb5,0x00,0x00,0x1e,0xa0,0x00,0x07,0xab,0x00,0x07,0xa9,0x40,0x07,0xad,0x00,0x00,0xad,0x00,0x00,0x0d,0x00,0x00}, // y
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xb4,0x00,0xa0,0x0a,0x90,0x0a,0x00,0xaa,0xc0,0xa0,0x0a,0x6b,0x4a,0x00,0xa1,0xe9,0xa0,0x0a,0x01,0xaa,0x00,0xa0,0x06,0xa0,0x05,0x00,0x0f}, // z
{0x00,0x00,0x00,0x05,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0xab,0xfc,0x1f,0xf3,0xaa,0xae,0xaa,0x00,0x06,0xb4,0x00,0x00,0x28,0x00,0x00,0x02,0x80,0x00,0x00,0x00,0x00}, // {
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x5a,0xaa,0xaa,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // |
{0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x02,0x80,0x00,0x00,0x29,0x00,0x1f,0xfa,0xaf,0xf6,0xaa,0x96,0xaa,0xa5,0x50,0x05,0x5a,0x00,0x00,0x00,0xa0,0x00,0x00,0x00,0x00,0x00,0x00}, // }
{0x00,0x0f,0x00,0x00,0x00,0x6c,0x00,0x00,0x02,0x80,0x00,0x00,0x28,0x00,0x00,0x02,0x80,0x00,0x00,0xa4,0x00,0x00,0x0a,0x00,0x00,0x00,0xa0,0x00,0x00,0x0a,0x00,0x00,0x00,0x64,0x00}, // ~
{0x00,0x08,0x40,0x00,0x00,0x88,0x00,0x00,0x08,0x88,0x00,0x00,0x88,0x8c,0xff,0xf8,0x88,0x80,0x00,0x88,0x88,0x00,0x08,0x88,0x00,0x00,0x88,0x40,0x00,0x08,0xc0,0x00,0x00,0x80,0x00},// ground
{0x00,0x08,0x80,0x00,0x00,0x88,0x80,0x00,0x08,0x88,0x00,0x00,0x88,0x88,0xaa,0xa8,0x88,0x80,0x00,0x88,0x88,0x00,0x08,0x88,0x00,0x00,0x88,0x00,0x00,0x08,0x80,0x00,0x00,0x80,0x00}
};

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
        //printf("i2c_write succeeded\n");
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

void ssd1327_draw_14x10_char(uint8_t x, uint8_t y, char c)
{
    ssd1327set_position(0, 0, 128, 128);
    uint8_t char_j[35] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; //E

    //     // make a 10x14 block of pixels
    uint8_t pixels[10][14] = {{0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF},{0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF},{0xF,0xF,0,0,0xF,0xF,0,0,0xF,0xF}};

    // int i = 0;

    // while (i < 140) {
    //     // get the refrenced_byte
    //     uint8_t refrenced_byte = char_j[i/4];

    //     // get the bit index
    //     uint8_t bit_index = i % 4;

    //     // get the two bits

    //     uint8_t bits = (refrenced_byte >> (bit_index * 2)) & 0x03;

    //     // set the pixel
    //     switch (bits) {
    //         case 0:
    //             printf("0");
    //             pixels[i % 10][i / 10] = 0x00;
    //             break;
    //         case 1:
    //             printf("1");
    //             pixels[i % 10][i / 10] = 0x05;
    //             break;
    //         case 2:
    //             printf("2");
    //             pixels[i % 10][i / 10] = 0x0A;
    //             break;
    //         case 3:
    //             printf("2");
    //             pixels[i % 10][i / 10] = 0x0F;
    //             break;
    //     }
    //     i=i+1;
    // }

    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 5; j++) {
            // set back buffer to pixels[i][j] << 4 + pixels[i][j+1]
                // ucBackBuffer[i * 64 + j] = (pixels[j*2][i] << 4) + pixels[j*2+1][i];
                //printf("Second pixel: %x\n",pixels[j*2+1][i]);
                // ucBackBuffer[i * 64 + j] = pixels[j*2][i] << 4| pixels[j*2+1][i];
                
                ucBackBuffer[i * 64 + j] = pixels[j*2][i] << 4| pixels[j*2+1][i];
                // ucBackBuffer[i * 64 + j] = 0xFF;


        }
    }

    // send data in blocks of 32 bytes
    total_screen_refresh();
}

void ssd1327_draw_14x10_string(uint8_t x, uint8_t y, char *string)
{
    ssd1327set_position(0, 0, 128, 128);
    // set 10x14 pixels to intended characters
    unsigned char c, *s;
    uint8_t i, ty, tx;
    i = 0;
    while (string[i] != 0) {
        c = string[i]-32;
        s = (unsigned char *)&aa_font[(uint8_t)c*35]; // dont know why this is needed
        for (ty=0; ty<14; ty++)
            {
                for (tx=0; tx<5; tx++) // 5 output bytes per line
                {
                    ucBackBuffer[tx+(128*ty)] = s[tx+(ty*10)];
                }
            }
        i++;
    }

    // send data in blocks of 32 bytes
    int blocksize = 192;

    for (int i = 0; i < 8192 / blocksize; i++)
    {
        // Send the data
        send_data_bytes(ucBackBuffer + (i*blocksize), blocksize);
    }

}