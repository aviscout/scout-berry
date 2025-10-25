/**
 ********************************************************************
 * @file    hal_uart_stub.c
 * @brief   macOS-compatible stub implementation of UART HAL functions
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This file provides macOS-compatible stub implementations
 *          of UART HAL functions for development and testing on macOS.
 *          The actual HAL functions are designed for Linux systems.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 *********************************************************************
 */

#include "hal_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* UART operations */
T_DjiReturnCode HalUart_Init(E_DjiHalUartNum uartNum, uint32_t baudRate, T_DjiUartHandle *uartHandle)
{
    (void)(uartNum);
    (void)(baudRate);
    
    if (!uartHandle) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("[HAL_UART_STUB] Initializing UART %d with baud rate %u\n", uartNum, baudRate);
    
    // Return a dummy handle
    *uartHandle = (T_DjiUartHandle)0x12345678;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUart_DeInit(T_DjiUartHandle uartHandle)
{
    (void)(uartHandle);
    
    printf("[HAL_UART_STUB] Deinitializing UART handle %p\n", uartHandle);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUart_WriteData(T_DjiUartHandle uartHandle, const uint8_t *buf, uint32_t len, uint32_t *realLen)
{
    (void)(uartHandle);
    
    if (!buf || len == 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("[HAL_UART_STUB] Writing %u bytes to UART handle %p: ", len, uartHandle);
    for (uint32_t i = 0; i < len && i < 16; i++) {
        printf("%02X ", buf[i]);
    }
    if (len > 16) printf("...");
    printf("\n");
    
    if (realLen) {
        *realLen = len;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUart_ReadData(T_DjiUartHandle uartHandle, uint8_t *buf, uint32_t len, uint32_t *realLen)
{
    (void)(uartHandle);
    (void)(buf);
    (void)(len);
    
    if (!buf || !realLen) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Simulate no data available
    *realLen = 0;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUart_GetStatus(E_DjiHalUartNum uartNum, T_DjiUartStatus *status)
{
    (void)(uartNum);
    
    if (!status) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Simulate UART is always ready
    status->isConnect = true;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}
