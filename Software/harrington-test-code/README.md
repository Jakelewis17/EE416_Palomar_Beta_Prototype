The following is the only code that matters out of the main.c file for ECG


// Some of the includes may not be needed as this is in ESP-IDF
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#include <driver/spi_master.h>
#include "esp_timer.h"

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



void app_main(void)
{
    esp_err_t ret;

    // Initialize the SPI bus
    ret = spi_bus_initialize(VSPI_HOST, &bus_config, 1);
    assert(ret == ESP_OK);

    // Add the SPI device
    spi_device_handle_t spi;
    ret = spi_bus_add_device(VSPI_HOST, &dev_config, &spi);
    assert(ret == ESP_OK);

    // Clean up - never really occurs
    //spi_bus_remove_device(spi);
    //spi_bus_free(VSPI_HOST);


    while (1) {
        // Read data from ADC121
        uint8_t adc_data[2];
        spi_transaction_t trans = {
            .length = 16, // 12-bit ADC, so 16 bits
            .tx_buffer = NULL,
            .rx_buffer = adc_data,
        };
        ret = spi_device_transmit(spi, &trans);
        assert(ret == ESP_OK);

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
    }
}