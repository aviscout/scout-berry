/**
 ********************************************************************
 * @file    get_drone_heartbeat.c
 * @brief   Simple script to get drone heartbeat and verify PSDK communication
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This script initializes DJI PSDK and subscribes to flight controller
 *          heartbeat data to verify communication with the drone. It's designed
 *          as a standalone tool for initial communication testing.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 * @section Usage
 * @code
 * // Build the script
 * cd scout_beacon/build
 * cmake ..
 * make get_drone_heartbeat
 *
 * // Run the script
 * sudo ./bin/get_drone_heartbeat
 * @endcode
 *
 * @section Features
 * - PSDK initialization and connection verification
 * - Aircraft information retrieval
 * - Flight controller heartbeat monitoring
 * - Connection status display
 * - Graceful shutdown handling
 *
 *********************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <dji_platform.h>
#include <dji_logger.h>
#include <dji_core.h>
#include <dji_aircraft_info.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "utils/util_misc.h"
#include "osal/osal.h"
#include "osal/osal_fs.h"
#include "osal/osal_socket.h"
#include "../hal/hal_uart.h"
#include "../hal/hal_network.h"
#include "../hal/hal_usb_bulk.h"
#include "dji_sdk_app_info.h"
#include "dji_sdk_config.h"

// Aircraft info types - try local header first, then PSDK header
// Types T_DjiAircraftInfoBaseInfo and T_DjiAircraftVersion are defined in PSDK
// They should be available via dji_core.h or dji_aircraft_info.h from PSDK includes

/* Private constants ---------------------------------------------------------*/
#define DJI_LOG_PATH                    "Logs/DJI"
#define DJI_LOG_INDEX_FILE_NAME         "Logs/index"
#define DJI_LOG_FOLDER_NAME             "Logs"
#define DJI_LOG_PATH_MAX_SIZE           (128)
#define HEARTBEAT_UPDATE_INTERVAL_MS    (1000)  // 1 second

/* Private types -------------------------------------------------------------*/
typedef struct {
    bool running;
    bool connected;
    uint32_t heartbeat_count;
    time_t start_time;
    time_t last_update_time;
} HeartbeatContext_t;

/* Private variables ---------------------------------------------------------*/
static HeartbeatContext_t s_context = {0};
static FILE *s_djiLogFile;
static FILE *s_djiLogFileCnt;

/* Private function prototypes -----------------------------------------------*/
static T_DjiReturnCode DjiUser_PrepareSystemEnvironment(void);
static T_DjiReturnCode DjiUser_CleanSystemEnvironment(void);
static T_DjiReturnCode DjiUser_FillInUserInfo(T_DjiUserInfo *userInfo);
static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_LocalWrite(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_LocalWriteFsInit(const char *path);
static void DjiUser_NormalExitHandler(int signalNum);
static void Heartbeat_DisplayAircraftInfo(T_DjiAircraftInfoBaseInfo *baseInfo, T_DjiAircraftVersion *version);
static void Heartbeat_MainLoop(void);
static void Heartbeat_PrintStatus(void);
static T_DjiReturnCode Heartbeat_CheckUartDevice(void);
static void Heartbeat_PrintErrorDiagnostics(T_DjiReturnCode errorCode);

/* Exported functions definition ---------------------------------------------*/

/**
 * @brief Main entry point for drone heartbeat script
 * 
 * @details This function initializes the DJI PSDK, retrieves aircraft information,
 *          and enters a main loop to monitor heartbeat status. The script provides
 *          a simple way to verify PSDK communication with the drone.
 * 
 * @param argc Number of command line arguments (unused)
 * @param argv Array of command line argument strings (unused)
 * 
 * @return int Exit status:
 *         - 0: Success
 *         - 1: Initialization error
 */
int main(int argc, char **argv)
{
    (void)(argc);
    (void)(argv);
    
    T_DjiReturnCode returnCode;
    T_DjiUserInfo userInfo;
    T_DjiAircraftInfoBaseInfo aircraftInfoBaseInfo;
    T_DjiAircraftVersion aircraftInfoVersion;
    
    printf("=== Drone Heartbeat Monitor ===\n");
    printf("AviScout - PSDK Communication Test\n");
    printf("Copyright (c) 2024 Scout Berry\n\n");
    
    // Setup signal handlers for graceful shutdown
    signal(SIGTERM, DjiUser_NormalExitHandler);
    signal(SIGINT, DjiUser_NormalExitHandler);
    
    /*!< Step 0: Pre-initialization checks */
    printf("Performing pre-initialization checks...\n");
    returnCode = Heartbeat_CheckUartDevice();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("ERROR: UART device check failed\n");
        printf("       Please ensure UART device is connected and accessible\n");
        return 1;
    }
    printf("UART device check passed\n\n");
    
    /*!< Step 1: Prepare system environment */
    printf("Initializing PSDK system environment...\n");
    returnCode = DjiUser_PrepareSystemEnvironment();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("ERROR: Prepare system environment failed (0x%08X)\n", returnCode);
        Heartbeat_PrintErrorDiagnostics(returnCode);
        return 1;
    }
    printf("System environment prepared successfully\n");
    
    /*!< Step 2: Fill application information */
    returnCode = DjiUser_FillInUserInfo(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("ERROR: Fill user info failed (0x%08X)\n", returnCode);
        printf("       Check dji_sdk_app_info.h configuration\n");
        DjiUser_CleanSystemEnvironment();
        return 1;
    }
    printf("User info configured: %s (ID: %s)\n", userInfo.appName, userInfo.appId);
    printf("Baud rate: %s\n", userInfo.baudRate);
    
    /*!< Step 3: Initialize the Payload SDK core */
    printf("\nInitializing PSDK core...\n");
    printf("This may take 10-30 seconds depending on drone connection status...\n");
    printf("Waiting for UART adapter to identify connection...\n");
    printf("\nNOTE: If you see repeated 'Try identify UART0 connection failed' messages,\n");
    printf("      the SDK is sending packets but not receiving responses from the drone.\n");
    printf("      This usually means:\n");
    printf("      - PSDK mode needs to be enabled AND drone restarted\n");
    printf("      - Physical connection issue\n");
    printf("      - Wrong UART port or baud rate\n\n");
    fflush(stdout);
    
    returnCode = DjiCore_Init(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("\nERROR: PSDK core initialization failed (0x%08X)\n", returnCode);
        Heartbeat_PrintErrorDiagnostics(returnCode);
        printf("\nTroubleshooting steps:\n");
        printf("1. Verify drone is powered ON and fully booted\n");
        printf("2. In DJI Assistant 2, verify PSDK mode is ENABLED:\n");
        printf("   - Go to 'Onboard SDK' or 'Payload SDK' section\n");
        printf("   - Check 'Enable API Control' or 'Enable Payload SDK'\n");
        printf("   - Verify 'Extension Port' shows as 'Active'\n");
        printf("3. CRITICAL: Restart the drone after enabling PSDK mode:\n");
        printf("   - Power OFF drone completely\n");
        printf("   - Wait 10 seconds\n");
        printf("   - Power ON drone and wait for full boot\n");
        printf("4. Verify UART cable connection between Pi and drone\n");
        printf("5. Check UART device: ls -la /dev/ttyUSB0\n");
        printf("6. Review PSDK logs: tail -f Logs/DJI/psdk_log_*.txt\n");
        printf("7. See docs/HOW_TO_VERIFY_PSDK_MODE.md for detailed instructions\n");
        DjiUser_CleanSystemEnvironment();
        return 1;
    }
    printf("\nPSDK core initialized successfully!\n");
    
    /*!< Step 4: Get aircraft information */
    printf("\nRetrieving aircraft information...\n");
    returnCode = DjiAircraftInfo_GetBaseInfo(&aircraftInfoBaseInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to get aircraft base info (0x%08X)\n", returnCode);
        printf("         Connection may not be fully established\n");
    } else {
        Heartbeat_DisplayAircraftInfo(&aircraftInfoBaseInfo, NULL);
    }
    
    if (aircraftInfoBaseInfo.mountPositionType != DJI_MOUNT_POSITION_TYPE_PAYLOAD_PORT) {
        returnCode = DjiAircraftInfo_GetAircraftVersion(&aircraftInfoVersion);
        if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            Heartbeat_DisplayAircraftInfo(&aircraftInfoBaseInfo, &aircraftInfoVersion);
        }
    }
    
    /*!< Step 5: Set application alias */
    returnCode = DjiCore_SetAlias("AviScout_Heartbeat");
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to set alias\n");
    }
    
    /*!< Step 6: Start SDK application */
    printf("\nStarting SDK application...\n");
    returnCode = DjiCore_ApplicationStart();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("ERROR: Failed to start SDK application (0x%08X)\n", returnCode);
        DjiUser_CleanSystemEnvironment();
        return 1;
    }
    printf("SDK application started successfully\n");
    
    s_context.connected = true;
    s_context.running = true;
    s_context.start_time = time(NULL);
    s_context.last_update_time = s_context.start_time;
    
    printf("\n=== Heartbeat Monitor Active ===\n");
    printf("Connection Status: CONNECTED\n");
    printf("Monitoring heartbeat...\n");
    printf("Press Ctrl+C to exit\n\n");
    
    /*!< Step 7: Main monitoring loop */
    Heartbeat_MainLoop();
    
    /*!< Step 8: Cleanup and exit */
    printf("\nShutting down gracefully...\n");
    DjiUser_CleanSystemEnvironment();
    
    printf("Heartbeat monitor terminated.\n");
    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Display aircraft information
 * 
 * @details Prints formatted aircraft information including type, mount position,
 *          and firmware version.
 * 
 * @param baseInfo Pointer to aircraft base info structure
 * @param version Pointer to aircraft version structure (can be NULL)
 */
static void Heartbeat_DisplayAircraftInfo(T_DjiAircraftInfoBaseInfo *baseInfo, T_DjiAircraftVersion *version)
{
    if (!baseInfo) {
        return;
    }
    
    printf("--- Aircraft Information ---\n");
    printf("Aircraft Type: %d\n", baseInfo->aircraftType);
    printf("Mount Position: %d\n", baseInfo->mountPosition);
    printf("Mount Position Type: %d\n", baseInfo->mountPositionType);
    
    if (version) {
        printf("Firmware Version: V%02d.%02d.%02d.%02d\n",
               version->majorVersion,
               version->minorVersion,
               version->modifyVersion,
               version->debugVersion);
    }
    printf("----------------------------\n");
}

/**
 * @brief Main monitoring loop for heartbeat display
 * 
 * @details Continuously monitors connection status and displays heartbeat
 *          information at regular intervals.
 */
static void Heartbeat_MainLoop(void)
{
    time_t current_time;
    time_t last_display_time = 0;
    
    while (s_context.running) {
        current_time = time(NULL);
        
        // Update heartbeat count
        s_context.heartbeat_count++;
        
        // Display status every second
        if (current_time != last_display_time) {
            Heartbeat_PrintStatus();
            last_display_time = current_time;
        }
        
        // Sleep for a short interval to avoid busy-waiting
        usleep(100000);  // 100ms
    }
}

/**
 * @brief Print current heartbeat status
 * 
 * @details Displays formatted heartbeat information including connection status,
 *          uptime, and heartbeat count.
 */
static void Heartbeat_PrintStatus(void)
{
    time_t current_time = time(NULL);
    time_t uptime = current_time - s_context.start_time;
    
    printf("[Heartbeat] Status: %s | Uptime: %lu s | Count: %lu | Time: %s",
           s_context.connected ? "CONNECTED" : "DISCONNECTED",
           (unsigned long)uptime,
           (unsigned long)s_context.heartbeat_count,
           ctime(&current_time));
    
    // Remove newline from ctime output
    printf("\b");
    fflush(stdout);
    
    s_context.last_update_time = current_time;
}

/**
 * @brief Prepare system environment for PSDK
 * 
 * @details Initializes OSAL, HAL, logger, and file system handlers required
 *          for PSDK operation. This function sets up all the platform-specific
 *          handlers that PSDK needs to function.
 * 
 * @return T_DjiReturnCode Return code:
 *         - DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS: Success
 *         - Other: Error code
 */
static T_DjiReturnCode DjiUser_PrepareSystemEnvironment(void)
{
    T_DjiReturnCode returnCode;
    T_DjiOsalHandler osalHandler = {
        .TaskCreate = Osal_TaskCreate,
        .TaskDestroy = Osal_TaskDestroy,
        .TaskSleepMs = Osal_TaskSleepMs,
        .MutexCreate = Osal_MutexCreate,
        .MutexDestroy = Osal_MutexDestroy,
        .MutexLock = Osal_MutexLock,
        .MutexUnlock = Osal_MutexUnlock,
        .SemaphoreCreate = Osal_SemaphoreCreate,
        .SemaphoreDestroy = Osal_SemaphoreDestroy,
        .SemaphoreWait = Osal_SemaphoreWait,
        .SemaphoreTimedWait = Osal_SemaphoreTimedWait,
        .SemaphorePost = Osal_SemaphorePost,
        .Malloc = Osal_Malloc,
        .Free = Osal_Free,
        .GetTimeMs = Osal_GetTimeMs,
        .GetTimeUs = Osal_GetTimeUs,
        .GetRandomNum = Osal_GetRandomNum,
    };
    
    T_DjiLoggerConsole printConsole = {
        .func = DjiUser_PrintConsole,
        .consoleLevel = DJI_LOGGER_CONSOLE_LOG_LEVEL_INFO,
        .isSupportColor = true,
    };
    
    T_DjiLoggerConsole localRecordConsole = {
        .consoleLevel = DJI_LOGGER_CONSOLE_LOG_LEVEL_DEBUG,
        .func = DjiUser_LocalWrite,
        .isSupportColor = true,
    };
    
    T_DjiHalUartHandler uartHandler = {
        .UartInit = HalUart_Init,
        .UartDeInit = HalUart_DeInit,
        .UartWriteData = HalUart_WriteData,
        .UartReadData = HalUart_ReadData,
        .UartGetStatus = HalUart_GetStatus,
    };
    
    T_DjiHalNetworkHandler networkHandler = {
        .NetworkInit = HalNetWork_Init,
        .NetworkDeInit = HalNetWork_DeInit,
        .NetworkGetDeviceInfo = HalNetWork_GetDeviceInfo,
    };
    
    T_DjiHalUsbBulkHandler usbBulkHandler = {
        .UsbBulkInit = HalUsbBulk_Init,
        .UsbBulkDeInit = HalUsbBulk_DeInit,
        .UsbBulkWriteData = HalUsbBulk_WriteData,
        .UsbBulkReadData = HalUsbBulk_ReadData,
        .UsbBulkGetDeviceInfo = HalUsbBulk_GetDeviceInfo,
    };
    
    T_DjiFileSystemHandler fileSystemHandler = {
        .FileOpen = Osal_FileOpen,
        .FileClose = Osal_FileClose,
        .FileWrite = Osal_FileWrite,
        .FileRead = Osal_FileRead,
        .FileSync = Osal_FileSync,
        .FileSeek = Osal_FileSeek,
        .DirOpen = Osal_DirOpen,
        .DirClose = Osal_DirClose,
        .DirRead = Osal_DirRead,
        .Mkdir = Osal_Mkdir,
        .Unlink = Osal_Unlink,
        .Rename = Osal_Rename,
        .Stat = Osal_Stat,
    };
    
    T_DjiSocketHandler socketHandler = {
        .Socket = Osal_Socket,
        .Bind = Osal_Bind,
        .Close = Osal_Close,
        .UdpSendData = Osal_UdpSendData,
        .UdpRecvData = Osal_UdpRecvData,
        .TcpListen = Osal_TcpListen,
        .TcpAccept = Osal_TcpAccept,
        .TcpConnect = Osal_TcpConnect,
        .TcpSendData = Osal_TcpSendData,
        .TcpRecvData = Osal_TcpRecvData,
    };
    
    // Register OSAL handler
    returnCode = DjiPlatform_RegOsalHandler(&osalHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("ERROR: Failed to register OSAL handler\n");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    // Register UART handler
    returnCode = DjiPlatform_RegHalUartHandler(&uartHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("ERROR: Failed to register UART handler\n");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    // Initialize log file system
    if (DjiUser_LocalWriteFsInit(DJI_LOG_PATH) != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to initialize log file system\n");
    }
    
    // Register console logger
    returnCode = DjiLogger_AddConsole(&printConsole);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to add console logger\n");
    }
    
    // Register file logger
    returnCode = DjiLogger_AddConsole(&localRecordConsole);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to add file logger\n");
    }
    
    // Register hardware connection handlers based on configuration
#if (CONFIG_HARDWARE_CONNECTION == DJI_USE_UART_AND_USB_BULK_DEVICE)
    returnCode = DjiPlatform_RegHalUsbBulkHandler(&usbBulkHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to register USB bulk handler\n");
    }
#elif (CONFIG_HARDWARE_CONNECTION == DJI_USE_UART_AND_NETWORK_DEVICE)
    returnCode = DjiPlatform_RegHalNetworkHandler(&networkHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to register network handler\n");
    }
    
    returnCode = DjiPlatform_RegSocketHandler(&socketHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to register socket handler\n");
    }
#elif (CONFIG_HARDWARE_CONNECTION == DJI_USE_ONLY_UART)
    // Only UART connection - no additional handlers needed
#endif
    
    // Register file system handler
    returnCode = DjiPlatform_RegFileSystemHandler(&fileSystemHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Failed to register file system handler\n");
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Clean up environment
 * 
 * @details Deinitializes PSDK core and cleans up resources.
 * 
 * @return T_DjiReturnCode Return code
 */
static T_DjiReturnCode DjiUser_CleanSystemEnvironment(void)
{
    T_DjiReturnCode returnCode;
    
    returnCode = DjiCore_DeInit();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("WARNING: Core deinit failed\n");
    }
    
    if (s_djiLogFile) {
        fclose(s_djiLogFile);
        s_djiLogFile = NULL;
    }
    
    if (s_djiLogFileCnt) {
        fclose(s_djiLogFileCnt);
        s_djiLogFileCnt = NULL;
    }
    
    return returnCode;
}

/**
 * @brief Fill in user information for PSDK
 * 
 * @details Populates the user info structure with application credentials
 *          from dji_sdk_app_info.h.
 * 
 * @param userInfo Pointer to user info structure to fill
 * @return T_DjiReturnCode Return code
 */
static T_DjiReturnCode DjiUser_FillInUserInfo(T_DjiUserInfo *userInfo)
{
    if (!userInfo) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    strcpy(userInfo->appName, USER_APP_NAME);
    strcpy(userInfo->appId, USER_APP_ID);
    strcpy(userInfo->appKey, USER_APP_KEY);
    strcpy(userInfo->appLicense, USER_APP_LICENSE);
    strcpy(userInfo->developerAccount, USER_DEVELOPER_ACCOUNT);
    strcpy(userInfo->baudRate, USER_BAUD_RATE);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Print console output handler
 * 
 * @details Handler function for PSDK console logging.
 * 
 * @param data Pointer to data to print
 * @param dataLen Length of data
 * @return T_DjiReturnCode Return code
 */
static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen)
{
    if (!data || dataLen == 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("%.*s", dataLen, data);
    fflush(stdout);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Local write handler for file logging
 * 
 * @details Handler function for PSDK file logging.
 * 
 * @param data Pointer to data to write
 * @param dataLen Length of data
 * @return T_DjiReturnCode Return code
 */
static T_DjiReturnCode DjiUser_LocalWrite(const uint8_t *data, uint16_t dataLen)
{
    if (!data || dataLen == 0 || !s_djiLogFile) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    fwrite(data, 1, dataLen, s_djiLogFile);
    fflush(s_djiLogFile);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Initialize local write file system
 * 
 * @details Creates log file and index file for PSDK logging.
 * 
 * @param path Path to log directory
 * @return T_DjiReturnCode Return code
 */
static T_DjiReturnCode DjiUser_LocalWriteFsInit(const char *path)
{
    char logPath[DJI_LOG_PATH_MAX_SIZE];
    char logIndexPath[DJI_LOG_PATH_MAX_SIZE];
    
    snprintf(logPath, sizeof(logPath), "%s/psdk_log_%u.txt", path, (unsigned int)time(NULL));
    snprintf(logIndexPath, sizeof(logIndexPath), "%s", DJI_LOG_INDEX_FILE_NAME);
    
    s_djiLogFile = fopen(logPath, "w");
    if (!s_djiLogFile) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    s_djiLogFileCnt = fopen(logIndexPath, "w");
    if (!s_djiLogFileCnt) {
        fclose(s_djiLogFile);
        s_djiLogFile = NULL;
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Normal exit handler for signal handling
 * 
 * @details Handles SIGTERM and SIGINT signals for graceful shutdown.
 * 
 * @param signalNum Signal number (unused)
 */
static void DjiUser_NormalExitHandler(int signalNum)
{
    (void)(signalNum);
    
    printf("\nReceived exit signal. Shutting down gracefully...\n");
    s_context.running = false;
}

/**
 * @brief Check UART device availability before initialization
 * 
 * @details Verifies that the UART device exists and is accessible before
 *          attempting PSDK initialization. This helps provide better error
 *          messages if the device is not available.
 * 
 * @return T_DjiReturnCode Return code:
 *         - DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS: UART device available
 *         - DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR: UART device not found
 */
static T_DjiReturnCode Heartbeat_CheckUartDevice(void)
{
    const char* uart_devices[] = {"/dev/ttyUSB0", "/dev/ttyACM0", NULL};
    bool device_found = false;
    const char* found_device = NULL;
    
    // Check for common UART device names
    for (int i = 0; uart_devices[i] != NULL; i++) {
        if (access(uart_devices[i], F_OK) == 0) {
            device_found = true;
            found_device = uart_devices[i];
            
            // Check if readable/writable
            if (access(uart_devices[i], R_OK | W_OK) == 0) {
                printf("  Found UART device: %s (readable/writable)\n", found_device);
            } else {
                printf("  Found UART device: %s (permissions issue - may need sudo)\n", found_device);
            }
            break;
        }
    }
    
    if (!device_found) {
        printf("  ERROR: No UART device found (/dev/ttyUSB0 or /dev/ttyACM0)\n");
        printf("  Please connect the UART device and try again\n");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Print detailed error diagnostics based on error code
 * 
 * @details Provides specific error information and troubleshooting guidance
 *          based on the PSDK error code returned.
 * 
 * @param errorCode PSDK error code to diagnose
 */
static void Heartbeat_PrintErrorDiagnostics(T_DjiReturnCode errorCode)
{
    printf("\n--- Error Diagnostics ---\n");
    printf("Error Code: 0x%08X\n", errorCode);
    
    // Common error codes and their meanings
    if (errorCode == 0x000000E1) {
        printf("Error Type: Access Adapter Initialization Timeout\n");
        printf("Meaning: PSDK could not establish communication with the drone\n");
        printf("\nPossible causes:\n");
        printf("  - Drone is not powered on\n");
        printf("  - PSDK mode is not enabled in DJI Assistant 2\n");
        printf("  - UART cable is disconnected or faulty\n");
        printf("  - Wrong UART device selected\n");
        printf("  - Baud rate mismatch\n");
    } else if ((errorCode & 0xFF000000) == 0x00000000) {
        printf("Error Type: System Module Error\n");
        printf("Module Code: 0x%02X\n", (errorCode >> 16) & 0xFF);
        printf("Error Code: 0x%04X\n", errorCode & 0xFFFF);
    } else {
        printf("Error Type: Unknown/Generic Error\n");
        printf("Check PSDK documentation for error code details\n");
    }
    
    printf("--- End Diagnostics ---\n");
}
