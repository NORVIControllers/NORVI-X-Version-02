# NORVI X

**NORVI X** is a next-generation modular industrial IoT controller designed for modern automation applications.

## Overview

NORVI X is a flexible and modular industrial IoT controller suitable for both small projects and large factories.
NORVI X simplifies monitoring, control, and data acquisition, making deployment efficient and straightforward.

## Expansion Module Overview
Our expansion modules are designed to scale with your project's complexity. Version 01 features a standard 40-pin interface, ideal for core connectivity and essential I/O tasks. 
For more advanced applications, Version 02 expands this to 50 pins, providing the additional overhead needed for enhanced data throughput and increased peripheral support.

![measurement](Images/measurement.PNG)

## Arduino IDE Configuration
To program the ESP32S3 microcontroller using the Arduino IDE, follow these steps:

**Board Configuration**  
- **Board:** ESP32S3 Dev Module  

### Required Setup Steps

- **Arduino IDE Version:** 1.8.19
1. Connect your board via USB interface.  
2. Select the correct board and port:  
   - Go to: `Tools → Board → Boards Manager`  
3. Configure the upload method under the `Tools` menu.

### Required Libraries
Ensure the following libraries are installed with the correct versions for proper operation:

- Open Arduino IDE → `Sketch → Include Library → Manage Libraries...`  
- Search for the library name and install the mentioned version.  
- Alternatively, clone the repositories directly into your Arduino libraries folder.

## Upload and Board Details
Example configurations and details for uploading code to the ESP32S3:

### Upload Configuration
![Upload Details](Images/Upload_Details.png)
## Technical Highlights
- Modular and expandable design  
- Reliable industrial-grade hardware  
- Easy integration with sensors and actuators  
## Support
For technical support or additional documentation, please visit: [NORVI Support](https://norvi.io/contact-us/)
