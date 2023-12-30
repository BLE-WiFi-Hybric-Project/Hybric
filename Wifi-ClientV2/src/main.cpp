#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "ESP32";
const char *password = "88888888";
const char *serverAddress = "192.168.4.1"; // Change this to the IP address of your ESP32 server

File root;
File file;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  root = SPIFFS.open("/");
}

void loop()
{

  // Open the file
  file = root.openNextFile();
  if (file)
  {
    // Get the file size
    size_t fileSize = file.size();

    // Read the contents of the file into a buffer
    uint8_t *fileBuffer = (uint8_t *)malloc(fileSize);
    file.read(fileBuffer, fileSize);

    // Create an HTTP client object
    HTTPClient http;

    // Construct the server URL
    String url = "http://" + String(serverAddress) + "/post";

    // Start the HTTP POST request
    http.begin(url);

    // Set the content type to multipart form data
    http.addHeader("Content-Type", "text/plain");

    // Provide the file data in the body of the request
    Serial.println("Start upload");
    int httpResponseCode = http.POST(fileBuffer, fileSize);

    // Check for a successful upload
    if (httpResponseCode == 200)
    {
      Serial.println("File uploaded successfully");
    }
    else
    {
      Serial.print("Error uploading file. HTTP response code: ");
      Serial.println(httpResponseCode);
    }

    // Close the file
    file.close();

    // End the HTTP connection
    http.end();
  }
  // Wait for some time before the next upload
  delay(5000);
}