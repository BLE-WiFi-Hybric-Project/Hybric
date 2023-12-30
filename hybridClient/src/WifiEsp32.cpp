#include "WiFiEsp32.h"

const char *ssid = "ESP32";
const char *password = "88888888";
const char *serverIP = "192.168.4.1"; // SoftAP IP address
const int serverPort = 80;

const int bufferSize = 256; // Adjust the buffer size as needed
bool waitingForAck = false;
size_t lastAckData;

bool ACK(WiFiClient &client)
{
    unsigned long startTime = millis();
    while (millis() - startTime < 5000)
    {
        String ack = client.readStringUntil('\r');
        if (ack == "ACK")
            return true;

        delay(10); // Adjust the delay as needed
    }
    // Timeout, no acknowledgment received
    return false;
}

void wifi_setup()
{
    // Connect to SoftAP
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.println("Connecting to WiFi...");
    }
    digitalWrite(0, HIGH);

    Serial.println("Connected to WiFi");

    root = SPIFFS.open("/");
    fileSend = root.openNextFile();
}

void wifi_loop()
{
    // Connect to the server
    WiFiClient client;
    if (client.connect(serverIP, serverPort))
    {
        Serial.println("Connected to server");
        digitalWrite(1, HIGH);

        // Create a buffer to store chunks of data
        char buffer[bufferSize];

        // Read data from file and send it to the server in chunks
        while (fileSend.available())
        {
            size_t bytesRead = fileSend.readBytes(buffer, bufferSize);
            lastAckData = bytesRead;
            client.write(buffer, bytesRead);

            while (!ACK(client))
            {
                retransmissionCount++;
                if (retransmissionCount <= MAX_RETRANSMISSIONS)
                {
                    // Retransmit the same chunk
                    Serial.println("Retransmitting chunk...");
                    client.write(buffer, lastAckData);
                }
                else
                {
                    Serial.println("Max retransmissions reached. Aborting.");
                    break;
                }
            }
            retransmissionCount = 0;
        }

        // Close the file
        fileSend.close();
        Serial.println("File sent");
        digitalWrite(1, HIGH);
        switchToWiFi = false;

        // Close the connection
        client.stop();
        Serial.println("Disconnected from server");
    }
    else
    {
        Serial.println("Connection to server failed");
    }

    // Delay before trying again
    delay(1000);
}
