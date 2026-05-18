/* SPI Slave example, sender (uses SPI master driver)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/_intsup.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "bme280.h"

//Main application
void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t handle;
    
//    memset(&t, 0, sizeof(t));

    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    //Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 1000000,
        .duty_cycle_pos = 128,      //50% duty cycle
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .cs_ena_posttrans = 3,      //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .queue_size = 3
    };

    //Initialize the SPI bus and add the device we want to send stuff to.
    ret = spi_bus_initialize(SENDER_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
    ret = spi_bus_add_device(SENDER_HOST, &devcfg, &handle);
    assert(ret == ESP_OK);

	BME_280_RESET(handle);
	BME_280_READ_CHIP_IC(handle);
	
	BME_280_CONFIG(handle); //NOTE: no need for forced mode
	BME_280_Read_Compensate_value(handle);
	
//	ESP_LOGE("MAIN", "new firmware");

    while (1) {
		
		BME_280_READ_RAW(handle);
		BME_280_Temperature_Final();
		BME_280_Pressure_Final();
		BME_280_Humidity_Final();
		vTaskDelay(pdMS_TO_TICKS(550));
		
    }

    //Never reached.
    ret = spi_bus_remove_device(handle);
    assert(ret == ESP_OK);
}
