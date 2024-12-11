#ifndef LAYOUT_H
#define LAYOUT_H

enum LedDirection { LeftToRight = 0, RightToLeft = 1 };

//        Touch sensor layout (view from the front)
// ---------------------------------------------------------
// |    0x29     |    0x2A     |    0x2B     |    0x2C     |
// ---------------------------------------------------------
// | 01 03 05 07 | 01 03 05 07 | 01 03 05 07 | 01 03 05 07 |
// | 02 04 06 08 | 02 04 06 08 | 02 04 06 08 | 02 04 06 08 |
//----------------------------------------------------------
// | 01 03 05 07 | 09 11 13 15 | 17 19 21 23 | 25 27 29 31 |
// | 02 04 06 08 | 10 12 14 16 | 18 20 22 24 | 26 28 30 32 |
// ---------------------------------------------------------
#define CAP1188_ADDR_0 0x29
#define CAP1188_ADDR_1 0x2A
#define CAP1188_ADDR_2 0x2B
#define CAP1188_ADDR_3 0x2C

// TODO: Support ALL LEDs
#define LED_COUNT 18
#define LED_KEY_COUNT 16
#define LED_OFFSET 2              // Offset from the first LED
#define LED_DIRECTION RightToLeft // 0 = left->right, 1 = right->left

// --- Pins ---

#define GPIO_I2C_SDA 8
#define GPIO_I2C_SCL 9

#define GPIO_LED_DATA 35

#endif
