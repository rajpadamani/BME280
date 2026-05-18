/*
 * bme280.h
 *
 *  Created on: May 15, 2026
 *      Author: Raj
 */

#ifndef MAIN_BME280_H_
#define MAIN_BME280_H_



#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// Please update the following configuration according to your HardWare spec /////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define GPIO_HANDSHAKE      2
#define GPIO_MOSI           7
#define GPIO_MISO          	2
#define GPIO_SCLK           6
#define GPIO_CS             10

#define SENDER_HOST SPI2_HOST


/*BME280 register*/
#define BME280_HUM_LSB			0xFE
#define BME280_HUM_MSB			0xFD

#define BME280_TEMP_XLSB		0xFC
#define BME280_TEMP_LSB			0xFB
#define BME280_TEMP_MSB			0xFA

#define BME280_PRESS_XLSB		0xF9
#define BME280_PRESS_LSB		0xF8
#define BME280_PRESS_MSB		0xF7

#define BME280_CONFIG			0xF5
#define BME280_CTRL_MEAS		0xF4
#define BME280_STATUS			0xF3
#define BME280_CTRL_HUM			0xF2

#define BME280_RESET			0xE0

#define BME280_CHIP_ID          0xD0

#define BME280_COMPENS_TEMP_PRESS			0x88
#define BME280_COMPENS_HUM1			0xA1
#define BME280_COMPENS_HUMINITY		0xE1
//#define BME280_CALIB26			0xE1


/*Read write bit set*/
#define BME280_WRITE		0x7F
#define BME280_READ			0x80

/*Over sampling configuration*/
#define OVERSAMPLING_1		1
#define OVERSAMPLING_2		2
#define OVERSAMPLING_4		3
#define OVERSAMPLING_8		4

/*BME280 working modes*/
#define BME280_FORCED_MODE		2
#define BME280_NORMAL_MODE		3



/*BME280 register read, write functions*/
esp_err_t BME280_Reg_Read(uint8_t reg, size_t len,uint8_t *data,spi_device_handle_t handle);
esp_err_t BME280_Reg_Write(uint8_t reg, uint8_t data, spi_device_handle_t handle);


void BME_280_RESET(spi_device_handle_t handle);
void BME_280_READ_CHIP_IC(spi_device_handle_t handle);
void BME_280_Read_Compensate_value(spi_device_handle_t handle);
void BME_280_CONFIG(spi_device_handle_t handle);
void BME_280_Temperature_Final();
void BME_280_Pressure_Final();
void BME_280_Humidity_Final();
void BME_280_READ_RAW(spi_device_handle_t handle);








#endif /* MAIN_BME280_H_ */
