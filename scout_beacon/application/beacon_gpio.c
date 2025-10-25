/**
 ********************************************************************
 * @file    beacon_gpio.c
 * @brief   Implementation of beacon GPIO interface using lgpio library
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * This file implements the interface for reading avalanche beacon signals
 * from GPIO pins on Raspberry Pi using the lgpio library.
 *
 *********************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "beacon_gpio.h"
#include <lgpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Private constants ---------------------------------------------------------*/
#define GPIO_CHIP_HANDLE    0
#define MAX_RETRIES         3
#define DEBOUNCE_TIME_MS    50
#define SEGMENT_READ_DELAY_US 1000

/* Private types -------------------------------------------------------------*/
typedef struct {
    int chip_handle;
    bool initialized;
    bool debug_enabled;
    BeaconCallback_t callback;
    BeaconData_t last_data;
    uint32_t last_update_time;
} BeaconGpioContext_t;

/* Private variables ---------------------------------------------------------*/
static BeaconGpioContext_t s_context = {0};

/* 7-segment digit mapping (same as Python implementation) */
static const uint8_t s_digit_map[10] = {
    0b00111111,  // 0: segments A,B,C,D,E,F
    0b00000110,  // 1: segments B,C
    0b01011011,  // 2: segments A,B,D,E,G
    0b01001111,  // 3: segments A,B,C,D,G
    0b01100110,  // 4: segments B,C,F,G
    0b01101101,  // 5: segments A,C,D,F,G
    0b01111101,  // 6: segments A,C,D,E,F,G
    0b00000111,  // 7: segments A,B,C
    0b01111111,  // 8: all segments
    0b01101111   // 9: segments A,B,C,D,F,G
};

/* Private function prototypes -----------------------------------------------*/
static int BeaconGpio_ReadSegmentPins(void);
static int BeaconGpio_DecodeDigit(uint8_t segment_state);
static float BeaconGpio_ReadDistance(void);
static int BeaconGpio_ReadBearing(void);
static int8_t BeaconGpio_ReadSignalStrength(void);
static void BeaconGpio_UpdateData(void);
static void BeaconGpio_TriggerCallback(void);
static void BeaconGpio_PrintDebug(const char* format, ...);

/* Exported functions --------------------------------------------------------*/

int BeaconGpio_Init(void)
{
    int ret;
    
    if (s_context.initialized) {
        BeaconGpio_PrintDebug("Beacon GPIO already initialized\n");
        return 0;
    }
    
    // Initialize lgpio chip handle
    s_context.chip_handle = lgGpiochipOpen(GPIO_CHIP_HANDLE);
    if (s_context.chip_handle < 0) {
        printf("Error: Failed to open GPIO chip: %s\n", strerror(errno));
        return -1;
    }
    
    // Configure bearing pins as inputs with pull-up
    int bearing_pins[] = {BEARING_PIN_270, BEARING_PIN_325, BEARING_PIN_0, 
                         BEARING_PIN_45, BEARING_PIN_90};
    
    for (int i = 0; i < 5; i++) {
        ret = lgGpioClaimInput(s_context.chip_handle, 0, bearing_pins[i], 
                              LG_PULL_UP, LG_RISING_EDGE);
        if (ret < 0) {
            printf("Error: Failed to configure bearing pin %d: %s\n", 
                   bearing_pins[i], strerror(errno));
            BeaconGpio_Cleanup();
            return -1;
        }
    }
    
    // Configure 7-segment display pins as inputs
    int segment_pins[] = {SEG_A_PIN, SEG_B_PIN, SEG_C_PIN, SEG_D_PIN, 
                          SEG_E_PIN, SEG_F_PIN, SEG_G_PIN, SEG_DP_PIN,
                          DIGIT_1_PIN, DIGIT_2_PIN};
    
    for (int i = 0; i < 10; i++) {
        ret = lgGpioClaimInput(s_context.chip_handle, 0, segment_pins[i], 
                              LG_PULL_DOWN, LG_BOTH_EDGES);
        if (ret < 0) {
            printf("Error: Failed to configure segment pin %d: %s\n", 
                   segment_pins[i], strerror(errno));
            BeaconGpio_Cleanup();
            return -1;
        }
    }
    
    // Configure RSSI pin as input
    ret = lgGpioClaimInput(s_context.chip_handle, 0, RSSI_PIN, 
                          LG_PULL_DOWN, LG_BOTH_EDGES);
    if (ret < 0) {
        printf("Error: Failed to configure RSSI pin %d: %s\n", 
               RSSI_PIN, strerror(errno));
        BeaconGpio_Cleanup();
        return -1;
    }
    
    s_context.initialized = true;
    s_context.debug_enabled = false;
    s_context.last_update_time = 0;
    
    BeaconGpio_PrintDebug("Beacon GPIO initialized successfully\n");
    return 0;
}

int BeaconGpio_Cleanup(void)
{
    if (!s_context.initialized) {
        return 0;
    }
    
    // Release all GPIO pins
    int all_pins[] = {BEARING_PIN_270, BEARING_PIN_325, BEARING_PIN_0, 
                     BEARING_PIN_45, BEARING_PIN_90, SEG_A_PIN, SEG_B_PIN, 
                     SEG_C_PIN, SEG_D_PIN, SEG_E_PIN, SEG_F_PIN, SEG_G_PIN, 
                     SEG_DP_PIN, DIGIT_1_PIN, DIGIT_2_PIN, RSSI_PIN};
    
    for (int i = 0; i < 16; i++) {
        lgGpioFree(s_context.chip_handle, all_pins[i]);
    }
    
    // Close GPIO chip
    if (s_context.chip_handle >= 0) {
        lgGpiochipClose(s_context.chip_handle);
        s_context.chip_handle = -1;
    }
    
    s_context.initialized = false;
    BeaconGpio_PrintDebug("Beacon GPIO cleaned up\n");
    return 0;
}

int BeaconGpio_RegisterCallback(BeaconCallback_t callback)
{
    if (!s_context.initialized) {
        printf("Error: Beacon GPIO not initialized\n");
        return -1;
    }
    
    s_context.callback = callback;
    BeaconGpio_PrintDebug("Beacon callback registered\n");
    return 0;
}

int BeaconGpio_ReadBeaconData(BeaconData_t* beacon_data)
{
    if (!s_context.initialized || !beacon_data) {
        printf("Error: Invalid parameters or GPIO not initialized\n");
        return -1;
    }
    
    // Update beacon data
    BeaconGpio_UpdateData();
    
    // Copy current data
    *beacon_data = s_context.last_data;
    
    return 0;
}

bool BeaconGpio_IsBeaconDetected(void)
{
    if (!s_context.initialized) {
        return false;
    }
    
    BeaconGpio_UpdateData();
    return s_context.last_data.signal_detected;
}

int BeaconGpio_GetBearing(void)
{
    if (!s_context.initialized) {
        return -1;
    }
    
    BeaconGpio_UpdateData();
    return s_context.last_data.bearing_valid ? s_context.last_data.bearing : -1;
}

float BeaconGpio_GetDistance(void)
{
    if (!s_context.initialized) {
        return -1.0f;
    }
    
    BeaconGpio_UpdateData();
    return s_context.last_data.distance_valid ? s_context.last_data.distance : -1.0f;
}

int8_t BeaconGpio_GetSignalStrength(void)
{
    if (!s_context.initialized) {
        return -999;
    }
    
    BeaconGpio_UpdateData();
    return s_context.last_data.signal_strength;
}

void BeaconGpio_SetDebug(bool enable)
{
    s_context.debug_enabled = enable;
}

/* Private functions ---------------------------------------------------------*/

static void BeaconGpio_PrintDebug(const char* format, ...)
{
    if (!s_context.debug_enabled) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    printf("[BEACON_GPIO] ");
    vprintf(format, args);
    va_end(args);
}

static int BeaconGpio_ReadSegmentPins(void)
{
    int segment_state = 0;
    int segment_pins[] = {SEG_A_PIN, SEG_B_PIN, SEG_C_PIN, SEG_D_PIN, 
                          SEG_E_PIN, SEG_F_PIN, SEG_G_PIN};
    
    for (int i = 0; i < 7; i++) {
        int pin_state = lgGpioRead(s_context.chip_handle, segment_pins[i]);
        if (pin_state < 0) {
            BeaconGpio_PrintDebug("Error reading segment pin %d\n", segment_pins[i]);
            return -1;
        }
        segment_state |= (pin_state << i);
    }
    
    return segment_state;
}

static int BeaconGpio_DecodeDigit(uint8_t segment_state)
{
    for (int digit = 0; digit < 10; digit++) {
        if (s_digit_map[digit] == segment_state) {
            return digit;
        }
    }
    return -1; // Invalid digit
}

static float BeaconGpio_ReadDistance(void)
{
    float distance = -1.0f;
    
    // Read digit 1
    lgGpioWrite(s_context.chip_handle, DIGIT_1_PIN, 1);
    usleep(SEGMENT_READ_DELAY_US);
    int digit1 = BeaconGpio_DecodeDigit(BeaconGpio_ReadSegmentPins());
    lgGpioWrite(s_context.chip_handle, DIGIT_1_PIN, 0);
    
    if (digit1 < 0) {
        return -1.0f;
    }
    
    // Read digit 2
    lgGpioWrite(s_context.chip_handle, DIGIT_2_PIN, 1);
    usleep(SEGMENT_READ_DELAY_US);
    int digit2 = BeaconGpio_DecodeDigit(BeaconGpio_ReadSegmentPins());
    lgGpioWrite(s_context.chip_handle, DIGIT_2_PIN, 0);
    
    if (digit2 < 0) {
        return -1.0f;
    }
    
    // Calculate distance: digit1.digit2 meters
    distance = digit1 + (digit2 * 0.1f);
    
    BeaconGpio_PrintDebug("Distance: %d.%d meters\n", digit1, digit2);
    return distance;
}

static int BeaconGpio_ReadBearing(void)
{
    int bearing_pins[] = {BEARING_PIN_270, BEARING_PIN_325, BEARING_PIN_0, 
                         BEARING_PIN_45, BEARING_PIN_90};
    int bearings[] = {270, 325, 0, 45, 90};
    
    for (int i = 0; i < 5; i++) {
        int pin_state = lgGpioRead(s_context.chip_handle, bearing_pins[i]);
        if (pin_state < 0) {
            BeaconGpio_PrintDebug("Error reading bearing pin %d\n", bearing_pins[i]);
            return -1;
        }
        if (pin_state == 1) {
            BeaconGpio_PrintDebug("Bearing detected: %d degrees\n", bearings[i]);
            return bearings[i];
        }
    }
    
    return -1; // No bearing detected
}

static int8_t BeaconGpio_ReadSignalStrength(void)
{
    // For now, return a simulated RSSI value
    // In a real implementation, this would read from an ADC
    // or analog-to-digital converter connected to the RSSI pin
    return -67; // Simulated -67 dBm signal strength
}

static void BeaconGpio_UpdateData(void)
{
    uint32_t current_time = time(NULL);
    
    // Update timestamp
    s_context.last_data.timestamp = current_time;
    
    // Read bearing
    int bearing = BeaconGpio_ReadBearing();
    if (bearing >= 0) {
        s_context.last_data.bearing = bearing;
        s_context.last_data.bearing_valid = true;
        s_context.last_data.signal_detected = true;
    } else {
        s_context.last_data.bearing_valid = false;
    }
    
    // Read distance
    float distance = BeaconGpio_ReadDistance();
    if (distance >= 0.0f) {
        s_context.last_data.distance = distance;
        s_context.last_data.distance_valid = true;
    } else {
        s_context.last_data.distance_valid = false;
    }
    
    // Read signal strength
    s_context.last_data.signal_strength = BeaconGpio_ReadSignalStrength();
    
    // Update signal detection based on bearing or distance
    if (s_context.last_data.bearing_valid || s_context.last_data.distance_valid) {
        s_context.last_data.signal_detected = true;
    }
    
    // Trigger callback if data changed and callback is registered
    if (s_context.callback && s_context.last_data.signal_detected) {
        BeaconGpio_TriggerCallback();
    }
    
    s_context.last_update_time = current_time;
}

static void BeaconGpio_TriggerCallback(void)
{
    if (s_context.callback) {
        BeaconGpio_PrintDebug("Triggering beacon callback\n");
        s_context.callback(&s_context.last_data);
    }
}
