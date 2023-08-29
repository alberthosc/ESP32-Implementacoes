#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"


void task1 (void * params)
{
    while (true)
    {
        ESP_LOGI("Task1", "Leitura de sensores");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task2 (void * params)
{
    while(true)
    {
        ESP_LOGI("Task2", "Escrever no display");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    xTaskCreate(&task1, "Leitura", 2048, "Task 1", 0, NULL);
    xTaskCreatePinnedToCore(&task2, "Display", 2048, "Task 2", 0, NULL, 1);
}