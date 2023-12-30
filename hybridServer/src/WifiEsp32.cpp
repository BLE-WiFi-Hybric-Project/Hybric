#include "WiFiEsp32.h"

const char *ssid = "ESP32";
const char *password = "88888888";

WiFiServer server(80);

const int bufferSize = 256; // Adjust the buffer size as needed

void wifi_setup()
{
    // Connect to SoftAP
    WiFi.softAP(ssid, password);

    // Print the IP address
    Serial.println("Access Point IP address: " + WiFi.softAPIP().toString());

    // Start the server
    server.begin();

    Serial.println("Server started");
}

void wifi_loop()
{

    WiFiClient client = server.available();

    if (client)
    {
        Serial.println("Client connected");
        Serial.println(client.readStringUntil('\r').c_str());
        // Create a buffer to store chunks of data
        char buffer[bufferSize];
        receivedFile = SPIFFS.open("/example1.txt", FILE_APPEND);

        while (client.connected())
        {
            size_t bytesRead = client.readBytes(buffer, bufferSize);

            if (bytesRead > 0)
            {
                receivedFile.write(reinterpret_cast<const uint8_t *>(buffer), bytesRead);
                // Print received data to Serial Monitor
                // Serial.write(buffer, bytesRead);
                client.print("ACK");
                Serial.println("Send ACK");
                delay(1);
            }
        }

        receivedFile.close();
        SPIFFS.remove("/example1.txt");
        Serial.println("File received and saved.");

        // Close the connection
        client.stop();
        Serial.println("Client disconnected");
    }

    delay(1000);
}
