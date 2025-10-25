# Avi Scout Hackathon Summary

## **1. Hackathon Project Overview**

The project involves capturing signals from an avalanche beacon, sending them to a Raspberry Pi for triangulation, and then transmitting flight instructions to a Mavic drone. The Raspberry Pi setup, drone integration, and communication protocols were key components.

### **1.1 Raspberry Pi Setup**

- Raspberry Pi was loaded with the correct operating system using a newer micro SD card.
    - Website:  [https://www.raspberrypi.com/software/](https://www.raspberrypi.com/software/)
- SSH port was opened for remote interaction via Wi-Fi also known as *headless* access

### **1.2 Drone Integration**

- Issues with powering the Raspberry Pi from the drone were resolved by configuring the SDK with the correct API key and credentials. The payload SDK was downloaded and built using CMake.
- **Developer Resources:**
    - [https://www.dji.com/ca/downloads/softwares/assistant-dji-2-for-mavic](https://www.dji.com/ca/downloads/softwares/assistant-dji-2-for-mavic)
    - [https://developer.dji.com/doc/payload-sdk-tutorial/en/quick-start/quick-guide/raspberry-pi.html](https://developer.dji.com/doc/payload-sdk-tutorial/en/quick-start/quick-guide/raspberry-pi.html)

### **1.3 Communication Protocols**

- **Progress** : Commands were successfully sent from the Raspberry Pi to the drone, retrieving information like the drone's serial number. The beacon communicated with the Raspberry Pi using a Python GPIO library.
- **Issues and Risks** : Writing the application in C and C++ was complex due to unfamiliarity with the languages.
- **Next Steps** : Create a single C/C++ application that leverages the DJI payload SDK and listens for signals from the beacon.

## Setup Instructions

1. Download the Raspberry Pi installer onto a computer
2. Obtain a newer microSD card (they noted an older one didn't work, so they used one from a camera)
3. Install Raspberry Pi OS onto the microSD card
4. Configure the system using RASP config
5. Setup OpenSSH port to enable headless interaction through Wi-Fi (allowing remote access from another computer)
6. For the DJI drone integration:
    - Apply for and obtain an API key for the DJI Payload SDK
    - Download the Payload SDK from GitHub
    - Follow CMake build instructions to create a binary executable (this was in C)
    - Input the API key, token, and username from the DJI Developer Center profile
    - Configure the PSDK with these credentials

**Additional hardware setup points:**

- Connected the Raspberry Pi to the drone via the E-port using an E-development kit
- Set up GPIO connections on a breadboard for the avalanche beacon receiver
- Powered the Raspberry Pi through the drone (this only worked after properly configuring the SDK credentials)

## 2. Notes

1. The integration of a single C/C++ application to handle both beacon signals and drone commands remains incomplete. Further expertise in C/C++ is needed to ensure successful integration and functionality.
2. The complexity of C/C++ programming poses a risk to timely project completion. Additional resources or personnel with expertise in these languages may be required to mitigate this risk and keep the project on schedule.
3. Determine how to compactly fit the Raspberry Pi, beacon, and development kit onto the drone. The details of this task are unclear and need further clarification to proceed effectively.

## 3. Hardware Configuration

![Raspberry Pi.png](Avi%20Scout%20Hackathon%20Summary/ca09a4ea-b696-453d-a538-ada262f8833c.png)

![mermaid-diagram-2025-01-14-194427.png](Avi%20Scout%20Hackathon%20Summary/mermaid-diagram-2025-01-14-194427.png)

![IMG_3010.jpeg](Avi%20Scout%20Hackathon%20Summary/IMG_3010.jpeg)

## 4. Dataflow

![IMG_0767.jpeg](Avi%20Scout%20Hackathon%20Summary/IMG_0767.jpeg)