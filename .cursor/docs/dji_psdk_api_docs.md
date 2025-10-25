### build_dpk.sh Examples

Source: https://github.com/dji-sdk/payload-sdk/blob/master/tools/build_dpk/README.md

Provides practical examples of how to use the build_dpk.sh script to generate .dpk application installation packages.

```shell
build_dpk.sh -i ../../samples/sample_c/platform/linux/manifold3/app_json/app.json
build_dpk.sh -i ../../samples/sample_c/platform/linux/manifold3/app_json/app.json -o dpk
```

--------------------------------

### file2c Usage and Examples

Source: https://github.com/dji-sdk/payload-sdk/blob/master/tools/file2c/readme.txt

Demonstrates the command-line usage of file2c, including how to specify input files and the expected output file naming convention. It shows a practical example of embedding an image file.

```bash
file2c [OPTION...] FILE [FILE...]

Examples:
  file2c icon.png              Create 'icon_png.h' 'icon_png.c" from the contents of 'bar.bin'
```

--------------------------------

### Startup Files for STM32F4

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Assembly files containing the vector table and initial CPU setup for the STM32F4 microcontroller.

```assembly
/* startup_stm32f407vgtx.s */
// Startup file for STM32F407VGtx

/* startup_stm32f401xc.s */
// Startup file for STM32F401XC
```

--------------------------------

### CMake Build Configuration for DJI Payload SDK

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/linux/raspberry_pi/CMakeLists.txt

This snippet details the core CMake configuration, including project setup, compiler flags, and platform detection. It sets up the build environment for the DJI Payload SDK on different architectures.

```cmake
cmake_minimum_required(VERSION 3.5)
project(dji_sdk_demo_on_rpi C)

set(CMAKE_C_FLAGS "-pthread -std=gnu99")
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")
set(CMAKE_EXE_LINKER_FLAGS "-pthread")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
add_definitions(-D_GNU_SOURCE)

if (NOT USE_SYSTEM_ARCH)
    add_definitions(-DSYSTEM_ARCH_LINUX)
endif ()

if (BUILD_TEST_CASES_ON MATCHES TRUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
endif ()

set(PACKAGE_NAME payloadsdk)

## "uname -m" to auto distinguish Manifold2-G or Manifold2-C
execute_process(COMMAND uname -m
        OUTPUT_VARIABLE DEVICE_SYSTEM_ID)

if (DEVICE_SYSTEM_ID MATCHES x86_64)
    set(TOOLCHAIN_NAME x86_64-linux-gnu-gcc)
    add_definitions(-DPLATFORM_ARCH_x86_64=1)
elseif (DEVICE_SYSTEM_ID MATCHES aarch64)
    set(TOOLCHAIN_NAME aarch64-linux-gnu-gcc)
    add_definitions(-DPLATFORM_ARCH_aarch64=1)
else ()
    message(FATAL_ERROR "FATAL: Please confirm your platform.")
endif ()

file(GLOB_RECURSE MODULE_COMMON_SRC ../common/*.c)
file(GLOB_RECURSE MODULE_HAL_SRC hal/*.c)
file(GLOB_RECURSE MODULE_APP_SRC application/*.c)
file(GLOB_RECURSE MODULE_SAMPLE_SRC ../../../module_sample/*.c)

include_directories(../../../module_sample)
include_directories(../common)
include_directories(application)

include_directories(../../../../../psdk_lib/include)
link_directories(../../../../../psdk_lib/lib/${TOOLCHAIN_NAME})
link_libraries(${CMAKE_CURRENT_LIST_DIR}/../../../../../psdk_lib/lib/${TOOLCHAIN_NAME}/lib${PACKAGE_NAME}.a)

if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

add_executable(${PROJECT_NAME}
        ${MODULE_APP_SRC}
        ${MODULE_SAMPLE_SRC}
        ${MODULE_COMMON_SRC}
        ${MODULE_HAL_SRC})
```

--------------------------------

### Manifold3 Firmware Version Compatibility

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/linux/manifold3/app_json/README.md

Specifies the minimum and maximum Manifold3 device firmware versions required for the application to install and run correctly. Installation fails if the device firmware is outside this defined range. Versions are formatted as xx.xx.xx.xx, with segments ranging from 0 to 99.

```APIDOC
ver_min
  Description: Indicates the minimum version of the Manifold3 device firmware package required for the application to run properly. If the firmware package version of the Manifold3 device is lower than this version, the application will not be able to install correctly. The value is in the format of xx.xx.xx.xx, where each segment can range from 0 to 99.
  Data type: string
  Optional: No

ver_max
  Description: Indicates the maximum version of the Manifold3 device firmware package required for the application to run properly. If the firmware package version of the Manifold3 device is higher than this version, the application will not be able to install correctly. The value is in the format of xx.xx.xx.xx, where each segment can range from 0 to 99.
  Data type: string
  Optional: No
```

--------------------------------

### LIBUSB Dependency Check and Integration

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c++/platform/linux/nvidia_jetson/CMakeLists.txt

Checks for an installed LIBUSB package. If found, it adds a definition for libusb installation and links the usb-1.0 library. Otherwise, it indicates that LIBUSB cannot be found.

```cmake
find_package(LIBUSB REQUIRED)
if (LIBUSB_FOUND)
    message(STATUS "Found LIBUSB installed in the system")
    message(STATUS " - Includes: ${LIBUSB_INCLUDE_DIR}")
    message(STATUS " - Libraries: ${LIBUSB_LIBRARY}")

    add_definitions(-DLIBUSB_INSTALLED)
    target_link_libraries(${PROJECT_NAME} usb-1.0)
else ()
    message(STATUS "Cannot Find LIBUSB")
endif (LIBUSB_FOUND)
```

--------------------------------

### Dependency Management and Linking for DJI Payload SDK

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/linux/raspberry_pi/CMakeLists.txt

This snippet demonstrates how the CMake script finds and links external libraries like OPUS and LIBUSB. It includes conditional logic to handle cases where these libraries might not be installed, along with adding definitions and linking the main executable.

```cmake
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../common/3rdparty)
find_package(OPUS REQUIRED)
if (OPUS_FOUND)
    message(STATUS "Found OPUS installed in the system")
    message(STATUS " - Includes: ${OPUS_INCLUDE_DIR}")
    message(STATUS " - Libraries: ${OPUS_LIBRARY}")

    add_definitions(-DOPUS_INSTALLED)
    target_link_libraries(${PROJECT_NAME} /usr/local/lib/libopus.a)
else ()
    message(STATUS "Cannot Find OPUS")
endif (OPUS_FOUND)

find_package(LIBUSB REQUIRED)
if (LIBUSB_FOUND)
    message(STATUS "Found LIBUSB installed in the system")
    message(STATUS " - Includes: ${LIBUSB_INCLUDE_DIR}")
    message(STATUS " - Libraries: ${LIBUSB_LIBRARY}")

    add_definitions(-DLIBUSB_INSTALLED)
    target_link_libraries(${PROJECT_NAME} usb-1.0)
else ()
    message(STATUS "Cannot Find LIBUSB")
endif (LIBUSB_FOUND)

target_link_libraries(${PROJECT_NAME} m dl)

add_custom_command(TARGET ${PROJECT_NAME}
        PRE_LINK COMMAND cmake ..
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
```

--------------------------------

### OPUS Dependency Check and Integration

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c++/platform/linux/nvidia_jetson/CMakeLists.txt

Checks for an installed OPUS package. If found, it adds a definition for OPUS installation and links the OPUS library. Otherwise, it indicates that OPUS cannot be found.

```cmake
find_package(OPUS REQUIRED)
if (OPUS_FOUND)
    message(STATUS "Found OPUS installed in the system")
    message(STATUS " - Includes: ${OPUS_INCLUDE_DIR}")
    message(STATUS " - Libraries: ${OPUS_LIBRARY}")

    add_definitions(-DOPUS_INSTALLED)
    target_link_libraries(${PROJECT_NAME} /usr/local/lib/libopus.a)
else ()
    message(STATUS "Cannot Find OPUS")
endif (OPUS_FOUND)
```

--------------------------------

### OpenCV Integration

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c++/platform/linux/manifold2/CMakeLists.txt

Checks for OpenCV installation and configures the build to use it for image display in AdvancedSensing APIs. If found, it defines OPEN_CV_INSTALLED and links OpenCV include directories and libraries.

```cmake
find_package(OpenCV QUIET)
if (OpenCV_FOUND)
    message("\n${PROJECT_NAME}...")
    message(STATUS "Found OpenCV installed in the system, will use it to display image in AdvancedSensing APIs")
    message(STATUS " - Includes: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS " - Libraries: ${OpenCV_LIBRARIES}")
    add_definitions(-DOPEN_CV_INSTALLED)
else ()
    message(STATUS "Did not find OpenCV in the system, image data is inside RecvContainer as raw data")
endif ()
```

--------------------------------

### CMake Build Configuration for DJI Payload SDK

Source: https://github.com/dji-sdk/payload-sdk/blob/master/CMakeLists.txt

Configures the CMake build system for the DJI Payload SDK. It supports selecting between Linux and RTOS targets, setting up cross-compilation toolchains for RTOS, and determining the correct library path based on the Linux architecture (x86_64 or aarch64). It also handles the installation of the SDK library and header files.

```cmake
cmake_minimum_required(VERSION 2.8)

if (NOT USE_SYSTEM_ARCH)
    # select use platform 'LINUX' or 'RTOS' here, reset cache and reload cmake project
    set(USE_SYSTEM_ARCH LINUX)
endif ()

if (USE_SYSTEM_ARCH MATCHES RTOS)
    cmake_minimum_required(VERSION 3.15)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_AR arm-none-eabi-ar)
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
    set(CMAKE_OBJDUMP arm-none-eabi-objdump)
    set(SIZE arm-none-eabi-size)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif ()

project(entry)

# Disable in-source builds to prevent source tree corruption.
if (" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.")
endif ()

if (USE_SYSTEM_ARCH MATCHES LINUX)
    add_definitions(-DSYSTEM_ARCH_LINUX)
    add_subdirectory(samples/sample_c/platform/linux/manifold2)
    add_subdirectory(samples/sample_c++/platform/linux/manifold2)
    
    execute_process(COMMAND uname -m OUTPUT_VARIABLE DEVICE_SYSTEM_ID)
    if (DEVICE_SYSTEM_ID MATCHES x86_64)
        set(LIBRARY_PATH psdk_lib/lib/x86_64-linux-gnu-gcc)
    elseif (DEVICE_SYSTEM_ID MATCHES aarch64)
        set(LIBRARY_PATH psdk_lib/lib/aarch64-linux-gnu-gcc)
    else ()
        message(FATAL_ERROR "FATAL: Please confirm your platform.")
    endif ()

    install(FILES ${LIBRARY_PATH}/libpayloadsdk.a
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
            )

    install(DIRECTORY psdk_lib/include
            DESTINATION "${CMAKE_INSTALL_PREFIX}"
            )
elseif (USE_SYSTEM_ARCH MATCHES RTOS)
    add_definitions(-DSYSTEM_ARCH_RTOS)
    add_subdirectory(samples/sample_c/platform/rtos_freertos/stm32f4_discovery/project/armgcc)
endif ()

add_custom_target(${PROJECT_NAME} ALL)

```

--------------------------------

### build_dpk.sh Usage and Options

Source: https://github.com/dji-sdk/payload-sdk/blob/master/tools/build_dpk/README.md

Details the command-line arguments and options for the build_dpk.sh script, including input file specification, output path, and help.

```shell
build_dpk.sh -i input app.json file
    [-o output dpk file path]
    [-h help]

Options:
  -h        Show this help message and exit
  -i        Specify input app.json file
  -o        Specify build dpk file path
```

--------------------------------

### STM32 Startup Files for GCC

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Assembly files for initializing STM32 microcontrollers for the GCC compiler. These files contain the entry point for the program and set up the initial stack and vector table.

```assembly
startup_stm32f401xc.s
```

```assembly
startup_stm32f401xe.s
```

```assembly
startup_stm32f405xx.s
```

```assembly
startup_stm32f407xx.s
```

```assembly
startup_stm32f410cx.s
```

```assembly
startup_stm32f410rx.s
```

```assembly
startup_stm32f410tx.s
```

```assembly
startup_stm32f411xe.s
```

```assembly
startup_stm32f412cx.s
```

```assembly
startup_stm32f412rx.s
```

```assembly
startup_stm32f412vx.s
```

```assembly
startup_stm32f412zx.s
```

```assembly
startup_stm32f413xx.s
```

```assembly
startup_stm32f415xx.s
```

```assembly
startup_stm32f417xx.s
```

```assembly
startup_stm32f423xx.s
```

```assembly
startup_stm32f427xx.s
```

```assembly
startup_stm32f429xx.s
```

```assembly
startup_stm32f437xx.s
```

```assembly
startup_stm32f439xx.s
```

```assembly
startup_stm32f446xx.s
```

```assembly
startup_stm32f469xx.s
```

```assembly
startup_stm32f479xx.s
```

--------------------------------

### DJI Payload SDK C++ Sample: Camera Manager (C++)

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Sample C++ code for managing camera functionalities within the DJI Payload SDK. It includes entry point files for testing camera manager operations.

```c++
// test_camera_manager_entry.cpp
// test_camera_manager_entry.h
```

--------------------------------

### DJI Payload SDK RTOS Sample Environment

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/rtos_freertos/gd32f527_development_board/README.txt

Details the hardware, IDE, and compiler toolchain used for the DJI Payload SDK RTOS Sample. This includes specific versions of Armcc, Armasm, ArmLink, ArmAr, and FromElf.

```APIDOC
Test Environment:
  Hardware:               GD32F527ZMT7 (External Crystal Frequency: 12MHz)
  IDE:                    Keil MDK v5.26.2.0
  C Compiler:             Armcc.exe V5.06 update 6 (build 750)
  Assembler:              Armasm.exe V5.06 update 6 (build 750)
  Linker/Locator:         ArmLink.exe V5.06 update 6 (build 750)
  Library Manager:        ArmAr.exe V5.06 update 6 (build 750)
  Hex Converter:          FromElf.exe V5.06 update 6 (build 750)
```

--------------------------------

### DJI Payload SDK C++ Sample: Perception (C++)

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Sample C++ code for perception functionalities within the DJI Payload SDK. It includes entry point files for perception-related tests.

```c++
// test_perception.cpp
// test_perception_entry.cpp
```

--------------------------------

### Project Build Configurations

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Contains project configuration files for different build environments, including CMakeLists.txt for GCC, linker scripts, and MDK project files.

```makefile
// project/armgcc/CMakeLists.txt
// project/armgcc/STM32F407VGTX_FLASH.ld
// project/armgcc/stm32f4discovery.cfg
// project/mdk/mdk_app.sct
// project/mdk/mdk_app.uvprojx
// project/mdk_bootloader/mdk_bootloader.uvprojx
```

--------------------------------

### STM32 Microcontroller Startup Files

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Assembly files (.s) that contain the startup code for various STM32 microcontroller families. These files initialize the system, set up the vector table, and prepare the environment for C/C++ execution.

```assembly
startup_stm32f401xc.s
startup_stm32f401xe.s
startup_stm32f405xx.s
startup_stm32f407xx.s
startup_stm32f410cx.s
startup_stm32f410rx.s
startup_stm32f410tx.s
startup_stm32f411xe.s
startup_stm32f412cx.s
startup_stm32f412rx.s
startup_stm32f412vx.s
startup_stm32f412zx.s
startup_stm32f413xx.s
startup_stm32f415xx.s
startup_stm32f417xx.s
startup_stm32f423xx.s
startup_stm32f427xx.s
startup_stm32f429xx.s
startup_stm32f437xx.s
startup_stm32f439xx.s
startup_stm32f446xx.s
startup_stm32f469xx.s
startup_stm32f479xx.s
```

--------------------------------

### OpenCV Dependency Check and Integration

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c++/platform/linux/nvidia_jetson/CMakeLists.txt

Checks for an installed OpenCV package. If found, it configures the build to use OpenCV for displaying images in AdvancedSensing APIs, including setting include directories and linking libraries. Otherwise, it notes that image data will be handled as raw data.

```cmake
if (BUILD_CROSS_COMPILE MATCHES TRUE)
    # Try to see if user has OpenCV installed
    # if yes, default callback will display the image
    find_package(OpenCV QUIET)
    if (OpenCV_FOUND)
        message("\n${PROJECT_NAME}...")
        message(STATUS "Found OpenCV installed in the system, will use it to display image in AdvancedSensing APIs")
        message(STATUS " - Includes: ${OpenCV_INCLUDE_DIRS}")
        message(STATUS " - Libraries: ${OpenCV_LIBRARIES}")
        add_definitions(-DOPEN_CV_INSTALLED)
        target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
    else ()
        message(STATUS "Did not find OpenCV in the system, image data is inside RecvContainer as raw data")
    endif ()

    # ... other find_package calls ...
endif ()
```

--------------------------------

### Executable Creation and Dependency Linking

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/linux/nvidia_jetson/CMakeLists.txt

This CMake snippet defines the main executable for the project, including all discovered source files. It also ensures the executable output path is set and links against the math ('m') and dynamic loading ('dl') libraries.

```cmake
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

add_executable(${PROJECT_NAME}
        ${MODULE_APP_SRC}
        ${MODULE_SAMPLE_SRC}
        ${MODULE_COMMON_SRC}
        ${MODULE_HAL_SRC})

target_link_libraries(${PROJECT_NAME} m dl)
```

--------------------------------

### STM32 Startup Files for IAR

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Assembly files for initializing STM32 microcontrollers for the IAR compiler. These files are essential for the correct startup sequence of the embedded system.

```assembly
startup_stm32f401xc.s
```

```assembly
startup_stm32f401xe.s
```

```assembly
startup_stm32f405xx.s
```

```assembly
startup_stm32f407xx.s
```

```assembly
startup_stm32f410cx.s
```

```assembly
startup_stm32f410rx.s
```

```assembly
startup_stm32f410tx.s
```

```assembly
startup_stm32f411xe.s
```

```assembly
startup_stm32f412cx.s
```

```assembly
startup_stm32f412rx.s
```

```assembly
startup_stm32f412vx.s
```

```assembly
startup_stm32f412zx.s
```

```assembly
startup_stm32f413xx.s
```

```assembly
startup_stm32f415xx.s
```

```assembly
startup_stm32f417xx.s
```

```assembly
startup_stm32f423xx.s
```

```assembly
startup_stm32f427xx.s
```

```assembly
startup_stm32f429xx.s
```

```assembly
startup_stm32f437xx.s
```

```assembly
startup_stm32f439xx.s
```

```assembly
startup_stm32f446xx.s
```

```assembly
startup_stm32f469xx.s
```

```assembly
startup_stm32f479xx.s
```

--------------------------------

### DJI Payload SDK Sample: Camera Emulator Media File Management

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Sample C code demonstrating how to manage media files (JPG, MP4, H264) using the camera emulator functionality of the DJI Payload SDK. Includes core logic and specific file type handling.

```c
// dji_media_file_core.c
#include "dji_media_file_core.h"
#include "dji_media_file_jpg.h"
#include "dji_media_file_mp4.h"

// Core media file management functions
void PSDK_MediaFile_Init() {
    // Initialization logic
}

// dji_media_file_jpg.c
#include "dji_media_file_jpg.h"

// JPG specific functions
void PSDK_MediaFile_ProcessJPG(const char* filename) {
    // Process JPG file
}

// dji_media_file_mp4.c
#include "dji_media_file_mp4.h"

// MP4 specific functions
void PSDK_MediaFile_ProcessMP4(const char* filename) {
    // Process MP4 file
}

```

```c
// test_payload_cam_emu_media.h
#ifndef TEST_PAYLOAD_CAM_EMU_MEDIA_H
#define TEST_PAYLOAD_CAM_EMU_MEDIA_H

void Test_CameraEmulatorMedia();

#endif // TEST_PAYLOAD_CAM_EMU_MEDIA_H

// test_payload_cam_emu_media.c
#include "test_payload_cam_emu_media.h"
#include "dji_media_file_core.h"

void Test_CameraEmulatorMedia() {
    PSDK_MediaFile_Init();
    PSDK_MediaFile_ProcessJPG("PSDK_0001_ORG.jpg");
    PSDK_MediaFile_ProcessMP4("PSDK_0004_ORG.mp4");
}

```

--------------------------------

### DJI SDK Payload SDK - Waypoint V3 Tests and Files

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Includes C and header files for testing Waypoint V3, along with a test KMZ file and its C header representation for waypoint data.

```c
test_waypoint_v3.c
```

```h
test_waypoint_v3.h
```

```kmz
waypoint_v3_test_file.kmz
```

```h
waypoint_v3_test_file_kmz.h
```

--------------------------------

### DJI Payload SDK File Structure

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Overview of the directory and file organization within the DJI Payload SDK project.

```txt
├── test_perception_entry.hpp
│       └── test_perception.hpp
│       └── platform
│           └── linux
│               ├── common
│               │   ├── 3rdparty
│               │   │   ├── FindFFMPEG.cmake
│               │   │   ├── FindLIBUSB.cmake
│               │   │   └── FindOPUS.cmake
│               │   └── osal
│               │       ├── osal.c
│               │       ├── osal_fs.c
│               │       ├── osal_fs.h
│               │       ├── osal.h
│               │       ├── osal_socket.c
│               │       └── osal_socket.h
│               ├── manifold2
│               │   ├── application
│               │   │   ├── application.cpp
│               │   │   ├── application.hpp
│               │   │   ├── dji_sdk_app_info.h
│               │   │   ├── dji_sdk_config.h
│               │   │   ├── dji_sdk_config.json
│               │   │   └── main.cpp
│               │   ├── CMakeLists.txt
│               │   └── hal
│               │       ├── hal_network.c
│               │       ├── hal_network.h
│               │       ├── hal_uart.c
│               │       ├── hal_uart.h
│               │       ├── hal_usb_bulk.c
│               │       └── hal_usb_bulk.h
│               └── nvidia_jetson
│                   ├── application
│                   │   ├── application.cpp
│                   │   ├── application.hpp
│                   │   ├── dji_sdk_app_info.h
│                   │   ├── dji_sdk_config.h
│                   │   └── main.cpp
│                   ├── CMakeLists.txt
│                   └── hal
│                       ├── hal_network.c
│                       ├── hal_network.h
│                       ├── hal_uart.c
│                       ├── hal_uart.h
│                       ├── hal_usb_bulk.c
│                       └── hal_usb_bulk.h
└── tools
    └── file2c
        ├── file2c.exe
        └── readme.txt
```

--------------------------------

### Define Dependency Paths and Preprocessor Macros in CMake

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/rtos_freertos/stm32f4_discovery/project/armgcc/CMakeLists.txt

This section specifies all necessary include directories for the DJI Payload SDK project, covering application code, STM32 HAL drivers, CMSIS, USB Host, BSP, OSAL, FreeRTOS, and STM32 USB Host Library paths. It also configures the linker to find and link the `libpayloadsdk.a` static library and adds global preprocessor definitions.

```CMake
include_directories(../../../../../module_sample
        ../../application
        ../../drivers/STM32F4xx_HAL_Driver/Inc
        ../../drivers/STM32F4xx_HAL_Driver/Inc/Legacy
        ../../drivers/CMSIS/Device/ST/STM32F4xx/Include
        ../../drivers/CMSIS/Include
        ../../drivers/USB_HOST/Target
        ../../drivers/USB_HOST/App
        ../../drivers/BSP
        ../../hal/
        ../../../common/osal/
        ../../middlewares/Third_Party/FreeRTOS/Source/include
        ../../middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
        ../../middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
        ../../middlewares/ST/STM32_USB_Host_Library/Core/Inc
        ../../middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../../../../../psdk_lib/include)
link_directories(${CMAKE_CURRENT_LIST_DIR}/../../../../../../../psdk_lib/lib/arm-none-eabi-gcc)
link_libraries(${CMAKE_CURRENT_LIST_DIR}/../../../../../../../psdk_lib/lib/arm-none-eabi-gcc/libpayloadsdk.a)

add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx)
```

--------------------------------

### Define Executable and Link Libraries

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c++/platform/linux/raspberry_pi/CMakeLists.txt

Sets the CMake module path, specifies directories for linking libraries, and links the core `libpayloadsdk.a` and `stdc++` libraries. It then defines the main executable target using all collected source files, ensures essential system libraries (`m` for math, `dl` for dynamic linking) are linked, and sets the executable's output directory.

```CMake
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../common/3rdparty)

link_directories(${CMAKE_CURRENT_LIST_DIR}/../../../../../psdk_lib/lib/aarch64-linux-gnu-gcc)
link_libraries(${CMAKE_CURRENT_LIST_DIR}/../../../../../psdk_lib/lib/aarch64-linux-gnu-gcc/libpayloadsdk.a -lstdc++)

add_executable(${PROJECT_NAME}
        ${MODULE_APP_SRC}
        ${MODULE_SAMPLE_SRC}
        ${MODULE_COMMON_SRC}
        ${MODULE_HAL_SRC})

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../../../psdk_lib/include)

if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

target_link_libraries(${PROJECT_NAME} m dl)
```

--------------------------------

### Project Linking and Include Directories

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c++/platform/linux/manifold2/CMakeLists.txt

Configures the executable output path, links the 'm' and 'dl' system libraries, and sets private include directories for OpenCV and the project's own headers.

```cmake
if (NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

target_link_libraries(${PROJECT_NAME} m)

target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
if (OpenCV_FOUND)
    target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
endif ()

target_link_libraries(${PROJECT_NAME} dl)
```

--------------------------------

### DJI Payload SDK Sample: Camera Manager Tests

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Sample C code for testing the camera manager functionalities within the DJI Payload SDK. Includes header and source files for camera manager operations.

```c
// test_camera_manager.h
#ifndef TEST_CAMERA_MANAGER_H
#define TEST_CAMERA_MANAGER_H

void Test_CameraManager();

#endif // TEST_CAMERA_MANAGER_H

// test_camera_manager.c
#include "test_camera_manager.h"
#include "dji_camera_manager.h"

void Test_CameraManager() {
    // Example usage of camera manager functions
    // dji_rc_get_channel_data(DJI_CHANNEL_5);
}

```

--------------------------------

### DJI SDK Payload SDK - Widget Tests and Resources

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

Provides C and header files for testing widget functionalities, including binary array lists and interaction tests. Also includes widget configuration files and image assets for different locales (Chinese and English) and screen types.

```c
file_binary_array_list_en.c
```

```h
file_binary_array_list_en.h
```

```c
test_widget.c
```

```h
test_widget.h
```

```c
test_widget_speaker.c
```

```h
test_widget_speaker.h
```

```json
widget_config.json
```

```h
icon_button1_png.h
```

```h
icon_button2_png.h
```

```h
icon_list_item1_png.h
```

```h
icon_list_item2_png.h
```

```h
icon_scale_png.h
```

```h
icon_switch_select_png.h
```

```h
icon_switch_unselect_png.h
```

```h
icon_button1_png.h
```

```h
icon_button2_png.h
```

```h
icon_list_item1_png.h
```

```h
icon_list_item2_png.h
```

```h
icon_scale_png.h
```

```h
icon_switch_select_png.h
```

```h
icon_switch_unselect_png.h
```

```json
widget_config.json
```

```h
icon_button1_png.h
```

--------------------------------

### USB Host Configuration Source Files

Source: https://github.com/dji-sdk/payload-sdk/blob/master/LICENSE.txt

This snippet includes the configuration source files for the USB Host Library. These C files provide the implementation for the USB host configurations, enabling specific functionalities and middleware integration.

```c
#include "usbh_conf.c"
```

--------------------------------

### Application Executable Binary Path

Source: https://github.com/dji-sdk/payload-sdk/blob/master/samples/sample_c/platform/linux/manifold3/app_json/README.md

Defines the relative path to the compiled application's executable binary file. This path is relative to the 'app.json' file and must point to an existing binary file.

```APIDOC
bin
  Description: The relative path of the executable bin file generated by the compilation, indicating the application executable bin file. It must be a relative path, specifically the path relative to the app.json file, and there must be a corresponding bin file present at that path.
  Data type: string
  Optional: No
```