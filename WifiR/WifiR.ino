#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <SPIFFS.h>  // Include SPIFFS library for ESP32

const char *ssid = "FREE-WIFI";
const char *password = "20012005";
const char *receivedFilename = "/received_file.txt"; // Replace with your desired received file name and extension

WiFiServer ESPserver(8888);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Start the server
  ESPserver.begin();
}

void loop() {
  WiFiClient client = ESPserver.available();
  if (client) {
    Serial.println("Client connected");

    // Initialize SPIFFS
    if (!SPIFFS.begin()) {
      Serial.println("Failed to mount SPIFFS");
      return;
    }

    // Create a new file to store received data
    File file = SPIFFS.open(receivedFilename, "w");
    if (!file) {
      Serial.println("Failed to create file");
      return;
    }

    // Read data from the client and write it to the file
    uint8_t buffer[1460]; // Read in chunks of 1460 bytes
    while (client.available() && (bytesRead = client.readBytes(buffer, sizeof(buffer)))) 
    {
      file.write(buffer, bytesRead);
      client.write('A'); // Send ACK
      Serial.println("Send ACK");
    }

    // Close the file and disconnect
    file.close();
    client.stop();
  }

  delay(1000); // Wait 1 second before checking for a new client
}
