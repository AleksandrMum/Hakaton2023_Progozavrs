/* Simple HTTP + SSL Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"


#include "sdkconfig.h"
#include "RCSwitch.h"

/* A simple example that demonstrates how to create GET and POST
 * handlers and start an HTTPS server.
*/

#define CONFIG_RF_GPIO 26
#define MAX_SENSOR_FIELDS_SIZE 10

const char* STAG = "Sensors-info";

struct msg {
	unsigned long int msg_id;
	char id;
	char length;
	char* data;
};



typedef struct {
	char id;
	char* name;
	ssize_t record_size; // In bytes
	ssize_t buffer_size; // In records
	ssize_t buffer_point; // In records
	void* local_buffer;
} sensor;

typedef struct {
	sensor* sensor;	
    void* next_node;
} sensors;

sensors* SENSORS = {NULL, NULL};

void sensor_print(sensor* sensor)
{
	printf("Sensor %s\n", sensor->name);
	float* data = (float*)sensor->local_buffer;
	for(char j = 0; j < 10; j++)
	{
		for(char i = 0; i < sensor->record_size/sizeof(float); i++)
		{
			printf("%f ", data[j*sensor->record_size/sizeof(float) + i]);
		}
	}
}


// Data must be the letght of sensor record!
void add_record(sensor* sensor, void* data) 
{
	if(sensor->local_buffer == NULL)
	{
		printf("Can't add record, sensor does not initialized!");
		return;
	}
	if(sensor->buffer_point == sensor->buffer_size)
	{
		sensor->buffer_point = 0;
	}
	memcpy(sensor->local_buffer + sensor->buffer_point*sensor->record_size, data, sensor->record_size);	
	sensor->buffer_point++;
}

sensor* sensor_init(char* name, char id, ssize_t float_fields, ssize_t buffer_size)
{
	if(float_fields > MAX_SENSOR_FIELDS_SIZE || float_fields == 0)
	{
		ESP_LOGI(STAG, "Can't create sensor - fields required or too many");
		return NULL;
	}

	ESP_LOGI(STAG, "Can't create sensor - fields required or too many");
	sensor* ret = malloc(sizeof(sensor));
	ret->name = name;
	ret->id = id;
	ret->record_size = sizeof(float)*float_fields;
	ret->buffer_point = 0;
	ret->local_buffer = malloc(sizeof(float)*float_fields*buffer_size);
	ret->sd_card_buffer = NULL;		
	ESP_LOGI(STAG, "Create sensor %s:%d with %d fields and buffer %d",  name, id, float_fields, buffer_size);
	
	sensors* search = SENSORS;
	while(search->next_node != NULL)
	{
		search->sensor=ret;
		search->next_node = malloc(sizeof(sensors));
		search->next_node->sensor = NULL;
		search->next_node->next_node = NULL;
		search = search->next_node;
	}

	return ret;
}

void list_sensors(sensors* sensors)
{
	while(sensors->next_node != NULL)
	{
		printf("Sensor %s\n", sensors->sensor->name);

		sensors = sensors->next_node;
	}
}

void sensors_task(void* pvParameter)
{
	sensor* temp1 = sensor_init("Temp1", 0, 1, 10);
	sensor* hum = sensor_init("Hum", 0, 1, 10);
	float temp = 0.1;

	while(1) 
	{	
		temp = temp + 1;
		add_record(temp1, &temp);
		vTaskDelay(1000);
		list_sensors(SENSORS);
	}
}


static void tcp_client_task(void* pvParameters) // No parameters
{
	short port = 55; 	// Server port
	port = htons(port); // Translate to big endian
	
	int32_t listening_socket;
	listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in bind_info;
	bind_info.sin_addr.s_addr = (unsigned int)ip;
	bind_info.sin_port = port;
	bind_info.sin_family = AF_INET;

	int error = bind(listening_socket, (struct sockaddr*)&bind_info, sizeof(bind_info)); 
	if (error)
	{
		ESP_LOGI(SRV, "Socket unable to bind: errno %d", error);
		goto EXIT;
	}

	error = listen(listening_socket, 1);
	if (error)
	{
		ESP_LOGI("Error during listening: errono %d", error);
		goto EXIT;
	}
	ESP_LOGI(SRV, "Starting socket...");

	struct sockaddr_in accepted_address;
	accepted_address.sin_addr.s_addr = 0;
	accepted_address.sin_port = 0;
	accepted_address.sin_family = AF_INET;
	char hello[32]; // Message to recive 
	
	socklen_t addr_len = sizeof(accepted_address);
	while(1)
	{
		int accept_sock = accept(listening_socket, (struct sockaddr*)&accepted_address, &addr_len);		
		if(accept_sock < 0)
		{
			ESP_LOGI(SRV, "Failed to achive connection: errno %d", accept_sock);
			continue;
		}
		ESP_LOGI(SRV, "Achived connection from:");
		unsigned char* ip_point = (unsigned char*)&(accepted_address.sin_addr.s_addr);
		for (uint8_t i = 0; i < sizeof(accepted_address.sin_addr.s_addr); i++)
		{
			printf("%d.", ip_point[i]);
		}
		
		recv(accept_sock, hello, sizeof(hello), 0);
		printf("\nRecived string %s\n", hello);
		close(accept_sock);	
	}

EXIT:
	close(listening_socket);
	vTaskDelete(NULL);
}

static const char *TAG = "SensorStation";



void app_main(void)
{
	xTaskCreate(receiver, "Reciver", 4096, NULL, 5, NULL);  	
	xTaskCreate(sensors_init, "TEST SENSORS", 4096,  NULL, 5, NULL);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register event handlers to start server when Wi-Fi or Ethernet is connected,
     * and stop server when disconnection happens.
     */

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
#endif // CONFIG_EXAMPLE_CONNECT_WIFI

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
	//FIXME!! Priority!
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", (int)esp_get_free_heap_size());		
}
