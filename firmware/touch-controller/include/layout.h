#ifndef LAYOUT_H
#define LAYOUT_H

// ---------------------------------------------------------
//                  View from the front/top
// ---------------------------------------------------------
//                    Touch sensor layout
// ---------------------------------------------------------
// |      0      |      1      |      2      |      3      |
// ---------------------------------------------------------
// |    0x2C     |    0x2B     |    0x2A     |    0x29     |
// ---------------------------------------------------------
// | 01 03 05 07 | 01 03 05 07 | 01 03 05 07 | 01 03 05 07 |
// | 02 04 06 08 | 02 04 06 08 | 02 04 06 08 | 02 04 06 08 |
//----------------------------------------------------------
// | 01 03 05 07 | 09 11 13 15 | 17 19 21 23 | 25 27 29 31 |
// | 02 04 06 08 | 10 12 14 16 | 18 20 22 24 | 26 28 30 32 |
// ---------------------------------------------------------
//                        ToF sensors
// ---------------------------------------------------------
// |      0      |      1      |      2      |      3      |
// ---------------------------------------------------------
// |    0x2D     |    0x2E     |    0x2F     |    0x30     |
// ---------------------------------------------------------

// Direction is relative to the front/top of the board
// Leftmost = Key0, Rightmost = Key15
enum LedDirection { LeftToRight = 0, RightToLeft = 1 };

// 0x2C->82k | 0x2B->100k | 0x2A->120k | 0x29->150k
// These are 7-bit addresses
#define CAP1188_ADDR_0 0x2C
#define CAP1188_ADDR_1 0x2B
#define CAP1188_ADDR_2 0x2A
#define CAP1188_ADDR_3 0x29

// These addresses are assigned dynamically at runtime
// These are 7-bit addresses
#define TOF_ADDR_0 0x2D
#define TOF_ADDR_1 0x2E
#define TOF_ADDR_2 0x2F
#define TOF_ADDR_3 0x30

#define LED_OFFSET 0         // Offset from the first LED
#define LED_SEGMENT_COUNT 47 // 16 keys (x2) and 15 dividers
#define LED_COUNT (LED_OFFSET + LED_SEGMENT_COUNT)
#define LED_DIRECTION RightToLeft // 0 = left->right, 1 = right->left

// --- Pins ---

#define GPIO_TOF_LPN0 4
#define GPIO_TOF_LPN1 5
#define GPIO_TOF_LPN2 6
#define GPIO_TOF_LPN3 7
#define GPIO_TOF_INT 15
#define GPIO_TOF_RESET 16

#define GPIO_I2C_SDA 8
#define GPIO_I2C_SCL 9

#define GPIO_TOUCH_ALERT 17
#define GPIO_TOUCH_RESET 18

#define GPIO_LED_DATA 11

#endif
