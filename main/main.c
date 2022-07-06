#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "wifi_functions.h"
#define VERSION			1.0
#define UPDATE_URL		"https://f002.backblazeb2.com/file/KaustubhKOTATest/blink-otav3.bin?Authorization=4_002f7b25ec2aea9000000000e_01a1a997_6b5372_acct_2BiLyRJWb6LR-ixAqCNOZySp80Y="
#define BLINK_GPIO 			GPIO_NUM_2
#define UPDATE_GPIO 		GPIO_NUM_3

// server certificates
extern const char server_cert_pem_start[] asm("_binary_certs_pem_start");
extern const char server_cert_pem_end[] asm("_binary_certs_pem_end");

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {

	switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)){
				printf("%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}

// Blink task
void blink_task(void *pvParameter) {

    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Check update task
// downloads every 30sec the json file with the latest firmware
void check_update_task(void *pvParameter) {

	while(1) {
		printf("Looking for a new firmware...\n");
		// configure the esp_http_client
		esp_http_client_config_t config = {
        .url = UPDATE_URL,
        .event_handler = _http_event_handler,
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);
		esp_err_t err = esp_http_client_perform(client);
		if(err == ESP_OK) {
			esp_http_client_config_t ota_client_config = {
					.url = UPDATE_URL,
					.cert_pem = server_cert_pem_start,
			};
			esp_err_t ret = esp_https_ota(&ota_client_config);
			if (ret == ESP_OK) {
				printf("OTA OK, restarting...\n");
				esp_restart();
			} else {
				printf("OTA failed...\n");
			}
		}
		// cleanup
		esp_http_client_cleanup(client);
		printf("\n");
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
	int out=1;
	printf("HTTPS OTA");
	// start the blink task
	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
	// connect to the wifi network
	wifi_initialise();
	wifi_wait_connected();
	printf("Connected to wifi network\n");
	// start the check update task
	if(out==1)
	{
		xTaskCreate(&check_update_task, "check_update_task", 8192, NULL, 5, NULL);
	}
}
