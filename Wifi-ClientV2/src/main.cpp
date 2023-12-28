#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

File root;
File file;
const char *ssid = "ESP32";
const char *password = "88888888";
const char *serverIP = "192.168.4.1";
bool switchToWiFi = false;

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("...");
  }
  Serial.println("\nConnected to WiFi");

  root = SPIFFS.open("/");
}

void loop()
{
  file = root.openNextFile();
  // Initialize the HTTP client
  HTTPClient http;
  String url = "http://" + String(serverIP) + "/upload";

  // Begin the HTTP request
  http.begin(url);
  http.addHeader("Content-Type", "multipart/form-data");
  http.addHeader("Content-Disposition", "attachment; filename=" + String(file.name()));

  // Send the file content
  int httpCode = http.sendRequest("POST", &file);

  // Check for a successful upload
  if (httpCode == 200)
  {
    Serial.println("File uploaded successfully");
    String response = http.getString();
    Serial.println("Server Response: " + response);
  }
  else
    Serial.println("Failed to upload file");

  // End the HTTP request and Close the file
  http.end();
  file.close();

  delay(5000); // Wait for a while before reattempting
}
