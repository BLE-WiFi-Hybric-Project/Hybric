#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "ESP32";
const char *password = "88888888";

bool ACK = false;

AsyncWebServer server(80);

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
    request->_tempFile = SPIFFS.open("/" + filename, "w");

  if (len)
  {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    // close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(200, "text/plain", "File Uploaded Successfully");
  }
}

void setup()
{
  Serial.begin(115200);

  // Start access point
  WiFi.softAP(ssid, password);

  // Print the IP address
  Serial.println("Access Point IP address: " + WiFi.softAPIP().toString());

  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  server.on(
      "/upload", HTTP_POST, [](AsyncWebServerRequest *request)
      { request->send(200); },
      handleUpload);

  // Serve other static files (like HTML, CSS, etc.)
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Start server
  server.begin();
}

void loop() {}