/**
 ********************************************************************
 * @file    lgpio_stub.c
 * @brief   Stub implementation of lgpio library for development
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This file provides stub implementations of lgpio library
 *          functions for development and testing on macOS. The actual
 *          lgpio library is only available on Raspberry Pi systems.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 *********************************************************************
 */

#include "lgpio_stub.h"
#include <stdio.h>
#include <stdlib.h>

/* GPIO chip operations */
int lgGpiochipOpen(int chip)
{
    printf("[LGPIO_STUB] Opening GPIO chip %d\n", chip);
    return 0; // Return valid handle
}

int lgGpiochipClose(int chip)
{
    printf("[LGPIO_STUB] Closing GPIO chip %d\n", chip);
    return 0;
}

/* GPIO pin operations */
int lgGpioClaimInput(int handle, int lFlags, int gpio)
{
    printf("[LGPIO_STUB] Claiming GPIO %d as input (flags: %d)\n", gpio, lFlags);
    return 0; // Success
}

int lgGpioFree(int handle, int gpio)
{
    printf("[LGPIO_STUB] Freeing GPIO %d\n", gpio);
    return 0; // Success
}

int lgGpioRead(int handle, int gpio)
{
    // Return random values for testing
    static int counter = 0;
    counter++;
    
    // Simulate different pin behaviors
    if (gpio >= 17 && gpio <= 22) {
        // Bearing pins - simulate detection
        return (counter % 20 == 0) ? 1 : 0;
    } else if (gpio >= 13 && gpio <= 25) {
        // Segment pins - simulate 7-segment display
        return (counter % 10 == 0) ? 1 : 0;
    } else if (gpio == 18) {
        // RSSI pin - simulate signal strength
        return (counter % 15 == 0) ? 1 : 0;
    }
    
    return 0; // Default to low
}

int lgGpioWrite(int handle, int gpio, int level)
{
    printf("[LGPIO_STUB] Writing GPIO %d = %d\n", gpio, level);
    return 0; // Success
}
