#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "ESP32";
const char *password = "88888888";

AsyncWebServer server(80);
bool ACK = false;

void setup()
{
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }
  server.on("/ack", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Handle the GET request
    ACK = true;
    request->send(200, "text/plain", "200"); });

  server.on(
      "/post", HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
      {
        // Handle incoming data (POST request payload)
        Serial.println("incoming data");

        // Create or open a file for writing
        File file = SPIFFS.open("/data.txt", "a"); // "a" means append, change to "w" for write (overwriting)

        if (!file)
          Serial.println("Failed to open file for writing");
        else
        {
          // Write data to the file
          file.write(data, len);
          file.close();
          Serial.println("Data written to file successfully");
        }

        // Send a response (HTTP status 200)
        request->send(200);
      });

  server.begin();
}

void loop()
{
  // Empty loop as the server handles requests asynchronously
  if (ACK)
  {
    Serial.println("ACK gotten");
    ACK = false;
  }
}
