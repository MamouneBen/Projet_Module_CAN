//Brief: code receive langage: ESPIDF, session automne 


#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void receive_can_task(void *pvParameter) {
    // Initialize CAN configuration
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_0, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();


    // Install CAN driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        printf("Failed to install CAN driver\n");
        vTaskDelete(NULL);
    }


    // Start CAN driver
    if (twai_start() != ESP_OK) {
        printf("Failed to start CAN driver\n");
        vTaskDelete(NULL);
    }


    while (1) {
        twai_message_t message;


        // Receive CAN message
        if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
            printf("Message received\n");


            // Process received message
            if (message.extd) {
                printf("Message is in Extended Format\n");
            } else {
                printf("Message is in Standard Format\n");
            }
            printf("ID is %ld\n\r", message.identifier);
            for (int i = 0; i < message.data_length_code; i++) {
                printf("%c", (char)message.data[i]);
            }
            printf("\n");
        } else {
            printf("Failed to receive message\n");
        }
    }
}


void app_main() {
    xTaskCreate(receive_can_task, "receive_can_task", 2048, NULL, 5, NULL);
}