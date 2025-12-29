/**
 ********************************************************************
 * @file    get_psdk_version.c
 * @brief   Simple program to retrieve and display DJI Payload SDK version
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This program initializes the DJI PSDK and displays the SDK version
 *          that is logged during initialization. It's a minimal program designed
 *          specifically for version checking without requiring drone connection.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 * @section Usage
 * @code
 * // Build the program
 * cd scout_beacon/build
 * cmake ..
 * make get_psdk_version
 *
 * // Run the program
 * ./bin/get_psdk_version
 * @endcode
 *
 * @section Output
 * The program outputs the PSDK version in a simple format:
 * V3.14.0-beta.0-build.2296
 *
 *********************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <dji_platform.h>
#include <dji_logger.h>
#include <dji_core.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "utils/util_misc.h"
#include "osal/osal.h"
#include "osal/osal_fs.h"
#include "../hal/hal_uart.h"
#include "../hal/hal_network.h"
#include "../hal/hal_usb_bulk.h"
#include "dji_sdk_app_info.h"
#include "dji_sdk_config.h"

/* Private constants ---------------------------------------------------------*/
#define DJI_LOG_PATH                    "Logs/DJI"
#define DJI_LOG_INDEX_FILE_NAME         "Logs/index"
#define DJI_LOG_PATH_MAX_SIZE           (128)

/* Private variables ---------------------------------------------------------*/
static FILE *s_djiLogFile;
static FILE *s_djiLogFileCnt;
static bool s_versionFound = false;
static char s_versionString[128] = {0};

/* Private function prototypes -----------------------------------------------*/
static T_DjiReturnCode DjiUser_PrepareSystemEnvironment(void);
static T_DjiReturnCode DjiUser_CleanSystemEnvironment(void);
static T_DjiReturnCode DjiUser_FillInUserInfo(T_DjiUserInfo *userInfo);
static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_LocalWrite(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_LocalWriteFsInit(const char *path);
static void DjiUser_NormalExitHandler(int signalNum);

/* Exported functions definition ---------------------------------------------*/

/**
 * @brief Main entry point for PSDK version checker
 * 
 * @details Initializes PSDK and captures the version string from the log output.
 *          The version is printed during DjiCore_Init(), so we capture it from
 *          the console output handler.
 * 
 * @param argc Number of command line arguments (unused)
 * @param argv Array of command line argument strings (unused)
 * 
 * @return int Exit status:
 *         - 0: Success - version found and printed
 *         - 1: Error - could not determine version
 */
int main(int argc, char **argv)
{
    (void)(argc);
    (void)(argv);
    
    T_DjiReturnCode returnCode;
    T_DjiUserInfo userInfo;
    
    // Setup signal handlers
    signal(SIGTERM, DjiUser_NormalExitHandler);
    signal(SIGINT, DjiUser_NormalExitHandler);
    
    /*!< Step 1: Prepare system environment */
    returnCode = DjiUser_PrepareSystemEnvironment();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to prepare system environment (0x%08X)\n", returnCode);
        return 1;
    }
    
    /*!< Step 2: Fill application information */
    returnCode = DjiUser_FillInUserInfo(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to fill user info (0x%08X)\n", returnCode);
        DjiUser_CleanSystemEnvironment();
        return 1;
    }
    
    /*!< Step 3: Initialize the Payload SDK core */
    // The version will be printed during initialization
    // We capture it in DjiUser_PrintConsole
    returnCode = DjiCore_Init(&userInfo);
    
    // Clean up immediately after getting version
    DjiUser_CleanSystemEnvironment();
    
    if (s_versionFound) {
        printf("%s\n", s_versionString);
        return 0;
    } else {
        fprintf(stderr, "ERROR: Could not determine PSDK version\n");
        fprintf(stderr, "       PSDK initialization returned: 0x%08X\n", returnCode);
        return 1;
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Prepare system environment for PSDK
 * 
 * @details Sets up minimal handlers needed for PSDK initialization.
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
        .isSupportColor = false,  // Disable color for cleaner output
    };
    
    T_DjiHalUartHandler uartHandler = {
        .UartInit = HalUart_Init,
        .UartDeInit = HalUart_DeInit,
        .UartWriteData = HalUart_WriteData,
        .UartReadData = HalUart_ReadData,
        .UartGetStatus = HalUart_GetStatus,
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
    
    returnCode = DjiPlatform_RegOsalHandler(&osalHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    returnCode = DjiPlatform_RegHalUartHandler(&uartHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    if (DjiUser_LocalWriteFsInit(DJI_LOG_PATH) != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        // Non-fatal - we can still get version without log file
    }
    
    returnCode = DjiLogger_AddConsole(&printConsole);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    returnCode = DjiPlatform_RegFileSystemHandler(&fileSystemHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Clean up environment
 */
static T_DjiReturnCode DjiUser_CleanSystemEnvironment(void)
{
    T_DjiReturnCode returnCode;
    
    returnCode = DjiCore_DeInit();
    
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
 * @brief Print console output handler - captures version string
 * 
 * @details This function intercepts console output and looks for the
 *          "Payload SDK Version" string to extract the version.
 */
static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen)
{
    if (!data || dataLen == 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    // Convert to null-terminated string for processing
    char buffer[256];
    int len = (dataLen < sizeof(buffer) - 1) ? dataLen : sizeof(buffer) - 1;
    memcpy(buffer, data, len);
    buffer[len] = '\0';
    
    // Look for "Payload SDK Version" in the output
    if (strstr(buffer, "Payload SDK Version") != NULL && !s_versionFound) {
        // Extract version string (format: "Payload SDK Version : V3.14.0-beta.0-build.2296")
        const char *versionStart = strstr(buffer, "V");
        if (versionStart != NULL) {
            // Find the end of version string (space or end of line)
            const char *versionEnd = versionStart;
            while (*versionEnd != '\0' && *versionEnd != ' ' && *versionEnd != '\n' && *versionEnd != '\r') {
                versionEnd++;
            }
            
            int versionLen = versionEnd - versionStart;
            if (versionLen > 0 && versionLen < sizeof(s_versionString)) {
                memcpy(s_versionString, versionStart, versionLen);
                s_versionString[versionLen] = '\0';
                s_versionFound = true;
            }
        }
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/**
 * @brief Local write handler for file logging
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
 */
static void DjiUser_NormalExitHandler(int signalNum)
{
    (void)(signalNum);
    // Clean exit - version may have been found
}
