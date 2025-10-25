/**
 ********************************************************************
 * @file    beacon_gpio.h
 * @brief   Header file for beacon GPIO interface using lgpio library
 *
 * @copyright (c) 2024 Scout Berry. All rights reserved.
 *
 * This file contains the interface for reading avalanche beacon signals
 * from GPIO pins on Raspberry Pi using the lgpio library.
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

// Signal strength measurement (RSSI) - analog input
#define RSSI_PIN           18  // ADC pin for signal strength

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Beacon data structure containing all beacon information
 */
typedef struct {
    uint8_t bearing;           // Bearing in degrees (0, 45, 90, 270, 325)
    float distance;            // Distance in meters from 7-segment display
    int8_t signal_strength;    // RSSI value (-dBm)
    uint32_t timestamp;       // Unix timestamp
    bool signal_detected;      // Beacon detection flag
    bool bearing_valid;        // Bearing measurement validity
    bool distance_valid;       // Distance measurement validity
} BeaconData_t;

/**
 * @brief GPIO callback function type for bearing detection
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
 * @brief Get current signal strength (RSSI)
 * @return Signal strength in dBm, or -999 if invalid
 */
int8_t BeaconGpio_GetSignalStrength(void);

/**
 * @brief Enable or disable debug output
 * @param enable true to enable debug output, false to disable
 */
void BeaconGpio_SetDebug(bool enable);

#ifdef __cplusplus
}
#endif

#endif // BEACON_GPIO_H
/************************ (C) COPYRIGHT Scout Berry *******END OF FILE******/
