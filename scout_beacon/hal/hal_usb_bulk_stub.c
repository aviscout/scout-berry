/**
 ********************************************************************
 * @file    hal_usb_bulk_stub.c
 * @brief   macOS-compatible stub implementation of USB Bulk HAL functions
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This file provides macOS-compatible stub implementations
 *          of USB Bulk HAL functions for development and testing on macOS.
 *          The actual HAL functions are designed for Linux systems.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 *********************************************************************
 */

#include "hal_usb_bulk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* USB Bulk operations */
T_DjiReturnCode HalUsbBulk_Init(T_DjiHalUsbBulkInfo usbBulkInfo, T_DjiUsbBulkHandle *usbBulkHandle)
{
    (void)(usbBulkInfo);
    
    if (!usbBulkHandle) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("[HAL_USB_BULK_STUB] Initializing USB bulk\n");
    
    // Return a dummy handle
    *usbBulkHandle = (T_DjiUsbBulkHandle)0xDEADBEEF;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUsbBulk_DeInit(T_DjiUsbBulkHandle usbBulkHandle)
{
    (void)(usbBulkHandle);
    
    printf("[HAL_USB_BULK_STUB] Deinitializing USB bulk handle %p\n", usbBulkHandle);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUsbBulk_WriteData(T_DjiUsbBulkHandle usbBulkHandle, const uint8_t *buf, uint32_t len, uint32_t *realLen)
{
    (void)(usbBulkHandle);
    
    if (!buf || len == 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("[HAL_USB_BULK_STUB] Writing %u bytes to USB bulk handle %p: ", len, usbBulkHandle);
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

T_DjiReturnCode HalUsbBulk_ReadData(T_DjiUsbBulkHandle usbBulkHandle, uint8_t *buf, uint32_t len, uint32_t *realLen)
{
    (void)(usbBulkHandle);
    (void)(buf);
    (void)(len);
    
    if (!buf || !realLen) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Simulate no data available
    *realLen = 0;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalUsbBulk_GetDeviceInfo(T_DjiHalUsbBulkDeviceInfo *deviceInfo)
{
    if (!deviceInfo) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Simulate USB device info
    deviceInfo->vid = 0x1234;
    deviceInfo->pid = 0x5678;
    
    // Initialize channel info (simplified)
    for (int i = 0; i < DJI_HAL_USB_BULK_NUM_MAX; i++) {
        deviceInfo->channelInfo[i].interfaceNum = i + 1;
        deviceInfo->channelInfo[i].endPointIn = 0x80 + i;
        deviceInfo->channelInfo[i].endPointOut = i;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}
