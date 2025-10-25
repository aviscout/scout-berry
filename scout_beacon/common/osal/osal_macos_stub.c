/**
 ********************************************************************
 * @file    osal_macos_stub.c
 * @brief   macOS-compatible stub implementation of OSAL functions
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This file provides macOS-compatible stub implementations
 *          of OSAL functions for development and testing on macOS.
 *          The actual OSAL functions are designed for Linux systems.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 *********************************************************************
 */

#include "osal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

/* Task operations */
T_DjiReturnCode Osal_TaskCreate(const char *name, void *(*taskFunc)(void *),
                                uint32_t stackSize, void *arg, T_DjiTaskHandle *task)
{
    (void)(name);
    (void)(stackSize);
    
    if (!taskFunc || !task) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    int result = pthread_create((pthread_t*)task, NULL, taskFunc, arg);
    if (result != 0) {
        printf("[OSAL_STUB] Task creation failed: %d\n", result);
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TaskDestroy(T_DjiTaskHandle task)
{
    if (!task) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    int result = pthread_cancel(*(pthread_t*)task);
    if (result != 0) {
        printf("[OSAL_STUB] Task destroy failed: %d\n", result);
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TaskSleepMs(uint32_t timeMs)
{
    usleep(timeMs * 1000);
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Mutex operations */
T_DjiReturnCode Osal_MutexCreate(T_DjiMutexHandle *mutex)
{
    if (!mutex) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    pthread_mutex_t *pMutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (!pMutex) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    int result = pthread_mutex_init(pMutex, NULL);
    if (result != 0) {
        free(pMutex);
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    *mutex = (T_DjiMutexHandle)pMutex;
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_MutexDestroy(T_DjiMutexHandle mutex)
{
    if (!mutex) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    pthread_mutex_t *pMutex = (pthread_mutex_t*)mutex;
    int result = pthread_mutex_destroy(pMutex);
    free(pMutex);
    
    if (result != 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_MutexLock(T_DjiMutexHandle mutex)
{
    if (!mutex) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    int result = pthread_mutex_lock((pthread_mutex_t*)mutex);
    if (result != 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_MutexUnlock(T_DjiMutexHandle mutex)
{
    if (!mutex) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    int result = pthread_mutex_unlock((pthread_mutex_t*)mutex);
    if (result != 0) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Semaphore operations (using condition variables as semaphores) */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint32_t count;
} semaphore_t;

T_DjiReturnCode Osal_SemaphoreCreate(uint32_t initValue, T_DjiSemaHandle *semaphore)
{
    if (!semaphore) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    semaphore_t *sem = (semaphore_t*)malloc(sizeof(semaphore_t));
    if (!sem) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_SYSTEM_ERROR;
    }
    
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->cond, NULL);
    sem->count = initValue;
    
    *semaphore = (T_DjiSemaHandle)sem;
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_SemaphoreDestroy(T_DjiSemaHandle semaphore)
{
    if (!semaphore) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    semaphore_t *sem = (semaphore_t*)semaphore;
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
    free(sem);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_SemaphoreWait(T_DjiSemaHandle semaphore)
{
    if (!semaphore) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    semaphore_t *sem = (semaphore_t*)semaphore;
    pthread_mutex_lock(&sem->mutex);
    
    while (sem->count == 0) {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    
    sem->count--;
    pthread_mutex_unlock(&sem->mutex);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_SemaphoreTimedWait(T_DjiSemaHandle semaphore, uint32_t waitTime)
{
    if (!semaphore) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    semaphore_t *sem = (semaphore_t*)semaphore;
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += waitTime / 1000;
    timeout.tv_nsec += (waitTime % 1000) * 1000000;
    
    pthread_mutex_lock(&sem->mutex);
    
    int result = 0;
    while (sem->count == 0 && result == 0) {
        result = pthread_cond_timedwait(&sem->cond, &sem->mutex, &timeout);
    }
    
    if (result == 0) {
        sem->count--;
    }
    
    pthread_mutex_unlock(&sem->mutex);
    
    return (result == 0) ? DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS : DJI_ERROR_SYSTEM_MODULE_CODE_TIMEOUT;
}

T_DjiReturnCode Osal_SemaphorePost(T_DjiSemaHandle semaphore)
{
    if (!semaphore) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    semaphore_t *sem = (semaphore_t*)semaphore;
    pthread_mutex_lock(&sem->mutex);
    sem->count++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Time operations */
T_DjiReturnCode Osal_GetTimeMs(uint32_t *ms)
{
    if (!ms) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *ms = (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_GetTimeUs(uint64_t *us)
{
    if (!us) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *us = (uint64_t)(tv.tv_sec * 1000000 + tv.tv_usec);
    
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_GetRandomNum(uint16_t *randomNum)
{
    if (!randomNum) {
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }
    
    *randomNum = (uint16_t)(rand() & 0xFFFF);
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Memory operations */
void *Osal_Malloc(uint32_t size)
{
    return malloc(size);
}

void Osal_Free(void *ptr)
{
    free(ptr);
}

/* File System Functions */

T_DjiReturnCode Osal_FileOpen(const char *fileName, const char *fileMode, T_DjiFileHandle *fileHandle)
{
    (void)(fileName);
    (void)(fileMode);
    (void)(fileHandle);
    printf("[OSAL_STUB] Osal_FileOpen() - File opened (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_FileClose(T_DjiFileHandle fileHandle)
{
    (void)(fileHandle);
    printf("[OSAL_STUB] Osal_FileClose() - File closed (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_FileRead(T_DjiFileHandle fileHandle, uint8_t *buf, uint32_t len, uint32_t *realLen)
{
    (void)(fileHandle);
    (void)(buf);
    (void)(len);
    if (realLen) *realLen = 0;
    printf("[OSAL_STUB] Osal_FileRead() - File read (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_FileWrite(T_DjiFileHandle fileHandle, const uint8_t *buf, uint32_t len, uint32_t *realLen)
{
    (void)(fileHandle);
    (void)(buf);
    (void)(len);
    if (realLen) *realLen = len;
    printf("[OSAL_STUB] Osal_FileWrite() - File written (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_FileSeek(T_DjiFileHandle fileHandle, uint32_t offset)
{
    (void)(fileHandle);
    (void)(offset);
    printf("[OSAL_STUB] Osal_FileSeek() - File seek (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_FileSync(T_DjiFileHandle fileHandle)
{
    (void)(fileHandle);
    printf("[OSAL_STUB] Osal_FileSync() - File sync (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_Stat(const char *filePath, T_DjiFileInfo *fileStat)
{
    (void)(filePath);
    (void)(fileStat);
    printf("[OSAL_STUB] Osal_Stat() - File stat (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_Mkdir(const char *dirPath)
{
    (void)(dirPath);
    printf("[OSAL_STUB] Osal_Mkdir() - Directory created (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_Rename(const char *oldPath, const char *newPath)
{
    (void)(oldPath);
    (void)(newPath);
    printf("[OSAL_STUB] Osal_Rename() - File renamed (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_Unlink(const char *filePath)
{
    (void)(filePath);
    printf("[OSAL_STUB] Osal_Unlink() - File unlinked (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_DirOpen(const char *dirPath, T_DjiDirHandle *dirHandle)
{
    (void)(dirPath);
    (void)(dirHandle);
    printf("[OSAL_STUB] Osal_DirOpen() - Directory opened (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_DirClose(T_DjiDirHandle dirHandle)
{
    (void)(dirHandle);
    printf("[OSAL_STUB] Osal_DirClose() - Directory closed (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_DirRead(T_DjiDirHandle dirHandle, T_DjiFileInfo *fileStat)
{
    (void)(dirHandle);
    (void)(fileStat);
    printf("[OSAL_STUB] Osal_DirRead() - Directory read (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

/* Socket Functions */

T_DjiReturnCode Osal_Socket(E_DjiSocketMode socketType, T_DjiSocketHandle *socketHandle)
{
    (void)(socketType);
    (void)(socketHandle);
    printf("[OSAL_STUB] Osal_Socket() - Socket created (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_Close(T_DjiSocketHandle socketHandle)
{
    (void)(socketHandle);
    printf("[OSAL_STUB] Osal_Close() - Socket closed (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_Bind(T_DjiSocketHandle socketHandle, const char *ipAddr, uint32_t port)
{
    (void)(socketHandle);
    (void)(ipAddr);
    (void)(port);
    printf("[OSAL_STUB] Osal_Bind() - Socket bound (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TcpListen(T_DjiSocketHandle socketHandle, uint32_t backlog)
{
    (void)(socketHandle);
    (void)(backlog);
    printf("[OSAL_STUB] Osal_TcpListen() - TCP listen (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TcpAccept(T_DjiSocketHandle socketHandle, T_DjiSocketHandle *clientSocketHandle, char *clientIpAddr, uint32_t *clientPort)
{
    (void)(socketHandle);
    (void)(clientSocketHandle);
    (void)(clientIpAddr);
    (void)(clientPort);
    printf("[OSAL_STUB] Osal_TcpAccept() - TCP accept (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TcpConnect(T_DjiSocketHandle socketHandle, const char *ipAddr, uint32_t port)
{
    (void)(socketHandle);
    (void)(ipAddr);
    (void)(port);
    printf("[OSAL_STUB] Osal_TcpConnect() - TCP connect (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TcpSendData(T_DjiSocketHandle socketHandle, const uint8_t *data, uint32_t dataLen, uint32_t *realLen)
{
    (void)(socketHandle);
    (void)(data);
    (void)(dataLen);
    if (realLen) *realLen = dataLen;
    printf("[OSAL_STUB] Osal_TcpSendData() - TCP send (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_TcpRecvData(T_DjiSocketHandle socketHandle, uint8_t *data, uint32_t dataLen, uint32_t *realLen)
{
    (void)(socketHandle);
    (void)(data);
    (void)(dataLen);
    if (realLen) *realLen = 0;
    printf("[OSAL_STUB] Osal_TcpRecvData() - TCP receive (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_UdpSendData(T_DjiSocketHandle socketHandle, const char *ipAddr, uint32_t port, const uint8_t *data, uint32_t dataLen, uint32_t *realLen)
{
    (void)(socketHandle);
    (void)(ipAddr);
    (void)(port);
    (void)(data);
    (void)(dataLen);
    if (realLen) *realLen = dataLen;
    printf("[OSAL_STUB] Osal_UdpSendData() - UDP send (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

T_DjiReturnCode Osal_UdpRecvData(T_DjiSocketHandle socketHandle, char *ipAddr, uint32_t *port, uint8_t *data, uint32_t dataLen, uint32_t *realLen)
{
    (void)(socketHandle);
    (void)(ipAddr);
    (void)(port);
    (void)(data);
    (void)(dataLen);
    if (realLen) *realLen = 0;
    printf("[OSAL_STUB] Osal_UdpRecvData() - UDP receive (stub)\n");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}
