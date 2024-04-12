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
#define LENGTH 20
double QRS_signal[LENGTH] = {2.19, 2.21, 2.22, 2.26, 2.29, 2.31, 2.33, 2.37, 2.38, 2.37, 
                            2.31, 2.25, 2.17, 2.08, 2.03, 1.99, 1.97, 1.96, 1.94, 1.94};
uint16_t data_in[LENGTH] = {0};
uint16_t threshold = 4750;
bool above_threshold = false;
#define SAMPLES 5
uint16_t beep_counter = SAMPLES;
int64_t beep_times[SAMPLES] = {0};

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
        //vTaskDelay(CONFIG_BLINK_PERIOD*5 / portTICK_PERIOD_MS);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
        // Get Data
        uint16_t raw_value = (adc_data[0] << 8) | adc_data[1];
        for (int i = 0; i < LENGTH-1; i++) {
            data_in[i] = data_in[i+1];
        }
        data_in[LENGTH-1] = raw_value;
        
        // Cross Correlation
        double sum = 0;
        for (int i = 0; i < LENGTH; i++) {
            sum += data_in[i] * QRS_signal[i];
        }
        uint16_t coorelated_value = (uint16_t)(sum/LENGTH);
        
        // Calculate Heartbeat
        if (above_threshold && coorelated_value < threshold) {
            above_threshold = false;
        } else if (!above_threshold && coorelated_value > threshold) {
            above_threshold = true;
            printf("beep ");

            for (int i = 0; i < SAMPLES-1; i++) {
                beep_times[i] = beep_times[i+1];
            }
            beep_times[SAMPLES-1] = esp_timer_get_time();

            double heartbeat = ((SAMPLES-1) / ((beep_times[SAMPLES-1] - beep_times[0]) / 1000000.0)) * 60;
            printf("%f\n", heartbeat);
        }
        //counter++;
        //if (counter > 5) {
        //    counter = 0;
            //printf("%d\n", coorelated_value);
            //printf("%d\n", raw_value);
        //}

        vTaskDelay(10 / portTICK_PERIOD_MS);
#endif
    }
}
