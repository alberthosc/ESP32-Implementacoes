// Albertho Síziney Costa   10/02/2023
// Acionamento de relé por sensor de palmas e MQTT

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

#define RELAY_1 GPIO_NUM_0
#define AnalogIOSensor GPIO_NUM_32
#define DigitalIOSensor GPIO_NUM_34
bool RelayStatus = false;

static const char *TAG = "MQTT_TCP";

void wifi_connection(void);
static void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_app_start(void);

void status_of_sensor(void * params)
{
    int R;
    float analogValue;
    while (true)
    {
        analogValue = adc1_get_raw(ADC1_CHANNEL_4);    //Analogic value of sensor
        // Current value will be read and converted to voltage
        analogValue = analogValue * (5.0 / 1023.0);
        //... and outputted here
        printf("\nAnalog voltage value: %f V",analogValue);
        printf("\nExtreme value: ");
        if(gpio_get_level(DigitalIOSensor) == 1)
        {
            RelayStatus = !RelayStatus;
            printf(" reached");
            gpio_set_level(RELAY_1, RelayStatus);
            gpio_set_level(GPIO_NUM_2, RelayStatus);
        }
        printf("\n----------------------------------------------------------------");

        R = gpio_get_level(RELAY_1);
        if (R == 1){printf("Relay 1 -> ON!\n");}
        else if (R == 0){printf("Relay 1 -> OFF!\n");}
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    } 
}

void app_main(void)
{
    gpio_pad_select_gpio(RELAY_1);
    gpio_set_direction(RELAY_1, GPIO_MODE_INPUT_OUTPUT);
    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(DigitalIOSensor);
    gpio_set_direction(DigitalIOSensor, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(AnalogIOSensor);
    gpio_set_direction(AnalogIOSensor, GPIO_MODE_INPUT_OUTPUT);
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten( ADC1_CHANNEL_4, ADC_ATTEN_11db );

    
    xTaskCreatePinnedToCore(&status_of_sensor, "Status of sensor", 2048, NULL, 1, NULL,0);

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
        
        msg_id = esp_mqtt_client_subscribe(client, "TopicRelay", 2); //Incrição no tópico "TopicRelay"
        
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

        if(event->data[0] == '1' && event->data[1] == '1')
        {
            gpio_set_level(RELAY_1, 1);
            gpio_set_level(GPIO_NUM_2, 1);
        }
        else if(event->data[0] == '1' && event->data[1] == '0')
        {
            gpio_set_level(RELAY_1, 0);
            gpio_set_level(GPIO_NUM_2, 0);
        }

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
            /* .ssid = "Albertho",
            .password ="fprg2262",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, */
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