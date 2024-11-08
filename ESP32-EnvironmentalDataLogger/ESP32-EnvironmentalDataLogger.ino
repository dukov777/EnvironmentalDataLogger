#include <WiFi.h>
#include "ThingSpeak.h"
#include <Arduino.h>
#include "secrets.h"

#define ENVIRONMENT_DATA_REPLY_SIZE 5

typedef union
{
    uint8_t buffer[ENVIRONMENT_DATA_REPLY_SIZE];
    // packet structure
    struct
    {
        uint8_t status;
        uint16_t humidity;
        int16_t temperature;
    } __attribute__((packed));
    ;
} EnvironmentDataReply_t;

EnvironmentDataReply_t dth22_data;

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password

WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 20000;


void setup()
{
    Serial.begin(115200); // Initialize serial
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    Serial2.setTimeout(5000);
    WiFi.mode(WIFI_STA);
    ThingSpeak.begin(client); // Initialize ThingSpeak
}


void loop()
{
    if ((millis() - lastTime) > timerDelay)
    {
        // Connect or reconnect to WiFi
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("Attempting to connect");
            while (WiFi.status() != WL_CONNECTED)
            {
                WiFi.begin(ssid, pass);
                delay(5000);
            }
            Serial.println("\nConnected.");
        }
        // Request DHT22 sensor data
        Serial2.write('\n');

        lastTime = millis();
    }


    // check if data is received from stm32
    if (Serial2.available() >= 5)
    {
        // once the DHT22 data is available send it to ThingSpeak
        Serial.println("Reading from device");
        size_t read_bytes = Serial2.readBytes(dth22_data.buffer, 5);

        if (read_bytes == 5)
        {
            Serial.println("Read Success");
            if (dth22_data.status == 0)
            {
                float humidity = (dth22_data.humidity>>8);
                float temperature = (dth22_data.temperature>>8);
                Serial.print("Humidity: ");
                Serial.print(humidity);
                Serial.println(" %");

                Serial.print("Temperature: ");
                Serial.print(temperature);
                Serial.println(" C");

                SendToThingSpeak(humidity, temperature);
            }
            else
            {
                Serial.print("Sensor Error: ");
                Serial.println(dth22_data.status);
            }
        }
        else
        {
            Serial.print("Failed: read_bytes is");
            Serial.print(read_bytes, DEC);
            Serial.println(" bytes");
        }
    }
}

void SendToThingSpeak(float humidity, float temperature)
{
    // set the fields with the values
    ThingSpeak.setField(1, humidity);
    ThingSpeak.setField(2, temperature);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200)
    {
        Serial.println("Channel update successful.");
    }
    else
    {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
}
