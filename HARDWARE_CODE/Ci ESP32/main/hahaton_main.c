#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "sdkconfig.h"
#include "RCSwitch.h"

#define CONFIG_RF_GPIO 26
#define MAX_SENSOR_FIELDS_SIZE 10
#define MAXIMUM_WIFI_RETRYES  20
#define EXAMPLE_ESP_MAXIMUM_RETRY  20
#define TRANSMIT_BIT_LENGTH 32

#define TEMPERATURE 26

// Wifi
#define SSID      "kapitan"
#define PASS     	"rfgbnfy55" 
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "RECEIVER";
static const char *SRV = "SERVER";
static const char *STAG = "Sensors-info";

#define SENSORS_COUNT 2 

typedef struct {
	char id;
	char* name;
	ssize_t record_size; // In bytes
	ssize_t buffer_size; // In records
	ssize_t buffer_point; // In records
	void* local_buffer;
} sensor;

typedef struct {
	char hash;
	char id; 
	char message[TRANSMIT_BIT_LENGTH/8 - 2]; 
}flow_node;


sensor* SENSORS[SENSORS_COUNT] = {NULL, NULL};//, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

sensor* sensor_init(char* name, char id, ssize_t float_fields, ssize_t buffer_size)
{
	sensor* ret = malloc(sizeof(sensor));
	ret->name = name;
	ret->id = id;
	ret->record_size = sizeof(float)*float_fields;
	ret->buffer_point = 0;
	ret->local_buffer = malloc(sizeof(float)*float_fields*buffer_size);
	ESP_LOGI(STAG, "Create sensor %s:%d with %d fields and buffer %d",  name, id, float_fields, buffer_size);
	return ret;	
}



static int s_retry_num = 0;
static unsigned int ip; // Must be set after connecting to the wifi

// WIFI connection
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
        esp_wifi_connect();
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
        if (s_retry_num < MAXIMUM_WIFI_RETRYES) 
		{
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
		ESP_LOGI(SRV, "Starting TCP/IP SERVER");
		ip = event->ip_info.ip.addr;
        
		s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
		},
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 SSID, PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 SSID, PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

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


unsigned char hash(unsigned long int x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}


void update_flow(flow_node* node, unsigned char hash, unsigned char id, char counter, unsigned char data[TRANSMIT_BIT_LENGTH/8 - 2])
{		 

	if(hash != node->hash && node->hash!=0)
	{
		printf("Hash do not match: %d %d. Fill node\n", hash, node->hash);

	}

	int i = 0;	
	while(i < SENSORS_COUNT)
	{
		if(id == SENSORS[i]->id)
			break;
		i++;
	}	
	
	printf("Find sensor %s\n", SENSORS[i]->name);
	
	if(counter)
	{
		printf("End of msg\n");
		char write_data[sizeof(data)*2];

		memcpy(write_data, node->message, 2);
		memcpy(write_data + 2, data, 2);
		
		add_record(SENSORS[i], write_data); 
		sensor_print(SENSORS[i]);
	}	
	else
	{
		memcpy(node->message, data, 2);
		node->hash = hash;
	}

}


void msg_handler(flow_node* messages[SENSORS_COUNT],  char msg[TRANSMIT_BIT_LENGTH/8])
{
	unsigned char hashed = msg[TRANSMIT_BIT_LENGTH/8 - 1];
	long int val;
	memcpy(&val, msg, TRANSMIT_BIT_LENGTH/8 - 1);
	unsigned char test_hash = hash(val);	
	unsigned char id = msg[0] & 0x3F; // 11 is counter, 111111 is id
	unsigned char counter; 
	counter = msg[0] & 0xC0;
	counter = counter >> 6;
	unsigned char data[2];
	memcpy(data, msg+1, TRANSMIT_BIT_LENGTH/8 - 2); // Two bytes of data
	printf("hash: %d %d\n id %d \n msg_num %d\n data %02x %02x\n", hashed, test_hash, id, counter, data[0], data[1]);

	for(int i = 0; i < SENSORS_COUNT; i++)
	{
		if(messages[i]->id == id)
		{
			update_flow(messages[i], hash, id, counter, data);
		}
	}
			

}



long int swapped(long int num) 
{
	return ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0xff0000) | // move byte 1 to byte 2
                    ((num>>8)&0xff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3
}



void receiver(void* pvParameter)
{

	ESP_LOGI(TAG, "Start receiver");
	RCSWITCH_t RCSwitch;
	setProtocol(&RCSwitch, 1);
	initSwich(&RCSwitch);
	enableReceive(&RCSwitch, CONFIG_RF_GPIO);

	flow_node* messages[SENSORS_COUNT]; 
		
	for(int j = 0; j < SENSORS_COUNT; j++)
	{
		messages[j] =(flow_node*)malloc(sizeof(flow_node));
		messages[j]->id = SENSORS[j]->id;
		messages[j]->hash=0;	
	}



	// Array for packet dublicates 
	long int previos_msgs[10];
	for (int i = 0; i < 10; i++)
	{
		previos_msgs[i] = 0;
	}	
	// Create flow nodes for sensors 
	
	//for (int j = 0; j < SENSORS_COUNT; j++);
	//{
	//	messages[j] = malloc(sizeof(flow_node)); 
	//	messages[j]-> id = SENSORS[j]->id;
	//	messages[j]->hash = 0;
	//}	
		



	long int rcv_value;   	
	int counter = 0; 
	char msg[TRANSMIT_BIT_LENGTH/8];

	while(1) {
			
		if (available(&RCSwitch)) {
			//if(getReceivedBitlength(&RCSwitch) != TRANSMIT_BIT_LENGTH)
			//	continue;
			rcv_value = getReceivedValue(&RCSwitch);
			for(int i = 0; i < counter; i++)
			{
				if( previos_msgs[i] == rcv_value) 
				{
					printf("Packet %d alredy arrived\n", (int)rcv_value);
					goto exit;
				}
			}
			previos_msgs[counter] = rcv_value;
			if(counter == 9)
				counter = 0;
			else
				counter++;

			rcv_value = swapped(rcv_value);
			memcpy(msg, &rcv_value, TRANSMIT_BIT_LENGTH/8);	
			
			printf("Recived ");
			for(int i = 0; i < TRANSMIT_BIT_LENGTH/8; i++)
				printf(" %02hhx", (unsigned char)msg[i]);
			printf("\n");
			msg_handler(messages, msg);
exit:	
			resetAvailable(&RCSwitch);
		} else {
			vTaskDelay(1);
		}
	} 
}


void app_main(void)
{
	// Create sensors
	SENSORS[0] = sensor_init("TEMPERATURE1", 26, 1, 1000);
	SENSORS[1] = sensor_init("FUCKCHIK", 27, 1, 1000);
	xTaskCreate(receiver, "Reciver", 4096, NULL, 5, NULL);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();
			
}
