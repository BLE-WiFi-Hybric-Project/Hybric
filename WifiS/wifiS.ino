#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "FREE-WIFI";
const char *password = "20012005";
String fileName = "/example.txt";

AsyncWebServer server(80);

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
  Serial.print(WiFi.localIP());

  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  // Route to handle file download
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
  // Nothing to do here
}
