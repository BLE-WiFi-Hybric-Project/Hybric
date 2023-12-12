// #include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

const char *ssid = "ESP32";
const char *password = "88888888";
const char *serverIP = "192.168.4.1";

void setup()
{
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print("...");
    }
    Serial.println("\nConnected to WiFi");

    if (!SPIFFS.begin())
    {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
}

void loop()
{
    // Wait for a few seconds
    delay(5000);

    // Send a GET request to the server to download the file
    HTTPClient http;
    String url = "http://" + String(serverIP) + "/download";
    String url2 = "http://" + String(serverIP) + "/ack";

    Serial.println("Sending GET request to: " + url);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
            // File received successfully
            Serial.println("--------------------Start download--------------------");

            // Save received data to a file on the client
            File file = SPIFFS.open("/recieved_example.txt", FILE_WRITE);
            if (file)
                http.writeToStream(&file);
            else
                Serial.println("Error opening file for writing");
            Serial.println("--------------------Finish download--------------------");

            // Send ACK to Server
            http.begin(url2);
            int httpCode2 = http.GET();
            if (httpCode > 0)
                Serial.println("Send ACK");
        }
        else
            Serial.printf("Failed to download file, error: %s\n", http.errorToString(httpCode).c_str());
    }
    else
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());

    http.end();
}
