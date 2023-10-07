


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
		ESP_LOGI(SRV, "Error during listening: errono %d", error);
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
