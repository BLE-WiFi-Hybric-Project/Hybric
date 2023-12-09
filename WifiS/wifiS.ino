#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <SPIFFS.h>

const char *ssid = "FREE-WIFI";
const char *password = "20012005";
const char* serverAddress = "192.168.1.13"; 
const int serverPort = 8888; 

WiFiClient ESPclient;
int retransmissionCount = 0;
const int MAX_RETRANSMISSIONS = 3;

unsigned long startMillis;
bool waitingForAck = false;
size_t lastBytesSent = 0;
File file;

bool ACK(){
  if (ESPclient.available()) {
    char ack = ESPclient.read();
    if (ack == 'A') {
      waitingForAck = false;
      Serial.println("Recieved");
      // delay(1);
      return false;
    }
  }
  return true;
}

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
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());
  }

  if (ESPclient.connect(serverAddress, serverPort)) {
    Serial.println("Connected to ESP server");
  } else {
    Serial.println("Failed to connect to ESP server");
  }
}

void loop() {
  if (ESPclient.connected()) {
    if (!file) {
      Serial.println("Failed to open file");
      ESPclient.stop();
      return;
    }
    
  // Read and send data in a loop
  uint8_t buffer[1460];
  Serial.println("Start");
  while (file.available() || waitingForAck) {
    // Check if there is new data to send
    if (!waitingForAck) {
      size_t bytesRead = file.read(buffer, sizeof(buffer));
      lastBytesSent = bytesRead;
      // Send data to the client
      ESPclient.write(buffer, bytesRead);
      waitingForAck = true;

      // Record the start time for timeout
      startMillis = millis();
    }

    // Check for timeout (assuming 1000 ms timeout for simplicity)
    while (ACK() && (millis() - startMillis > 1000)) {
      if(retransmissionCount == MAX_RETRANSMISSIONS)
        break;
      Serial.println("Timeout! Retransmitting...");
      ESPclient.write(buffer, lastBytesSent);
      // Reset the timeout start time
      startMillis = millis();
      retransmissionCount++;
    }

    retransmissionCount = 0;
  }
 
    // Close the file and the connection
    file.close();
    ESPclient.stop();
    Serial.println("Client disconnected");
  }
}
