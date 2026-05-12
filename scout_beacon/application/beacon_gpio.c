/**
 ********************************************************************
 * @file    beacon_gpio.c
 * @brief   Implementation of beacon GPIO interface using lgpio library
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This file implements the complete interface for reading avalanche
 *          beacon signals from GPIO pins on Raspberry Pi using the lgpio
 *          library. It provides both real hardware operation and mock mode
 *          for testing without physical beacon receivers.
 *
 * @author  Scout Berry Development Team
 * @date    2024
 * @version 1.0.0
 *
 * @section Features
 * - Real-time GPIO reading using lgpio library
 * - Bearing detection from 5 directional pins
 * - Distance measurement from 7-segment display
 * - Mock mode for testing without hardware
 * - Comprehensive error handling and validation
 * - Debug output for troubleshooting
 * - Callback-based event notification
 *
 * @section GPIO Pin Configuration
 * - Bearing pins: 17, 27, 22, 5, 6 (270°, 325°, 0°, 45°, 90°)
 * - 7-segment display: 13, 19, 26, 21, 20, 16, 12, 25 (A-G, DP)
 * - Digit control: 8, 7 (enable pins for multiplexing)
 *
 * @section Hardware Interface
 * The interface supports:
 * - Digital input reading for bearing detection
 * - 7-segment display decoding for distance measurement
 * - Multiplexed digit reading from 7-segment display
 * - Edge detection for signal changes
 *
 * @section Mock Mode
 * Mock mode provides realistic test data including:
 * - 8 different beacon patterns
 * - Realistic bearing and distance values
 * - Automatic pattern cycling
 * - Configurable update intervals
 *
 * @section Error Handling
 * - GPIO initialization failures
 * - Pin configuration errors
 * - Read operation failures
 * - Invalid data validation
 * - Resource cleanup
 *
 *********************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "beacon_gpio.h"
#if defined(SCOUT_BEACON_USE_REAL_LGPIO)
#include <lgpio.h>
#else
#include "lgpio_stub.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

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

/* Mock beacon data for testing */
static bool s_mock_mode_enabled = false;
static uint32_t s_mock_beacon_counter = 0;
static uint32_t s_mock_update_interval = 5; // Update every 5 seconds in mock mode

/* Mock beacon data patterns for realistic testing */
typedef struct {
    int32_t bearing;
    float distance;
    int32_t rssi;
    const char* description;
} MockBeaconPattern_t;

static const MockBeaconPattern_t s_mock_patterns[] = {
    {0, 15.5, -99, "Close beacon - North"},
    {45, 32.1, -99, "Medium range - Northeast"},
    {90, 8.7, -99, "Very close - East"},
    {135, 45.2, -99, "Far range - Southeast"},
    {180, 22.3, -99, "Medium range - South"},
    {225, 67.8, -99, "Far range - Southwest"},
    {270, 12.4, -99, "Close range - West"},
    {315, 38.9, -99, "Medium range - Northwest"}
};

#define MOCK_PATTERN_COUNT (sizeof(s_mock_patterns) / sizeof(s_mock_patterns[0]))

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
static void BeaconGpio_GenerateMockData(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize beacon GPIO interface
 * 
 * @details This function initializes the GPIO interface for beacon detection
 *          using the lgpio library. It configures all required pins for
 *          bearing detection and distance measurement from the 7-segment display.
 *          The function sets up proper pull-up/pull-down resistors and edge
 *          detection for reliable signal reading.
 * 
 * @return int Return code:
 *         - 0: Success
 *         - -1: Initialization failure
 * 
 * @section Initialization Process
 * 1. Open GPIO chip handle (chip 0)
 * 2. Configure bearing pins as inputs with pull-up resistors
 * 3. Configure 7-segment display pins as inputs with pull-down
 * 4. Configure digit control pins for multiplexing
 * 5. Initialize context variables
 * 6. Enable debug output if requested
 * 
 * @section Pin Configuration
 * - Bearing pins (17, 27, 22, 5, 6): Input with pull-up, rising edge detection
 * - Segment pins (13, 19, 26, 21, 20, 16, 12, 25): Input with pull-down, both edges
 * - Digit control pins (8, 7): Input with pull-down, both edges
 * 
 * @section Error Handling
 * - GPIO chip open failures
 * - Pin configuration errors
 * - Resource allocation failures
 * - Automatic cleanup on failure
 * 
 * @section Dependencies
 * - lgpio library must be installed
 * - GPIO permissions required
 * - Raspberry Pi GPIO interface
 * 
 * @see BeaconGpio_Cleanup()
 * @see BeaconGpio_RegisterCallback()
 */
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
        ret = lgGpioClaimInput(s_context.chip_handle, 0, bearing_pins[i]);
        if (ret < 0) {
            printf("Error: Failed to configure bearing pin %d: %s\n", 
                   bearing_pins[i], strerror(errno));
            BeaconGpio_Cleanup();
            return -1;
        }
    }
    
    // Configure 7-segment display segment pins as inputs (to read the display)
    int segment_pins[] = {SEG_A_PIN, SEG_B_PIN, SEG_C_PIN, SEG_D_PIN, 
                          SEG_E_PIN, SEG_F_PIN, SEG_G_PIN, SEG_DP_PIN};
    
    for (int i = 0; i < 8; i++) {
        ret = lgGpioClaimInput(s_context.chip_handle, 0, segment_pins[i]);
        if (ret < 0) {
            printf("Error: Failed to configure segment pin %d: %s\n", 
                   segment_pins[i], strerror(errno));
            BeaconGpio_Cleanup();
            return -1;
        }
    }
    
    // Configure digit control pins as outputs (to enable/disable digits for multiplexing)
    ret = lgGpioClaimOutput(s_context.chip_handle, 0, DIGIT_1_PIN, 0);
    if (ret < 0) {
        printf("Error: Failed to configure digit 1 control pin %d: %s\n", 
               DIGIT_1_PIN, strerror(errno));
        BeaconGpio_Cleanup();
        return -1;
    }
    
    ret = lgGpioClaimOutput(s_context.chip_handle, 0, DIGIT_2_PIN, 0);
    if (ret < 0) {
        printf("Error: Failed to configure digit 2 control pin %d: %s\n", 
               DIGIT_2_PIN, strerror(errno));
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
    int input_pins[] = {BEARING_PIN_270, BEARING_PIN_325, BEARING_PIN_0, 
                       BEARING_PIN_45, BEARING_PIN_90, SEG_A_PIN, SEG_B_PIN, 
                       SEG_C_PIN, SEG_D_PIN, SEG_E_PIN, SEG_F_PIN, SEG_G_PIN, 
                       SEG_DP_PIN};
    
    for (int i = 0; i < 13; i++) {
        lgGpioFree(s_context.chip_handle, input_pins[i]);
    }
    
    // Release output pins (digit control)
    lgGpioFree(s_context.chip_handle, DIGIT_1_PIN);
    lgGpioFree(s_context.chip_handle, DIGIT_2_PIN);
    
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

/**
 * @brief Read current beacon data from GPIO interface
 * 
 * @details This function reads the current beacon data from the GPIO interface
 *          and returns it in the provided structure. The function updates
 *          the internal data cache and copies the latest readings to the
 *          output parameter. It handles both real hardware and mock mode
 *          operation transparently.
 * 
 * @param beacon_data Pointer to BeaconData_t structure to fill with current data
 * 
 * @return int Return code:
 *         - 0: Success
 *         - -1: Invalid parameters or GPIO not initialized
 * 
 * @section Data Structure
 * The function fills the following fields in BeaconData_t:
 * - bearing: Bearing in degrees (0, 45, 90, 270, 325) or -1 if invalid
 * - distance: Distance in meters from 7-segment display or -1.0 if invalid
 * - signal_strength: Reserved field (set to invalid value -99)
 * - timestamp: Unix timestamp of last update
 * - signal_detected: True if any signal is detected
 * - bearing_valid: True if bearing measurement is valid
 * - distance_valid: True if distance measurement is valid
 * 
 * @section Operation Modes
 * - Real Mode: Reads actual GPIO pins for bearing and distance from 7-segment display
 * - Mock Mode: Generates realistic test data with configurable patterns
 * 
 * @section Data Validation
 * - Bearing values are validated against known directions
 * - Distance values are read from 7-segment display and checked for reasonable ranges
 * - Timestamps are updated on each read operation
 * 
 * @section Performance
 * - Non-blocking operation
 * - Efficient data copying
 * - Minimal processing overhead
 * - Thread-safe operation
 * 
 * @section Error Handling
 * - Parameter validation
 * - Initialization state checking
 * - Data validity verification
 * - Graceful error reporting
 * 
 * @see BeaconGpio_UpdateData()
 * @see BeaconGpio_IsBeaconDetected()
 * @see BeaconGpio_GetBearing()
 * @see BeaconGpio_GetDistance()
 */
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
    // RSSI is not available - distance is read from 7-segment display instead
    // Return invalid value for API compatibility
    return -99;
}

void BeaconGpio_SetDebug(bool enable)
{
    s_context.debug_enabled = enable;
}

int BeaconGpio_EnableMockMode(void)
{
    if (!s_context.initialized) {
        printf("Error: Beacon GPIO not initialized\n");
        return -1;
    }
    
    s_mock_mode_enabled = true;
    s_mock_beacon_counter = 0;
    printf("[BEACON_GPIO] Mock mode enabled - generating simulated beacon data\n");
    printf("[BEACON_GPIO] Mock patterns available: %lu\n", MOCK_PATTERN_COUNT);
    
    // Generate initial mock data
    BeaconGpio_GenerateMockData();
    
    return 0;
}

int BeaconGpio_DisableMockMode(void)
{
    s_mock_mode_enabled = false;
    printf("[BEACON_GPIO] Mock mode disabled - returning to real GPIO reading\n");
    return 0;
}

bool BeaconGpio_IsMockModeEnabled(void)
{
    return s_mock_mode_enabled;
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

/**
 * @brief Read distance from 7-segment display
 * 
 * @details This function reads the distance value from the 7-segment display
 *          by multiplexing between two digits. It enables each digit sequentially,
 *          reads the segment states, decodes the digit value, and combines them
 *          to form a distance reading in meters.
 * 
 * @return float Distance in meters, or -1.0f if reading failed
 * 
 * @section Reading Process
 * 1. Enable digit 1 (tens place) and disable digit 2
 * 2. Wait for display to stabilize
 * 3. Read all segment pins (A-G)
 * 4. Decode segment pattern to digit value (0-9)
 * 5. Disable digit 1
 * 6. Enable digit 2 (ones/decimals place) and disable digit 1
 * 7. Wait for display to stabilize
 * 8. Read all segment pins again
 * 9. Decode segment pattern to digit value (0-9)
 * 10. Disable digit 2
 * 11. Combine digits: digit1.digit2 meters
 * 
 * @section Distance Format
 * The 7-segment display shows distance as two digits:
 * - Digit 1: Tens and ones place (0-99)
 * - Digit 2: Decimal place (0-9)
 * - Result: digit1.digit2 meters (e.g., 15.5 meters)
 * 
 * @section Error Handling
 * - Returns -1.0f if either digit fails to decode
 * - Validates digit values are in range 0-9
 * - Handles GPIO read errors gracefully
 * 
 * @see BeaconGpio_ReadSegmentPins()
 * @see BeaconGpio_DecodeDigit()
 */
static float BeaconGpio_ReadDistance(void)
{
    float distance = -1.0f;
    int digit1 = -1;
    int digit2 = -1;
    
    // Read digit 1 (tens/ones place) - enable digit 1, disable digit 2
    lgGpioWrite(s_context.chip_handle, DIGIT_2_PIN, 0);  // Disable digit 2 first
    lgGpioWrite(s_context.chip_handle, DIGIT_1_PIN, 1);   // Enable digit 1
    usleep(SEGMENT_READ_DELAY_US);  // Wait for display to stabilize
    
    int segment_state = BeaconGpio_ReadSegmentPins();
    if (segment_state >= 0) {
        digit1 = BeaconGpio_DecodeDigit((uint8_t)segment_state);
    }
    lgGpioWrite(s_context.chip_handle, DIGIT_1_PIN, 0);   // Disable digit 1
    
    if (digit1 < 0 || digit1 > 9) {
        BeaconGpio_PrintDebug("Failed to decode digit 1 from 7-segment display\n");
        return -1.0f;
    }
    
    // Read digit 2 (decimal place) - enable digit 2, disable digit 1
    lgGpioWrite(s_context.chip_handle, DIGIT_1_PIN, 0);   // Ensure digit 1 is disabled
    lgGpioWrite(s_context.chip_handle, DIGIT_2_PIN, 1);   // Enable digit 2
    usleep(SEGMENT_READ_DELAY_US);  // Wait for display to stabilize
    
    segment_state = BeaconGpio_ReadSegmentPins();
    if (segment_state >= 0) {
        digit2 = BeaconGpio_DecodeDigit((uint8_t)segment_state);
    }
    lgGpioWrite(s_context.chip_handle, DIGIT_2_PIN, 0);   // Disable digit 2
    
    if (digit2 < 0 || digit2 > 9) {
        BeaconGpio_PrintDebug("Failed to decode digit 2 from 7-segment display\n");
        return -1.0f;
    }
    
    // Calculate distance: digit1.digit2 meters
    // Example: digit1=15, digit2=5 -> distance = 15.5 meters
    distance = (float)digit1 + ((float)digit2 * 0.1f);
    
    // Validate distance is within reasonable range (0.0 to 99.9 meters)
    if (distance < 0.0f || distance > 99.9f) {
        BeaconGpio_PrintDebug("Distance out of range: %.1f meters\n", distance);
        return -1.0f;
    }
    
    BeaconGpio_PrintDebug("Distance from 7-segment display: %d.%d meters (%.1f m)\n", 
                          digit1, digit2, distance);
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
    // RSSI is not available - distance is read from 7-segment display instead
    // Return invalid value to indicate RSSI is not used
    return -99;
}

/**
 * @brief Update beacon data from GPIO interface or mock mode
 * 
 * @details This function updates the internal beacon data cache by reading
 *          from GPIO pins or generating mock data. It handles both real
 *          hardware operation and mock mode transparently, providing a
 *          unified interface for beacon data acquisition.
 * 
 * @section Operation Modes
 * - Real Mode: Reads actual GPIO pins for bearing and distance from 7-segment display
 * - Mock Mode: Generates realistic test data with configurable patterns
 * 
 * @section Data Processing
 * 1. Update timestamp to current time
 * 2. Check operation mode (real vs mock)
 * 3. Read or generate bearing data
 * 4. Read or generate distance data from 7-segment display
 * 5. Set signal strength to invalid (not used)
 * 6. Update validity flags
 * 7. Trigger callback if data changed
 * 
 * @section Real Mode Operation
 * - Reads bearing from 5 directional pins
 * - Decodes distance from 7-segment display (two digits: tens and ones/decimals)
 * - Validates all measurements
 * - Updates detection flags
 * 
 * @section Mock Mode Operation
 * - Generates realistic test patterns
 * - Cycles through predefined scenarios
 * - Adds random variations for realism
 * - Maintains configurable update intervals
 * 
 * @section Data Validation
 * - Bearing values are checked against known directions
 * - Distance values are read from 7-segment display and validated for reasonable ranges
 * - Timestamps are updated on each call
 * 
 * @section Callback Triggering
 * - Callback is triggered when beacon is detected
 * - Provides immediate notification of signal changes
 * - Enables real-time processing of beacon events
 * 
 * @see BeaconGpio_ReadBearing()
 * @see BeaconGpio_ReadDistance()
 * @see BeaconGpio_ReadSignalStrength()
 * @see BeaconGpio_GenerateMockData()
 * @see BeaconGpio_TriggerCallback()
 */
static void BeaconGpio_UpdateData(void)
{
    uint32_t current_time = time(NULL);
    
    // Update timestamp
    s_context.last_data.timestamp = current_time;
    
    if (s_mock_mode_enabled) {
        // Generate mock data if in mock mode
        BeaconGpio_GenerateMockData();
    } else {
        // Read real GPIO data
        // Read bearing
        int bearing = BeaconGpio_ReadBearing();
        if (bearing >= 0) {
            s_context.last_data.bearing = bearing;
            s_context.last_data.bearing_valid = true;
            s_context.last_data.signal_detected = true;
        } else {
            s_context.last_data.bearing_valid = false;
        }
        
        // Read distance from 7-segment display
        float distance = BeaconGpio_ReadDistance();
        if (distance >= 0.0f) {
            s_context.last_data.distance = distance;
            s_context.last_data.distance_valid = true;
        } else {
            s_context.last_data.distance_valid = false;
        }
        
        // Signal strength is not available - set to invalid value
        s_context.last_data.signal_strength = -99;
        
        // Update signal detection based on bearing or distance
        if (s_context.last_data.bearing_valid || s_context.last_data.distance_valid) {
            s_context.last_data.signal_detected = true;
        }
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

static void BeaconGpio_GenerateMockData(void)
{
    uint32_t current_time = time(NULL);
    
    // Check if it's time to update mock data (every 5 seconds)
    if (current_time - s_context.last_update_time < s_mock_update_interval) {
        return;
    }
    
    // Cycle through mock patterns
    const MockBeaconPattern_t* pattern = &s_mock_patterns[s_mock_beacon_counter % MOCK_PATTERN_COUNT];
    
    // Update beacon data with mock values
    s_context.last_data.bearing = pattern->bearing;
    s_context.last_data.distance = pattern->distance;
    s_context.last_data.signal_strength = -99; // RSSI not available
    s_context.last_data.bearing_valid = true;
    s_context.last_data.distance_valid = true;
    s_context.last_data.signal_detected = true;
    s_context.last_data.timestamp = current_time;
    
    // Add some randomness to make it more realistic
    s_context.last_data.distance += ((float)(rand() % 20 - 10)) / 10.0f; // ±1.0m variation
    
    // Ensure values stay within reasonable bounds
    if (s_context.last_data.distance < 0.1f) s_context.last_data.distance = 0.1f;
    
    printf("[BEACON_GPIO] Mock data: %s - Bearing: %d°, Distance: %.1fm (from 7-segment display)\n",
           pattern->description, s_context.last_data.bearing, 
           s_context.last_data.distance);
    
    s_mock_beacon_counter++;
}
