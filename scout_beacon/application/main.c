/**
 ********************************************************************
 * @file    main.c
 * @brief   Scout Beacon Application - Main entry point
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * This application integrates avalanche beacon detection with DJI PSDK
 * for autonomous drone rescue operations.
 *
 *********************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <dji_platform.h>
#include <dji_logger.h>
#include <dji_core.h>
#include <dji_low_speed_data_channel.h>
#include <dji_data_transmission.h>
#include <utils/util_misc.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "monitor/sys_monitor.h"
#include "osal/osal.h"
#include "osal/osal_fs.h"
#include "osal/osal_socket.h"
#include "../hal/hal_uart.h"
#include "../hal/hal_network.h"
#include "../hal/hal_usb_bulk.h"
#include "dji_sdk_app_info.h"
#include "dji_aircraft_info.h"
#include "data_transmission/test_data_transmission.h"
#include "dji_sdk_config.h"
#include "beacon_gpio.h"

/* Private constants ---------------------------------------------------------*/
#define DJI_LOG_PATH                    "Logs/DJI"
#define DJI_LOG_INDEX_FILE_NAME         "Logs/index"
#define DJI_LOG_FOLDER_NAME             "Logs"
#define DJI_LOG_PATH_MAX_SIZE           (128)
#define DJI_LOG_FOLDER_NAME_MAX_SIZE    (32)
#define DJI_LOG_MAX_COUNT               (10)
#define BEACON_DATA_MAX_SIZE            (256)
#define BEACON_UPDATE_INTERVAL_MS       (1000)  // 1 second

/* Private types -------------------------------------------------------------*/
typedef struct {
    pthread_t monitor_thread;
    bool running;
    bool beacon_detected;
    BeaconData_t last_beacon_data;
    uint32_t last_transmission_time;
} ScoutBeaconContext_t;

/* Private variables ---------------------------------------------------------*/
static ScoutBeaconContext_t s_context = {0};
static FILE *s_djiLogFile;
static FILE *s_djiLogFileCnt;

/* Private function prototypes -----------------------------------------------*/
static T_DjiReturnCode DjiUser_PrepareSystemEnvironment(void);
static T_DjiReturnCode DjiUser_CleanSystemEnvironment(void);
static T_DjiReturnCode DjiUser_FillInUserInfo(T_DjiUserInfo *userInfo);
static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_LocalWrite(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_LocalWriteFsInit(const char *path);
static void *DjiUser_MonitorTask(void *argument);
static void DjiUser_NormalExitHandler(int signalNum);
static void ScoutBeacon_BeaconCallback(BeaconData_t* beacon_data);
static T_DjiReturnCode ScoutBeacon_SendBeaconData(const BeaconData_t* beacon_data);
static void ScoutBeacon_LogBeaconData(const BeaconData_t* beacon_data);
static void ScoutBeacon_InitializeModules(void);
static void ScoutBeacon_MainLoop(void);

/* Exported functions definition ---------------------------------------------*/
int main(int argc, char **argv)
{
    T_DjiReturnCode returnCode;
    T_DjiUserInfo userInfo;
    T_DjiAircraftInfoBaseInfo aircraftInfoBaseInfo;
    T_DjiAircraftVersion aircraftInfoVersion;
    T_DjiFirmwareVersion firmwareVersion = {
        .majorVersion = 1,
        .minorVersion = 0,
        .modifyVersion = 0,
        .debugVersion = 0,
    };

    USER_UTIL_UNUSED(argc);
    USER_UTIL_UNUSED(argv);

    printf("=== Scout Beacon Application Starting ===\n");
    printf("AviScout - Avalanche Beacon Detection System\n");
    printf("Copyright (c) 2024 Scout Berry\n\n");

    // Setup signal handlers for graceful shutdown
    signal(SIGTERM, DjiUser_NormalExitHandler);
    signal(SIGINT, DjiUser_NormalExitHandler);

    /*!< Step 1: Prepare system environment */
    returnCode = DjiUser_PrepareSystemEnvironment();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("Prepare system environment error");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    /*!< Step 2: Fill application information */
    returnCode = DjiUser_FillInUserInfo(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("Fill user info error, please check user info config");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    /*!< Step 3: Initialize the Payload SDK core */
    returnCode = DjiCore_Init(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("Core init error");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    // Get aircraft information
    returnCode = DjiAircraftInfo_GetBaseInfo(&aircraftInfoBaseInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("get aircraft base info error");
    } else {
        USER_LOG_INFO("Aircraft type: %d, Mount position: %d", 
                     aircraftInfoBaseInfo.aircraftType, 
                     aircraftInfoBaseInfo.mountPosition);
    }

    if (aircraftInfoBaseInfo.mountPositionType != DJI_MOUNT_POSITION_TYPE_PAYLOAD_PORT) {
        returnCode = DjiAircraftInfo_GetAircraftVersion(&aircraftInfoVersion);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            USER_LOG_ERROR("get aircraft version info error");
        } else {
            USER_LOG_INFO("Aircraft version is V%02d.%02d.%02d.%02d", 
                         aircraftInfoVersion.majorVersion,
                         aircraftInfoVersion.minorVersion, 
                         aircraftInfoVersion.modifyVersion,
                         aircraftInfoVersion.debugVersion);
        }
    }

    // Set application alias and version
    returnCode = DjiCore_SetAlias("AviScout_Beacon");
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("set alias error");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    returnCode = DjiCore_SetFirmwareVersion(firmwareVersion);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("set firmware version error");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    returnCode = DjiCore_SetSerialNumber("AVISCOUT001");
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("set serial number error");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    /*!< Step 4: Initialize beacon GPIO interface */
    if (BeaconGpio_Init() != 0) {
        USER_LOG_ERROR("Failed to initialize beacon GPIO interface");
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    // Register beacon detection callback
    if (BeaconGpio_RegisterCallback(ScoutBeacon_BeaconCallback) != 0) {
        USER_LOG_ERROR("Failed to register beacon callback");
        BeaconGpio_Cleanup();
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    // Enable debug output for GPIO
    BeaconGpio_SetDebug(true);

    /*!< Step 5: Initialize PSDK modules */
    ScoutBeacon_InitializeModules();

    /*!< Step 6: Start SDK application */
    returnCode = DjiCore_StartApplication();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("start application error");
        BeaconGpio_Cleanup();
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }

    USER_LOG_INFO("Scout Beacon Application started successfully");
    printf("Beacon detection active. Monitoring for avalanche beacon signals...\n");
    printf("Press Ctrl+C to exit.\n\n");

    /*!< Step 7: Main application loop */
    ScoutBeacon_MainLoop();

    /*!< Step 8: Cleanup and exit */
    USER_LOG_INFO("Scout Beacon Application shutting down");
    BeaconGpio_Cleanup();
    DjiUser_CleanSystemEnvironment();
    
    printf("Scout Beacon Application terminated.\n");
    return 0;
}

/* Private functions ---------------------------------------------------------*/

static void ScoutBeacon_InitializeModules(void)
{
    T_DjiReturnCode returnCode;

#ifdef CONFIG_MODULE_SAMPLE_DATA_TRANSMISSION_ON
    returnCode = DjiTest_DataTransmissionStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("Data transmission module init error");
    } else {
        USER_LOG_INFO("Data transmission module initialized");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_FC_SUBSCRIPTION_ON
    returnCode = DjiTest_FcSubscriptionStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("FC subscription module init error");
    } else {
        USER_LOG_INFO("FC subscription module initialized");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_POWER_MANAGEMENT_ON
    returnCode = DjiTest_PowerManagementStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("Power management module init error");
    } else {
        USER_LOG_INFO("Power management module initialized");
    }
#endif
}

static void ScoutBeacon_MainLoop(void)
{
    s_context.running = true;
    
    while (s_context.running) {
        // Read current beacon data
        BeaconData_t current_data;
        if (BeaconGpio_ReadBeaconData(&current_data) == 0) {
            // Check if beacon is detected
            if (current_data.signal_detected) {
                if (!s_context.beacon_detected) {
                    USER_LOG_INFO("BEACON DETECTED! Bearing: %d°, Distance: %.1fm, RSSI: %ddBm", 
                                 current_data.bearing, current_data.distance, current_data.signal_strength);
                    s_context.beacon_detected = true;
                }
                
                // Send beacon data via PSDK
                ScoutBeacon_SendBeaconData(&current_data);
                
                // Log beacon data
                ScoutBeacon_LogBeaconData(&current_data);
                
                s_context.last_beacon_data = current_data;
            } else {
                if (s_context.beacon_detected) {
                    USER_LOG_INFO("Beacon signal lost");
                    s_context.beacon_detected = false;
                }
            }
        }
        
        // Sleep for update interval
        usleep(BEACON_UPDATE_INTERVAL_MS * 1000);
    }
}

static void ScoutBeacon_BeaconCallback(BeaconData_t* beacon_data)
{
    if (!beacon_data) {
        return;
    }
    
    USER_LOG_INFO("Beacon callback triggered - Bearing: %d°, Distance: %.1fm", 
                 beacon_data->bearing, beacon_data->distance);
    
    // Send beacon data immediately when callback is triggered
    ScoutBeacon_SendBeaconData(beacon_data);
}

static T_DjiReturnCode ScoutBeacon_SendBeaconData(const BeaconData_t* beacon_data)
{
    if (!beacon_data) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Create JSON-like data structure for transmission
    char beacon_json[BEACON_DATA_MAX_SIZE];
    snprintf(beacon_json, sizeof(beacon_json),
        "{"
        "\"event\":\"BEACON_DETECTED\","
        "\"timestamp\":%u,"
        "\"bearing\":%d,"
        "\"distance\":%.1f,"
        "\"signal_strength\":%d,"
        "\"bearing_valid\":%s,"
        "\"distance_valid\":%s"
        "}",
        beacon_data->timestamp,
        beacon_data->bearing,
        beacon_data->distance,
        beacon_data->signal_strength,
        beacon_data->bearing_valid ? "true" : "false",
        beacon_data->distance_valid ? "true" : "false"
    );
    
    // Send via low-speed data channel
    T_DjiReturnCode returnCode = DjiLowSpeedDataChannel_SendData(
        DJI_LOW_SPEED_DATA_CHANNEL_INDEX_0,
        (uint8_t*)beacon_json,
        strlen(beacon_json)
    );
    
    if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_INFO("Beacon data transmitted successfully");
        s_context.last_transmission_time = time(NULL);
    } else {
        USER_LOG_ERROR("Failed to transmit beacon data: 0x%08X", returnCode);
    }
    
    return returnCode;
}

static void ScoutBeacon_LogBeaconData(const BeaconData_t* beacon_data)
{
    printf("[BEACON] Time: %u, Bearing: %d°, Distance: %.1fm, RSSI: %ddBm, Valid: B=%s D=%s\n",
           beacon_data->timestamp,
           beacon_data->bearing,
           beacon_data->distance,
           beacon_data->signal_strength,
           beacon_data->bearing_valid ? "Y" : "N",
           beacon_data->distance_valid ? "Y" : "N");
}

/* System environment functions (copied from manifold2 template) */
static T_DjiReturnCode DjiUser_PrepareSystemEnvironment(void)
{
    T_DjiReturnCode returnCode;
    
    // Initialize OSAL
    returnCode = DjiUser_OsalInit();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }
    
    // Initialize HAL
    returnCode = DjiUser_HalInit();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }
    
    // Initialize logger
    returnCode = DjiUser_LoggerInit();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }
    
    // Initialize file system
    returnCode = DjiUser_LocalWriteFsInit(DJI_LOG_PATH);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return returnCode;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

static T_DjiReturnCode DjiUser_CleanSystemEnvironment(void)
{
    DjiUser_LoggerDeInit();
    DjiUser_HalDeInit();
    DjiUser_OsalDeInit();
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

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

static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen)
{
    if (!data || dataLen == 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    printf("%.*s", dataLen, data);
    fflush(stdout);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

static T_DjiReturnCode DjiUser_LocalWrite(const uint8_t *data, uint16_t dataLen)
{
    if (!data || dataLen == 0 || !s_djiLogFile) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    fwrite(data, 1, dataLen, s_djiLogFile);
    fflush(s_djiLogFile);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

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

static void *DjiUser_MonitorTask(void *argument)
{
    USER_UTIL_UNUSED(argument);
    
    while (s_context.running) {
        // Monitor system resources
        sleep(5);
    }
    
    return NULL;
}

static void DjiUser_NormalExitHandler(int signalNum)
{
    USER_UTIL_UNUSED(signalNum);
    
    printf("\nReceived exit signal. Shutting down gracefully...\n");
    s_context.running = false;
    
    if (s_djiLogFile) {
        fclose(s_djiLogFile);
        s_djiLogFile = NULL;
    }
    
    if (s_djiLogFileCnt) {
        fclose(s_djiLogFileCnt);
        s_djiLogFileCnt = NULL;
    }
}
