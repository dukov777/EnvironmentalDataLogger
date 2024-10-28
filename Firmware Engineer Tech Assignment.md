# Firmware Engineer Tech Assignment

### **Assignment: Environmental Data Logger**

**Objective:** Develop a firmware solution that reads temperature and humidity data from a sensor connected to an STM32 microcontroller and transmits the data via Wi-Fi using an ESP32 module. You can implement a function that generates random temperature and humidity values.

**Tasks:**

1. **STM32:**  
   * Set up the STM32 to read temperature and humidity values.  
   * Store the data in a simple buffer.  
   * Use low-power modes when not reading data.  
2. **ESP32:**  
   * Establish communication with the STM32 via UART or SPI.  
   * Receive the sensor data from STM32.  
   * Send the data to a cloud service or local server via Wi-Fi (using a free service like ThingSpeak).

**Tools/Libraries:**

* **STM32:** Use STM32CubeIDE with HAL libraries.  
* **ESP32:** Use the Arduino IDE with the ESP32 core and Wi-Fi libraries.  
* **Sensor:** DHT22 with corresponding libraries.

  