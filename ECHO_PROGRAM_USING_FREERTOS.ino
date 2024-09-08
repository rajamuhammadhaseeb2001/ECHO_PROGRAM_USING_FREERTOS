#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else 
static const BaseType_t app_cpu = 1;
#endif

char *ptr = NULL;  // Pointer to dynamically allocated memory
volatile bool checking = false;
int i = 0;

void testTask1(void *parameter) 
{
  while(1) 
  {
    if (checking == true) // Correct comparison operator
    {  
      if (ptr != NULL) // Check if ptr is not NULL
      {
        Serial.println(ptr);  // Print the string stored in memory
        vPortFree(ptr);  // Free the allocated memory
        ptr = NULL;  // Reset the pointer to NULL after freeing
        checking = false;  // Reset the flag
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay to avoid hogging the CPU
  }
}

void testTask2(void *parameter) 
{
  while(1) 
  {
    if (ptr == NULL) 
    {  // Allocate memory only if it is not already allocated
      ptr = (char *) pvPortMalloc(1024 * sizeof(char));
      if (ptr == NULL) // Check if allocation failed 
      {  
        Serial.println("Memory allocation failed");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait before retrying
        continue;  // Retry memory allocation
      }
    }
    
    while (Serial.available() > 0) 
    {
      char c = Serial.read();
      if (c == '\n' || c == '\r') 
      { 
        if (i > 0) // Only process if there is input data
        {  
          ptr[i] = '\0';  // Null-terminate the string
          i = 0;  // Reset index for next input
          checking = true;  // Signal testTask1 to print and free memory
        }
      } 
      else 
      {
        if (i < 1023) // Ensure no buffer overflow
        {  
          ptr[i++] = c;  // Store character and increment index
        }
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Yield to other tasks
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Allow time for Serial to initialize
  Serial.println();
  Serial.println("---FreeRTOS Memory Demo---");

  xTaskCreatePinnedToCore(
    testTask1,
    "Test Task 1", 
    1200,
    NULL,
    1,
    NULL,
    app_cpu
  );

  xTaskCreatePinnedToCore(
    testTask2,
    "Test Task 2", 
    1200,
    NULL,
    1,
    NULL,
    app_cpu
  );
}

void loop() 
{
  // Empty loop as tasks handle the functionality
}
