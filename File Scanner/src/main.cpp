#include <Arduino.h>
#include <SPIFFS.h>

void listFiles(const char *dirname)
{
  Serial.println("Listing files in " + String(dirname));

  File root = SPIFFS.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    // Record the start time
    unsigned long startTime = millis();

    Serial.print("File: " + String(file.name()));
    Serial.print(" | Size: " + String(file.size()) + " bytes");
    Serial.print(" (" + String(file.size() / 1024.0, 3) + " KB)");

    // Record the end time
    unsigned long endTime = millis();

    // Calculate and print the elapsed time
    Serial.print(" | Elapsed Time: ");
    Serial.print(endTime - startTime);
    Serial.println(" ms");

    file = root.openNextFile();
    delay(10); // Add a small delay to allow Serial to catch up
  }
}

void setup()
{
  Serial.begin(115200);

  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS mounted successfully!");
    listFiles("/");
  }
  else
  {
    Serial.println("Failed to mount SPIFFS");
  }
}

void loop()
{
  // Nothing to do here
}
