/**
 ********************************************************************
 * @file    beacon_gpio.h
 * @brief   Header file for beacon GPIO interface using lgpio library
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * @details This header file defines the complete interface for reading
 *          avalanche beacon signals from GPIO pins on Raspberry Pi using
 *          the lgpio library. It provides both real hardware operation
 *          and mock mode for testing without physical beacon receivers.
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

#ifndef BEACON_GPIO_H
#define BEACON_GPIO_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Exported constants --------------------------------------------------------*/
// GPIO Pin definitions (BCM numbering)
#define BEARING_PIN_270    17  // Bearing 270 degrees
#define BEARING_PIN_325    27  // Bearing 325 degrees  
#define BEARING_PIN_0      22  // Bearing 0 degrees
#define BEARING_PIN_45     5   // Bearing 45 degrees
#define BEARING_PIN_90     6   // Bearing 90 degrees

// 7-segment display pins for distance measurement
#define SEG_A_PIN          13  // Segment A
#define SEG_B_PIN          19  // Segment B
#define SEG_C_PIN          26  // Segment C
#define SEG_D_PIN          21  // Segment D
#define SEG_E_PIN          20  // Segment E
#define SEG_F_PIN          16  // Segment F
#define SEG_G_PIN          12  // Segment G
#define SEG_DP_PIN         25  // Decimal Point
#define DIGIT_1_PIN        8   // Digit 1 Enable
#define DIGIT_2_PIN        7   // Digit 2 Enable

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Beacon data structure containing all beacon information
 * 
 * @details This structure contains all the data read from the avalanche beacon
 *          receiver, including bearing, distance, signal strength, and validity
 *          flags. It is used throughout the application for data transmission
 *          and processing.
 * 
 * @section Data Fields
 * - bearing: Bearing in degrees (0, 45, 90, 270, 325) or -1 if invalid
 * - distance: Distance in meters from 7-segment display or -1.0 if invalid
 * - signal_strength: Reserved field (not used, set to invalid value)
 * - timestamp: Unix timestamp of last update
 * - signal_detected: True if any beacon signal is detected
 * - bearing_valid: True if bearing measurement is valid
 * - distance_valid: True if distance measurement is valid
 * 
 * @section Data Validation
 * - Bearing values are validated against known directions
 * - Distance values are read from 7-segment display and checked for reasonable ranges (0.1m to 99.9m)
 * - Timestamps are updated on each read operation
 * 
 * @section Usage
 * This structure is used for:
 * - GPIO data reading and caching
 * - PSDK data transmission
 * - Application state management
 * - Debug output and logging
 * 
 * @see BeaconGpio_ReadBeaconData()
 * @see ScoutBeacon_SendBeaconData()
 * @see ScoutBeacon_LogBeaconData()
 */
typedef struct {
    uint8_t bearing;           // Bearing in degrees (0, 45, 90, 270, 325)
    float distance;            // Distance in meters from 7-segment display
    int8_t signal_strength;    // Reserved field (not used, set to -99)
    uint32_t timestamp;       // Unix timestamp
    bool signal_detected;      // Beacon detection flag
    bool bearing_valid;        // Bearing measurement validity
    bool distance_valid;       // Distance measurement validity
} BeaconData_t;

/**
 * @brief GPIO callback function type for beacon detection events
 * 
 * @details This function pointer type defines the callback interface for
 *          beacon detection events. The callback is triggered when a beacon
 *          signal is detected, providing immediate notification and processing
 *          of beacon data.
 * 
 * @param beacon_data Pointer to beacon data structure containing:
 *                   - bearing: Bearing in degrees (0, 45, 90, 270, 325)
 *                   - distance: Distance in meters (from 7-segment display)
 *                   - signal_strength: Reserved field (not used)
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
 * - Transmission errors are handled by calling function
 * 
 * @section Performance
 * - Minimal processing overhead
 * - Immediate data transmission
 * - Non-blocking operation
 * 
 * @see BeaconGpio_RegisterCallback()
 * @see ScoutBeacon_BeaconCallback()
 */
typedef void (*BeaconCallback_t)(BeaconData_t* beacon_data);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize beacon GPIO interface
 * @return 0 on success, -1 on failure
 */
int BeaconGpio_Init(void);

/**
 * @brief Cleanup beacon GPIO interface
 * @return 0 on success, -1 on failure
 */
int BeaconGpio_Cleanup(void);

/**
 * @brief Register callback for beacon detection events
 * @param callback Function to call when beacon is detected
 * @return 0 on success, -1 on failure
 */
int BeaconGpio_RegisterCallback(BeaconCallback_t callback);

/**
 * @brief Read current beacon data
 * @param beacon_data Pointer to structure to fill with current data
 * @return 0 on success, -1 on failure
 */
int BeaconGpio_ReadBeaconData(BeaconData_t* beacon_data);

/**
 * @brief Check if beacon signal is currently detected
 * @return true if beacon detected, false otherwise
 */
bool BeaconGpio_IsBeaconDetected(void);

/**
 * @brief Get current bearing from GPIO pins
 * @return Bearing in degrees (0, 45, 90, 270, 325) or -1 if no valid bearing
 */
int BeaconGpio_GetBearing(void);

/**
 * @brief Get current distance from 7-segment display
 * @return Distance in meters, or -1.0 if invalid
 */
float BeaconGpio_GetDistance(void);

/**
 * @brief Get current signal strength (deprecated - not used)
 * @return Always returns -99 (invalid) as RSSI is not available
 * @note This function is kept for API compatibility but always returns invalid value
 *       Distance is read from 7-segment display instead
 */
int8_t BeaconGpio_GetSignalStrength(void);

/**
 * @brief Enable or disable debug output
 * @param enable true to enable debug output, false to disable
 */
void BeaconGpio_SetDebug(bool enable);

/* Mock mode functions for testing */
/**
 * @brief Enable mock mode for testing without physical hardware
 * @return 0 on success, -1 on failure
 */
int BeaconGpio_EnableMockMode(void);

/**
 * @brief Disable mock mode and return to real GPIO reading
 * @return 0 on success, -1 on failure
 */
int BeaconGpio_DisableMockMode(void);

/**
 * @brief Check if mock mode is currently enabled
 * @return true if mock mode enabled, false otherwise
 */
bool BeaconGpio_IsMockModeEnabled(void);

#ifdef __cplusplus
}
#endif

#endif // BEACON_GPIO_H
/************************ (C) COPYRIGHT Scout Berry *******END OF FILE******/
