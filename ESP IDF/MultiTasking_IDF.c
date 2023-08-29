// Albertho Síziney Costa   24/11/2022
// ELE0629 - TOPICOS ESPECIAIS EM SISTEMAS EMBARCADOS - T01
// Atividade 2: Multitasking

/*
Desenvolva o código multitarefas com o FreeRTOS que realiza as seguintes ações:

Tarefa 1: Pisca um LED com o período ajustado através de uma variável recebida por uma fila (Queue)
vinda da Tarefa 3

Tarefa 2: Pisca um LED por 1 segundo, apenas quando a Tarefa 3 envia uma notificação

Tarefa 3: Realiza a leitura de uma entrada analógica (conectada a um potenciômetro) e utiliza o valor 
para configurar o tempo de delay (em ms) da Tarefa 1, enviando o valor de delay através de uma fila. 
O valor lido do ADC (0 - 4096) deve ser convertido em valores de 0 a 500 ms. Esta tarefa é executada 
a cada 3 segundos. Quando o valor do potenciômetro estiver em 0, uma notificação deve ser enviada para 
a Tarefa 2.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#define led_verde CONFIG_GPIO_OUTPUT_15
#define led_vermelho CONFIG_GPIO_OUTPUT_4
#define potenciometro CONFIG_GPIO_INPUT_2


xQueueHandle fila_de_delay;
xTaskHandle message;

void pisca_led_1(int params)
{
    while (true)
    {
        float millisegundos;

        if(xQueueReceive(fila_de_delay, &millisegundos, 5000/portTICK_PERIOD_MS))
        {

            printf("Led Verde -> Período: %f\n", millisegundos);
            digitalWrite(led_verde, HIGH); 
            vTaskDelay(millisegundos/portTICK_PERIOD_MS); 
            ESP_LOGI("Task 1", "Pisca LED verde");
            digitalWrite(led_verde, LOW); 

        }else{
            ESP_LOGE("Leitura", "Erro na leitura");
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    } 
}

void pisca_led_2(int params)
{
    while (true)
    {
        ulong resposta = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        printf("Notificação recebida %lu", resposta);

        digitalWrite(led_vermelho, HIGH); 
        vTaskDelay(1000/portTICK_PERIOD_MS); 
        ESP_LOGI("Task 1", "Pisca LED vermelho por 1s");
        digitalWrite(led_verde, LOW); 
        vTaskDelay(500 / portTICK_PERIOD_MS);
    } 
}

void leitura_e_envio(void * params)
{
    float millisegundos; 
    
    while (true)
    {
        millisegundos = = map(analogRead(potenciometro),0,4096,0,500);
        long resposta = xQueueSend(fila_de_delay, &millisegundos, 1000/portTICK_PERIOD_MS);
        printf("Resposta: %ld\n", resposta);
        if(resposta)
        {
            ESP_LOGI("Leitura", "periodo adicionada à fila");
        }else{
            ESP_LOGE("Leitura", "Falha no envio do periodo");
        }
        if(millisegundos == 0)
        {
            xTaskNotify(message, 10, eNoAction);
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    fila_de_delay = xQueueCreate(5, sizeof(float));

    xTaskCreate(&pisca_led_1, "Pisca LED verde", 2048, NULL, 1, NULL,0);    // (x,x,x,x,grau de importância menor,x,core 0 do processador)
    xTaskCreate(&pisca_led_2, "Pisca LED vermelho", 2048, NULL, 1, NULL,0); // (x,x,x,x,grau de importância menor,x,core 0 do processador)
    xTaskCreate(&leitura_e_envio, "Leitura do potenciometro", 2048, NULL, 2, NULL,1);   // (x,x,x,x,grau de importância maior,x,core 1 do processador)
}