// Albertho Síziney Costa   02/12/2022
// SISTEMAS EMBARCADOS
// Atividade: Aplicação com MQTT

// Link do drive com projeto completo: https://drive.google.com/file/d/128tLhNYqbjWcWF5lTrgvy0OSYF45T5iR/view?usp=share_link

/*
Criar uma solução de automação para controlar qualquer dispositivo no ESP32 via MQTT através de um celular 
(pode utilizar o app MQTT Tool) usando o broker de testes do Mosquitto. A solução precisa possuir ao menos 2 
tópicos mqtt: um para a publicação do status atual do dispositivo e outro para a ação de controle. O código deve 
obrigatoriamente ser desenvolvido em ESP-IDF, como mostra o exemplo mostrado em aula. */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_rom_gpio.h"
#include <driver/gpio.h>
#include <driver/adc.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "secrets.h"

#define led_azul GPIO_NUM_15
#define led_verde GPIO_NUM_2
static const char *TAG = "MQTT_TCP";

void wifi_connection(void);
static void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_app_start(void);

void status_of_blue_led(void * params)
{
    int status_led_azul;
    while (true)
    {
        status_led_azul = gpio_get_level(led_azul);
        if (status_led_azul == 1){printf("Led azul -> LIGADO!\n");}
        else if (status_led_azul == 0){printf("Led azul -> DESLIGADO!\n");}
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    } 
}

void status_of_green_led(void * params)
{
    int status_led_verde;
    while (true)
    {
        status_led_verde = gpio_get_level(led_verde);
        if (status_led_verde == 1){printf("Led verde -> LIGADO!\n");}
        else if (status_led_verde == 0){printf("Led verde -> DESLIGADO!\n");}
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    } 
}

void app_main(void)
{
    gpio_pad_select_gpio(led_azul);
    gpio_set_direction(led_azul, GPIO_MODE_INPUT_OUTPUT);

    gpio_pad_select_gpio(led_verde);
    gpio_set_direction(led_verde, GPIO_MODE_INPUT_OUTPUT);

    xTaskCreatePinnedToCore(&status_of_blue_led, "Aciona Led Azul", 2048, NULL, 1, NULL,0);
    xTaskCreatePinnedToCore(&status_of_green_led, "Aciona Led Verde", 2048, NULL, 1, NULL,1);

    nvs_flash_init();   // Flash memory initiation
    wifi_connection();  // WiFi connection
    
    mqtt_app_start();
    printf("MQTT Started!\n");
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;

    int msg_id;

    switch (event->event_id) {

    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(TAG, "MQTT EVENT CONNECTED");
        
        msg_id = esp_mqtt_client_subscribe(client, "Led_Verde", 2); //Incrição no tópico "Led_Verde"
        msg_id = esp_mqtt_client_subscribe(client, "Led_Azul", 2); //Incrição no tópico "Led_Azul"

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT EVENT DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT EVENT SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT EVENT UNSUBSCRIBED");
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT EVENT PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT EVENT DATA");
        printf("TOPIC = %.*s\r\n", event->topic_len, event->topic);
        printf("DATA = %.*s\r\n", event->data_len, event->data);

        //Turn On Blue Led
        if(event->topic[4] == 'A') //Nome do topico "Led_Azul", logo: event->topic[4] == 'A'
        {    
            if(event->data[0] == '1'){
                gpio_set_level(led_azul, 1);     
            }
            else if(event->data[0] == '0'){
                gpio_set_level(led_azul, 0);
            }
        }
        //Turn On Green Led
        if(event->topic[4] == 'V') //Nome do topico "Led_Verde", logo: event->topic[4] == 'V'
        {    
            if(event->data[0] == '1'){
                gpio_set_level(led_verde, 1);
            }
            else if(event->data[0] == '0'){
                gpio_set_level(led_verde, 0);
            }
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT EVENT ERROR");
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://test.mosquitto.org",
    };//mqtt://mqtt.eclipseprojects.io
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    printf("Starting MQTT... \n");
    esp_mqtt_client_start(client);
}

void wifi_connection(void)
{
    // 1 - WiFi LwIP Init Phase
    esp_netif_init();                       // TCP/IP initiation
    esp_event_loop_create_default();        // Event Loop
    esp_netif_create_default_wifi_sta();    // WiFi Station
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    // 2 - WiFi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "brisa-1776554",
            .password ="5qabth84",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration);
    // 3 - WiFi Start Phase
    esp_wifi_start();
    // 4 - Connect Phase
    esp_wifi_connect();
}

static void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("Wifi connecting...\n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("Wifi connected!\n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("Wifi disconnected!\n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("Wifi got IP!\n\n");
        break;                    
    default:
        printf("EVENT ID: %d\n", event_id);
        break;
    }
}