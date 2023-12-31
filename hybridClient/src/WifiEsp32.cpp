#include "WiFiEsp32.h"
#include "wifiFix.h"

const char *ssid = "ESP32";
const char *password = "88888888";
const char *serverIP = "192.168.4.1"; // SoftAP IP address
const size_t chunkSize = 20 * 1024;   // 1KB

WiFiClient *wifi = new WiFiClientFixed();

void wifi_setup()
{
    Serial.begin(115200);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
}

void wifi_loop()
{
    // Open the file
    if (fileSend)
    {
        // Get the file size
        size_t fileSize = fileSend.size();

        // Create an HTTP client object
        HTTPClient http;

        // Construct the server URL
        String url = "http://" + String(serverIP) + "/post";
        String url1 = "http://" + String(serverIP) + "/ack";
        // Start the HTTP POST request
        http.begin(*wifi, url);

        // Set the content type to multipart form data
        http.addHeader("Content-Type", "text/plain");

        // Provide the file data in the body of the request
        Serial.println("Start upload");
        for (size_t pos = 0; pos < fileSize; pos += chunkSize)
        {
            // Calculate the remaining bytes to read in this chunk
            size_t bytesToRead = min(chunkSize, fileSize - pos);

            // Read the contents of the file into a buffer
            uint8_t *fileBuffer = (uint8_t *)malloc(bytesToRead);
            fileSend.seek(pos); // Move file cursor to the correct position
            fileSend.read(fileBuffer, bytesToRead);

            // Provide the file data in the body of the request
            int httpResponseCode = http.POST(fileBuffer, bytesToRead);

            // Check for a successful upload
            if (httpResponseCode == 200)
                Serial.println("OK");
            else
            {
                while (httpResponseCode != 200)
                {
                    if (retransmissionCount == MAX_RETRANSMISSIONS)
                    {
                        Serial.println("Fail to Upload FIle");
                        return;
                    }
                    Serial.print("Error uploading chunk. HTTP response code: ");
                    Serial.println(httpResponseCode);
                    http.setTimeout(1000);
                    int httpResponseCode = http.POST(fileBuffer, bytesToRead);
                    retransmissionCount++;
                }
                // Handle error if needed
            }
            retransmissionCount = 0;
            // Free the allocated memory for this chunk
            free(fileBuffer);

            http.setTimeout(1000);
        }
        http.end();

        http.begin(url1);
        int httpResponseCode = http.GET();
        if (httpResponseCode == 200)
            Serial.println("Success");

        Serial.println("Done upload");

        // Close the file
        fileSend.close();
        Serial.println("File close");

        // End the HTTP connection
        delay(100);
        http.end();
    }
    // Wait for some time before the next upload
    delay(1000);
}