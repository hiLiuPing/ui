#include "app_freertos.h"

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"

static void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void)
{
  BaseType_t status;

  status = xTaskCreate(StartDefaultTask,
                       "defaultTask",
                       256U,
                       NULL,
                       tskIDLE_PRIORITY + 1U,
                       NULL);
  if (status != pdPASS)
  {
    Error_Handler();
  }
}

static void StartDefaultTask(void *argument)
{
  (void)argument;

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(1000U));
  }
}

void vApplicationMallocFailedHook(void)
{
  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;

  taskDISABLE_INTERRUPTS();
  for (;;)
  {
  }
}
