#include <WiFi.h>
#include <SPIFFS.h>

const char *ssid = "FREE-WIFI";
const char *password = "20012005";
const char* serverAddress = "192.168.1.1";
const int serverPort = 8888;
WiFiClient client;

unsigned long startMillis;
bool waitingForAck = false;
size_t lastBytesSent = 0;
File file;

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  File root = SPIFFS.open("/");
  file = root.openNextFile();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Kết nối đến máy chủ (ESP32 #1)
  if (client.connect(serverAddress, serverPort)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Connection to server failed");
  }
}

void loop() {
  if (client.connected()) {
    if (!file) {
      Serial.println("Failed to open file");
      client.stop();
      return;
    }
    
    // Send a frame only if not waiting for ACK
    uint8_t buffer[1460];
    if (!waitingForAck) {
        // Adjust the size as needed
      
      size_t bytesRead = file.read(buffer, sizeof(buffer));
      client.write(buffer, bytesRead);
      lastBytesSent = bytesRead;
      waitingForAck = true;
    }

    // Check for ACK
    if (client.available()) {
      char ack = client.read();
      if (ack == 'A') {
        waitingForAck = false;
      }
    }

    // Check for timeout (assuming 1000 ms timeout for simplicity)
    if (waitingForAck && (millis() - startMillis > 1000)) {
      Serial.println("Timeout! Retransmitting...");
      client.write(buffer, lastBytesSent);
      waitingForAck = false;
    }
  }

  // Check if the entire file has been sent
  if (file.available() == 0 && !waitingForAck) {
    Serial.println("File sent successfully");
    client.stop();
    file.close();
  }
}
