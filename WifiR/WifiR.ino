#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <LittleFS.h>  // Include LittleFS library

const char *ssid = "FREE-WiFI";
const char *password = "20012005";
const char *receivedFilename = "/received_file.txt"; // Replace with your desired received file name and extension

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Start the server
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");

    // Create a new file to store received data
    File file = LittleFS.open(receivedFilename, "w");
    if (!file) {
      Serial.println("Failed to create file");
      return;
    }

    // Read data from the client and write it to the file
    uint8_t buffer[1460]; // Read in chunks of 1460 bytes
    size_t bytesRead;

    while (client.available() && (bytesRead = client.readBytes(buffer, sizeof(buffer)))) 
    {
      size_t bytesRead = client.readBytes(buffer, 1460);
      file.write(buffer, bytesRead);
      client.write('A'); // Send ACK
    }

    // Close the file and disconnect
    file.close();
    client.stop();
  }

  delay(1000); // Wait 1 second before checking for a new client
}
