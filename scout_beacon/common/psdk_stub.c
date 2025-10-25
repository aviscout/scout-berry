/**
 * @file psdk_stub.c
 * @brief PSDK stub implementation for macOS development
 * 
 * This file provides stub implementations for DJI PSDK functions
 * to allow development and testing on macOS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// Include PSDK headers for type definitions
#include <dji_platform.h>
#include <dji_core.h>
#include <dji_logger.h>
#include <dji_low_speed_data_channel.h>
#include <dji_aircraft_info.h>

/* Core PSDK Functions */

T_DjiReturnCode DjiCore_ApplicationStart(void)
{
    printf("[PSDK_STUB] DjiCore_ApplicationStart() - PSDK core started (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_ApplicationStop(void)
{
    printf("[PSDK_STUB] DjiCore_ApplicationStop() - PSDK core stopped (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_ApplicationRun(void)
{
    printf("[PSDK_STUB] DjiCore_ApplicationRun() - PSDK core running (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Logger Functions */

T_DjiReturnCode DjiLogger_Init(void)
{
    printf("[PSDK_STUB] DjiLogger_Init() - Logger initialized (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiLogger_DeInit(void)
{
    printf("[PSDK_STUB] DjiLogger_DeInit() - Logger deinitialized (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiLogger_Log(E_DjiLoggerConsoleLogLevel level, const char *format, ...)
{
    (void)(level);
    (void)(format);
    // Stub implementation - just print to stdout
    printf("[PSDK_STUB] Logger: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Low Speed Data Channel Functions */

T_DjiReturnCode DjiLowSpeedDataChannel_Init(void)
{
    printf("[PSDK_STUB] DjiLowSpeedDataChannel_Init() - Low speed data channel initialized (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiLowSpeedDataChannel_DeInit(void)
{
    printf("[PSDK_STUB] DjiLowSpeedDataChannel_DeInit() - Low speed data channel deinitialized (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiLowSpeedDataChannel_SendData(E_DjiChannelAddress channelAddress, 
                                               const uint8_t *data, 
                                               uint8_t len)
{
    (void)(channelAddress);
    
    if (!data || len == 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("[PSDK_STUB] DjiLowSpeedDataChannel_SendData() - Sending %d bytes to channel %d: ", 
           len, channelAddress);
    for (int i = 0; i < len && i < 16; i++) {
        printf("%02X ", data[i]);
    }
    if (len > 16) printf("...");
    printf("\n");
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiLowSpeedDataChannel_RegRecvDataCallback(E_DjiChannelAddress channelAddress,
                                                           DjiLowSpeedDataChannelRecvDataCallback callback)
{
    (void)(callback);
    printf("[PSDK_STUB] DjiLowSpeedDataChannel_RegRecvDataCallback() - Callback registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Registration Functions */

T_DjiReturnCode DjiCore_RegHalUartHandler(const T_DjiHalUartHandler *halUartHandler)
{
    (void)(halUartHandler);
    printf("[PSDK_STUB] DjiCore_RegHalUartHandler() - UART handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_RegHalNetworkHandler(const T_DjiHalNetworkHandler *halNetworkHandler)
{
    (void)(halNetworkHandler);
    printf("[PSDK_STUB] DjiCore_RegHalNetworkHandler() - Network handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_RegHalUsbBulkHandler(const T_DjiHalUsbBulkHandler *halUsbBulkHandler)
{
    (void)(halUsbBulkHandler);
    printf("[PSDK_STUB] DjiCore_RegHalUsbBulkHandler() - USB bulk handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_RegOsalHandler(const T_DjiOsalHandler *osalHandler)
{
    (void)(osalHandler);
    printf("[PSDK_STUB] DjiCore_RegOsalHandler() - OSAL handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

// Note: T_DjiLoggerHandler doesn't exist in PSDK, removing this function

T_DjiReturnCode DjiCore_RegFileSystemHandler(const T_DjiFileSystemHandler *fileSystemHandler)
{
    (void)(fileSystemHandler);
    printf("[PSDK_STUB] DjiCore_RegFileSystemHandler() - File system handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_RegSocketHandler(const T_DjiSocketHandler *socketHandler)
{
    (void)(socketHandler);
    printf("[PSDK_STUB] DjiCore_RegSocketHandler() - Socket handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Additional PSDK Core Functions */

T_DjiReturnCode DjiCore_Init(const T_DjiUserInfo *userInfo)
{
    (void)(userInfo);
    printf("[PSDK_STUB] DjiCore_Init() - PSDK core initialized (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_DeInit(void)
{
    printf("[PSDK_STUB] DjiCore_DeInit() - PSDK core deinitialized (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_SetAlias(const char *alias)
{
    (void)(alias);
    printf("[PSDK_STUB] DjiCore_SetAlias() - Alias set to %s (stub)\n", alias);
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_SetSerialNumber(const char *serialNumber)
{
    (void)(serialNumber);
    printf("[PSDK_STUB] DjiCore_SetSerialNumber() - Serial number set to %s (stub)\n", serialNumber);
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiCore_SetFirmwareVersion(T_DjiFirmwareVersion version)
{
    (void)(version);
    printf("[PSDK_STUB] DjiCore_SetFirmwareVersion() - Firmware version set (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Aircraft Info Functions */

T_DjiReturnCode DjiAircraftInfo_GetBaseInfo(T_DjiAircraftInfoBaseInfo *baseInfo)
{
    if (!baseInfo) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Fill with dummy data
    baseInfo->aircraftSeries = DJI_AIRCRAFT_SERIES_M300;
    baseInfo->mountPositionType = DJI_MOUNT_POSITION_TYPE_PAYLOAD_PORT;
    baseInfo->aircraftType = DJI_AIRCRAFT_TYPE_M300_RTK;
    baseInfo->djiAdapterType = DJI_SDK_ADAPTER_TYPE_XPORT;
    baseInfo->mountPosition = DJI_MOUNT_POSITION_PAYLOAD_PORT_NO1;
    
    printf("[PSDK_STUB] DjiAircraftInfo_GetBaseInfo() - Aircraft info retrieved (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiAircraftInfo_GetAircraftVersion(T_DjiAircraftVersion *aircraftVersion)
{
    if (!aircraftVersion) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Fill with dummy data
    aircraftVersion->majorVersion = 1;
    aircraftVersion->minorVersion = 0;
    aircraftVersion->modifyVersion = 0;
    aircraftVersion->debugVersion = 0;
    
    printf("[PSDK_STUB] DjiAircraftInfo_GetAircraftVersion() - Aircraft version retrieved (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Logger Additional Functions */

T_DjiReturnCode DjiLogger_AddConsole(T_DjiLoggerConsole *console)
{
    (void)(console);
    printf("[PSDK_STUB] DjiLogger_AddConsole() - Console added (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

void DjiLogger_UserLogOutput(E_DjiLoggerConsoleLogLevel level, const char *fmt, ...)
{
    (void)(level);
    (void)(fmt);
    printf("[PSDK_STUB] DjiLogger_UserLogOutput() - User log output (stub)\n");
}

/* Platform Registration Functions */

T_DjiReturnCode DjiPlatform_RegHalUartHandler(const T_DjiHalUartHandler *halUartHandler)
{
    (void)(halUartHandler);
    printf("[PSDK_STUB] DjiPlatform_RegHalUartHandler() - UART handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiPlatform_RegHalNetworkHandler(const T_DjiHalNetworkHandler *halNetworkHandler)
{
    (void)(halNetworkHandler);
    printf("[PSDK_STUB] DjiPlatform_RegHalNetworkHandler() - Network handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiPlatform_RegOsalHandler(const T_DjiOsalHandler *osalHandler)
{
    (void)(osalHandler);
    printf("[PSDK_STUB] DjiPlatform_RegOsalHandler() - OSAL handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiPlatform_RegFileSystemHandler(const T_DjiFileSystemHandler *fileSystemHandler)
{
    (void)(fileSystemHandler);
    printf("[PSDK_STUB] DjiPlatform_RegFileSystemHandler() - File system handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode DjiPlatform_RegSocketHandler(const T_DjiSocketHandler *socketHandler)
{
    (void)(socketHandler);
    printf("[PSDK_STUB] DjiPlatform_RegSocketHandler() - Socket handler registered (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}
