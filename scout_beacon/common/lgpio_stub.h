/**
 ********************************************************************
 * @file    lgpio_stub.h
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

#ifndef LGPIO_STUB_H
#define LGPIO_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO chip operations */
int lgGpiochipOpen(int chip);
int lgGpiochipClose(int chip);

/* GPIO pin operations */
int lgGpioClaimInput(int handle, int lFlags, int gpio);
int lgGpioFree(int handle, int gpio);
int lgGpioRead(int handle, int gpio);
int lgGpioWrite(int handle, int gpio, int level);

#ifdef __cplusplus
}
#endif

#endif // LGPIO_STUB_H
