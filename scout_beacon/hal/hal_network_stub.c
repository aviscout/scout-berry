/**
 ********************************************************************
 * @file    hal_network_stub.c
 * @brief   macOS-compatible stub implementation of Network HAL functions
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This file provides macOS-compatible stub implementations
 *          of Network HAL functions for development and testing on macOS.
 *          The actual HAL functions are designed for Linux systems.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 *********************************************************************
 */

#include "hal_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Network operations */
T_DjiReturnCode HalNetWork_Init(const char *ipAddr, const char *netMask, T_DjiNetworkHandle *halObj)
{
    (void)(ipAddr);
    (void)(netMask);
    
    if (!halObj) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("[HAL_NETWORK_STUB] Initializing network with IP %s, mask %s\n", ipAddr, netMask);
    
    // Return a dummy handle
    *halObj = (T_DjiNetworkHandle)0x87654321;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalNetWork_DeInit(T_DjiNetworkHandle halObj)
{
    (void)(halObj);
    
    printf("[HAL_NETWORK_STUB] Deinitializing network handle %p\n", halObj);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode HalNetWork_GetDeviceInfo(T_DjiHalNetworkDeviceInfo *deviceInfo)
{
    if (!deviceInfo) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Simulate network device info - only has USB net adapter info
    deviceInfo->usbNetAdapter.vid = 0x1234;
    deviceInfo->usbNetAdapter.pid = 0x5678;
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}
