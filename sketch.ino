#include <Arduino_FreeRTOS.h>
#include <queue.h>

// Configuration
#define LED_PIN 13
#define MAX_CMD_LENGTH 10
#define QUEUE_SIZE 5

// Global variables
QueueHandle_t xCommandQueue;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  // Create queue for commands
  xCommandQueue = xQueueCreate(QUEUE_SIZE, sizeof(char[MAX_CMD_LENGTH]));
  
  // Create tasks
  xTaskCreate(serialReaderTask,"SerialReader",128,NULL,1,NULL);
  
  xTaskCreate(commandProcessorTask,"CommandProcessor",128,NULL,1,NULL);
  
  Serial.println("System ready. Send 'ON' or 'OFF'");
}

void loop() {}

// Task to read serial input
void serialReaderTask(void *pvParameters) {
  char cmd[MAX_CMD_LENGTH];
  uint8_t idx = 0;
  
  while (1) {
    if (Serial.available()) {
      char c = Serial.read();
      
      if (c == '\n' || idx >= MAX_CMD_LENGTH - 1) {
        cmd[idx] = '\0'; // Null-terminate
        if (idx > 0) {   // Only send if we got data
          xQueueSend(xCommandQueue, cmd, portMAX_DELAY);
        }
        idx = 0;
      } else if (isAlpha(c)) {
        cmd[idx++] = toupper(c); // Convert to uppercase
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Task to process commands
void commandProcessorTask(void *pvParameters) {
  char receivedCmd[MAX_CMD_LENGTH];
  
  while (1) {
    if (xQueueReceive(xCommandQueue, receivedCmd, portMAX_DELAY) == pdTRUE) {
      if (strcmp(receivedCmd, "ON") == 0) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED turned ON");
      } 
      else if (strcmp(receivedCmd, "OFF") == 0) {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED turned OFF");
      }
      else {
        Serial.println("Unknown command. Send 'ON' or 'OFF'");
      }
    }
  }
}