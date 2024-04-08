/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#include <driver/spi_master.h>
#include "esp_timer.h"

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
//#define MASTER true

#define BLINK_GPIO GPIO_NUM_32
#define BLINK_GPIO2 GPIO_NUM_33

#define MOTOR_A1 GPIO_NUM_12
#define MOTOR_B1 GPIO_NUM_13


#ifdef MASTER
static uint8_t s_led_state = 0;

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
    gpio_set_level(BLINK_GPIO2, !s_led_state);
}
#else
// Define the SPI bus configuration
spi_bus_config_t bus_config = {
    .miso_io_num = 14, // MISO connected to GPIO14 (IO14)
    .mosi_io_num = -1, // Not used for ADC121
    .sclk_io_num = 13, // SCLK connected to GPIO13 (IO13)
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 0,
};

// Define the SPI device configuration
spi_device_interface_config_t dev_config = {
    .mode = 0, // SPI mode 0 (CPOL=0, CPHA=0)
    .clock_speed_hz = 1000000, // Set your desired clock speed (1MHz min for ADC121)
    .spics_io_num = 12, // CS connected to GPIO12 (IO12)
    .queue_size = 1,
};
#endif


void app_main(void)
{
    /* Configure the peripheral according to the LED type */
#ifdef MASTER
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    gpio_reset_pin(BLINK_GPIO2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);


    gpio_reset_pin(MOTOR_A1);
    gpio_reset_pin(MOTOR_B1);
    gpio_set_direction(MOTOR_A1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_B1, GPIO_MODE_OUTPUT);

#else
    esp_err_t ret;

    // Initialize the SPI bus
    ret = spi_bus_initialize(VSPI_HOST, &bus_config, 1);
    assert(ret == ESP_OK);

    // Add the SPI device
    spi_device_handle_t spi;
    ret = spi_bus_add_device(VSPI_HOST, &dev_config, &spi);
    assert(ret == ESP_OK);

    ESP_LOGI(TAG, "Example configured to ADC!");

    // Clean up - never really occurs
    //spi_bus_remove_device(spi);
    //spi_bus_free(VSPI_HOST);
#endif

    while (1) {
#ifdef MASTER
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();

        gpio_set_level(MOTOR_A1, s_led_state);
        gpio_set_level(MOTOR_B1, 0);

        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD*5 / portTICK_PERIOD_MS);
#else
        // Read data from ADC121
        uint8_t adc_data[2];
        spi_transaction_t trans = {
            .length = 16, // 12-bit ADC, so 16 bits
            .tx_buffer = NULL,
            .rx_buffer = adc_data,
        };
        ret = spi_device_transmit(spi, &trans);
        assert(ret == ESP_OK);

        //ESP_LOGI(TAG, "Raw ADC Data: %02X %02X", adc_data[0], adc_data[1]);
        uint16_t raw_value = (adc_data[0] << 8) | adc_data[1];
        printf("%u\n", raw_value);

        // Process the ADC data (convert to voltage, etc.)
        // ...

        vTaskDelay(10 / portTICK_PERIOD_MS);
#endif
    }
}
