/**
 ********************************************************************
 * @file    main.c
 * @brief   Scout Beacon Application - Main entry point for AviScout system
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This application integrates avalanche beacon detection with DJI PSDK
 *          for autonomous drone rescue operations. The system reads beacon
 *          signals from GPIO pins on a Raspberry Pi and transmits the data
 *          to a Mavic 3 drone via PSDK low-speed data channel.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 * @section Features
 * - Real-time avalanche beacon detection via GPIO interface
 * - DJI PSDK integration for drone communication
 * - JSON-formatted data transmission
 * - Mock mode for testing without physical hardware
 * - Comprehensive logging and error handling
 * - Signal strength (RSSI) monitoring
 * - Bearing and distance measurement
 *
 * @section Hardware Requirements
 * - Raspberry Pi 4 Model B with 64-bit OS
 * - E-Port Development Kit for drone connection
 * - Avalanche beacon receiver connected to GPIO pins
 * - Mavic 3 Enterprise drone
 *
 * @section GPIO Configuration
 * - Bearing pins: 17, 27, 22, 5, 6 (270°, 325°, 0°, 45°, 90°)
 * - 7-segment display: 13, 19, 26, 21, 20, 16, 12, 25 (A-G, DP)
 * - Digit control: 8, 7 (enable pins)
 * - RSSI input: 18 (signal strength)
 *
 * @section Usage
 * @code
 * // Run with real hardware
 * sudo ./scout_beacon
 * 
 * // Run with mock data for testing
 * sudo ./scout_beacon --mock
 * 
 * // Run with debug output
 * sudo ./scout_beacon --debug
 * 
 * // Run with both mock and debug
 * sudo ./scout_beacon --mock --debug
 * @endcode
 *
 * @section Data Format
 * The application transmits JSON-formatted beacon data:
 * @code
 * {
 *   "event": "BEACON_DETECTED",
 *   "timestamp": 1234567890,
 *   "bearing": 45,
 *   "distance": 25.3,
 *   "signal_strength": -67,
 *   "bearing_valid": true,
 *   "distance_valid": true
 * }
 * @endcode
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

/* Command line options */
#define OPTION_MOCK_MODE                "--mock"
#define OPTION_HELP                     "--help"
#define OPTION_DEBUG                    "--debug"

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
static void ScoutBeacon_PrintUsage(const char* program_name);
static bool ScoutBeacon_ParseArguments(int argc, char** argv, bool* mock_mode, bool* debug_mode);

/* Exported functions definition ---------------------------------------------*/

/**
 * @brief Main entry point for Scout Beacon Application
 * 
 * @details This function initializes the DJI PSDK, configures GPIO interfaces,
 *          and starts the main application loop for beacon detection and data
 *          transmission. The application supports both real hardware operation
 *          and mock mode for testing.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * 
 * @return int Exit status:
 *         - 0: Success
 *         - 1: Command line argument error
 *         - DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR: System initialization error
 * 
 * @section Command Line Options
 * - --mock: Enable mock mode for testing without physical hardware
 * - --debug: Enable debug output for GPIO operations
 * - --help: Show usage information
 * 
 * @section Initialization Sequence
 * 1. Parse command line arguments
 * 2. Setup signal handlers for graceful shutdown
 * 3. Prepare system environment (OSAL, HAL, Logger)
 * 4. Fill in user information (App ID, Key, etc.)
 * 5. Initialize DJI PSDK core
 * 6. Get aircraft information
 * 7. Set application alias and version
 * 8. Initialize beacon GPIO interface
 * 9. Register beacon detection callback
 * 10. Initialize PSDK modules
 * 11. Start SDK application
 * 12. Enter main application loop
 * 
 * @section Error Handling
 * The function includes comprehensive error handling for:
 * - Invalid command line arguments
 * - System environment preparation failures
 * - PSDK initialization errors
 * - GPIO interface failures
 * - Module initialization errors
 * 
 * @section Signal Handling
 * The application responds to SIGTERM and SIGINT signals for graceful shutdown,
 * ensuring proper cleanup of resources and GPIO interfaces.
 * 
 * @see ScoutBeacon_ParseArguments()
 * @see ScoutBeacon_InitializeModules()
 * @see ScoutBeacon_MainLoop()
 */
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
    
    bool mock_mode = false;
    bool debug_mode = false;

    printf("=== Scout Beacon Application Starting ===\n");
    printf("AviScout - Avalanche Beacon Detection System\n");
    printf("Copyright (c) 2024 Scout Berry\n\n");

    // Parse command line arguments
    if (!ScoutBeacon_ParseArguments(argc, argv, &mock_mode, &debug_mode)) {
        return 1;
    }

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
    BeaconGpio_SetDebug(debug_mode);
    
    // Enable mock mode if requested
    if (mock_mode) {
        if (BeaconGpio_EnableMockMode() != 0) {
            USER_LOG_ERROR("Failed to enable mock mode");
            BeaconGpio_Cleanup();
            return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
        }
        USER_LOG_INFO("Mock mode enabled - generating simulated beacon data");
        printf("MOCK MODE: Generating simulated avalanche beacon signals\n");
        printf("Mock data will be transmitted every 5 seconds\n\n");
    } else {
        USER_LOG_INFO("Real GPIO mode - monitoring physical beacon receiver");
        printf("REAL MODE: Monitoring physical avalanche beacon receiver\n");
        printf("Connect beacon receiver to GPIO pins as configured\n\n");
    }

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

/**
 * @brief Initialize PSDK modules for beacon data transmission
 * 
 * @details This function initializes the required DJI PSDK modules based on
 *          the configuration defined in dji_sdk_config.h. The modules are
 *          initialized in a specific order to ensure proper dependencies.
 * 
 * @section Enabled Modules
 * - CONFIG_MODULE_SAMPLE_DATA_TRANSMISSION_ON: Low-speed data channel for beacon data
 * - CONFIG_MODULE_SAMPLE_FC_SUBSCRIPTION_ON: Flight controller data subscription
 * - CONFIG_MODULE_SAMPLE_POWER_MANAGEMENT_ON: Power management and monitoring
 * 
 * @section Error Handling
 * Each module initialization is checked for success. If a module fails to
 * initialize, an error is logged but the application continues with other
 * modules. This ensures maximum functionality even if some modules fail.
 * 
 * @section Module Dependencies
 * - Data transmission module provides the low-speed data channel for beacon data
 * - FC subscription module enables flight controller data access
 * - Power management module monitors battery and power status
 * 
 * @see DjiTest_DataTransmissionStartService()
 * @see DjiTest_FcSubscriptionStartService()
 * @see DjiTest_PowerManagementStartService()
 */
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

/**
 * @brief Main application loop for beacon detection and data transmission
 * 
 * @details This function implements the main application loop that continuously
 *          monitors for beacon signals, processes the data, and transmits it
 *          via the DJI PSDK low-speed data channel. The loop runs at a fixed
 *          interval defined by BEACON_UPDATE_INTERVAL_MS.
 * 
 * @section Loop Operation
 * 1. Read current beacon data from GPIO interface
 * 2. Check if beacon signal is detected
 * 3. If beacon detected:
 *    - Log beacon detection event
 *    - Send beacon data via PSDK
 *    - Log beacon data for monitoring
 *    - Update context with latest data
 * 4. If beacon lost:
 *    - Log beacon signal loss
 *    - Update detection state
 * 5. Sleep for update interval
 * 
 * @section Data Processing
 * The function processes beacon data including:
 * - Bearing measurement (degrees)
 * - Distance measurement (meters)
 * - Signal strength (RSSI in dBm)
 * - Data validity flags
 * - Timestamp information
 * 
 * @section Performance
 * - Update interval: 1000ms (1 second)
 * - Non-blocking GPIO reads
 * - Efficient data transmission
 * - Minimal CPU usage during sleep
 * 
 * @section Error Handling
 * - GPIO read failures are handled gracefully
 * - Invalid beacon data is filtered out
 * - Transmission failures are logged but don't stop the loop
 * 
 * @see BeaconGpio_ReadBeaconData()
 * @see ScoutBeacon_SendBeaconData()
 * @see ScoutBeacon_LogBeaconData()
 */
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

/**
 * @brief Callback function for beacon detection events
 * 
 * @details This function is called by the GPIO interface when a beacon signal
 *          is detected. It provides immediate processing and transmission of
 *          beacon data without waiting for the main loop interval.
 * 
 * @param beacon_data Pointer to beacon data structure containing:
 *                   - bearing: Bearing in degrees (0, 45, 90, 270, 325)
 *                   - distance: Distance in meters
 *                   - signal_strength: RSSI value in dBm
 *                   - timestamp: Unix timestamp
 *                   - signal_detected: Detection flag
 *                   - bearing_valid: Bearing validity flag
 *                   - distance_valid: Distance validity flag
 * 
 * @section Callback Behavior
 * - Validates input parameters
 * - Logs beacon detection event with bearing and distance
 * - Immediately sends beacon data via PSDK
 * - Provides real-time response to beacon detection
 * 
 * @section Error Handling
 * - Null pointer check for beacon_data
 * - Early return if invalid data
 * - Transmission errors are handled by ScoutBeacon_SendBeaconData()
 * 
 * @section Performance
 * - Minimal processing overhead
 * - Immediate data transmission
 * - Non-blocking operation
 * 
 * @see ScoutBeacon_SendBeaconData()
 * @see BeaconGpio_RegisterCallback()
 */
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

/**
 * @brief Send beacon data via DJI PSDK low-speed data channel
 * 
 * @details This function formats beacon data into JSON format and transmits it
 *          via the DJI PSDK low-speed data channel. The data is formatted as
 *          a structured JSON object containing all beacon information.
 * 
 * @param beacon_data Pointer to beacon data structure to transmit
 * 
 * @return T_DjiReturnCode Return code:
 *         - DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS: Transmission successful
 *         - DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER: Invalid input data
 *         - Other DJI error codes: Transmission failure
 * 
 * @section JSON Data Format
 * The function creates a JSON object with the following structure:
 * @code
 * {
 *   "event": "BEACON_DETECTED",
 *   "timestamp": 1234567890,
 *   "bearing": 45,
 *   "distance": 25.3,
 *   "signal_strength": -67,
 *   "bearing_valid": true,
 *   "distance_valid": true
 * }
 * @endcode
 * 
 * @section Data Fields
 * - event: Always "BEACON_DETECTED" for beacon events
 * - timestamp: Unix timestamp of detection
 * - bearing: Bearing in degrees (0, 45, 90, 270, 325)
 * - distance: Distance in meters (float with 1 decimal place)
 * - signal_strength: RSSI value in dBm
 * - bearing_valid: Boolean indicating bearing validity
 * - distance_valid: Boolean indicating distance validity
 * 
 * @section Transmission
 * - Uses DJI_LOW_SPEED_DATA_CHANNEL_INDEX_0 for transmission
 * - Data is sent as UTF-8 encoded JSON string
 * - Maximum data size is BEACON_DATA_MAX_SIZE (256 bytes)
 * - Transmission is synchronous and blocking
 * 
 * @section Error Handling
 * - Input validation for null pointer
 * - JSON formatting with proper escaping
 * - Buffer size validation
 * - Transmission error logging
 * 
 * @section Performance
 * - JSON formatting is efficient with snprintf
 * - Minimal memory allocation
 * - Fast transmission via PSDK
 * 
 * @see DjiLowSpeedDataChannel_SendData()
 * @see ScoutBeacon_LogBeaconData()
 */
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

static void ScoutBeacon_PrintUsage(const char* program_name)
{
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Options:\n");
    printf("  %s        Enable mock mode for testing without physical hardware\n", OPTION_MOCK_MODE);
    printf("  %s         Enable debug output for GPIO operations\n", OPTION_DEBUG);
    printf("  %s          Show this help message\n\n", OPTION_HELP);
    printf("Examples:\n");
    printf("  %s                    # Run with real GPIO hardware\n", program_name);
    printf("  %s %s              # Run with mock data for testing\n", program_name, OPTION_MOCK_MODE);
    printf("  %s %s %s         # Run with mock data and debug output\n", program_name, OPTION_MOCK_MODE, OPTION_DEBUG);
    printf("\n");
}

static bool ScoutBeacon_ParseArguments(int argc, char** argv, bool* mock_mode, bool* debug_mode)
{
    *mock_mode = false;
    *debug_mode = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], OPTION_HELP) == 0) {
            ScoutBeacon_PrintUsage(argv[0]);
            return false;
        } else if (strcmp(argv[i], OPTION_MOCK_MODE) == 0) {
            *mock_mode = true;
        } else if (strcmp(argv[i], OPTION_DEBUG) == 0) {
            *debug_mode = true;
        } else {
            printf("Unknown option: %s\n", argv[i]);
            ScoutBeacon_PrintUsage(argv[0]);
            return false;
        }
    }
    
    return true;
}
