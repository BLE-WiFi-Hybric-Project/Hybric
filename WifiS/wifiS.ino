#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "ESP32";
const char *password = "88888888";

bool ACK = false;

AsyncWebServer server(80);

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

  server.on("/ack", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Handle the GET request
    ACK = true;
    request->send(200, "text/plain", "200"); });

  // Handle root URL
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String fileName = "/example.txt";
    File file = SPIFFS.open(fileName, "r");
    if (file) {
      request->send(SPIFFS, fileName, "text/plain");
      file.close();
    } else {
      request->send(404, "text/plain", "File not found");
    } });

  // Serve other static files (like HTML, CSS, etc.)
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Start server
  server.begin();
}

void loop()
{
  if (ACK)
  {
    Serial.println("ACK received in loop");
    ACK = false; // Reset ACK to false after processing
  }
}
