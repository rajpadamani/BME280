/*
 * bme2880.c
 *
 *  Created on: May 15, 2026
 *      Author: Raj
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "bme280.h"

/*BME Raw value global*/
volatile int temperature_raw, pressure_raw, humidity_raw = 0;

float finaltemp,finalpressure,final_humidity = 0;

/*Final values in float*/
float Temperature, Pressure, Humidity = 0;

int32_t t_fine;

/*Compensation values*/
unsigned short dig_T1, dig_P1 = 0;
signed short dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
unsigned char dig_H1, dig_H3;
signed char dig_H6;


esp_err_t BME280_Reg_Read(uint8_t reg, size_t len,uint8_t *data,spi_device_handle_t handle)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	esp_err_t spi_read_ret;

	reg = reg | BME280_READ;
	t.length = 8 + (len * 8);
	t.tx_buffer = &reg;
	t.rx_buffer = data;
		
	spi_read_ret = spi_device_transmit(handle, &t);

	return spi_read_ret;
}

esp_err_t BME280_Reg_Write(uint8_t reg, uint8_t data, spi_device_handle_t handle)
{
	spi_transaction_t t;
	memset(&t,0,sizeof(t));
	esp_err_t spi_write_ret;
	
	reg = reg & BME280_WRITE;
	uint8_t data_write[2] = {reg,data};
	
	t.length = 16;
	t.tx_buffer = &data_write;
	t.rx_buffer = NULL;
	
	spi_write_ret = spi_device_transmit(handle,&t);
	
	return spi_write_ret;
}



void BME_280_RESET(spi_device_handle_t handle)
{
	BME280_Reg_Write(BME280_RESET,0xB6,handle);
}

void BME_280_READ_CHIP_IC(spi_device_handle_t handle)
{
	uint8_t chip_id[2] = {0};
	BME280_Reg_Read(BME280_CHIP_ID, 1, chip_id, handle);
	printf("BME280 CHIP ID: %x\n\r",chip_id[1]);
}

void BME_280_Read_Compensate_value(spi_device_handle_t handle)
{
	uint8_t comp[30] = {0};
	BME280_Reg_Read(BME280_COMPENS_TEMP_PRESS,24,comp,handle);
	
	dig_T1 = comp[1] | (comp[2]<<8);
	dig_T2 = comp[3] | (comp[4]<<8);
	dig_T3 = comp[5] | (comp[6]<<8);
	
	dig_P1 = comp[7] | (comp[8]<<8);
	dig_P2 = comp[9] | (comp[10]<<8);
	dig_P3 = comp[11] | (comp[12]<<8);
	dig_P4 = comp[13] | (comp[14]<<8);		 
	dig_P5 = comp[15] | (comp[16]<<8);	
	dig_P6 = comp[17] | (comp[18]<<8);
	dig_P7 = comp[19] | (comp[20]<<8);
	dig_P8 = comp[21] | (comp[22]<<8);
	dig_P9 = comp[23] | (comp[24]<<8);
	
	BME280_Reg_Read(BME280_COMPENS_HUM1, 1, comp, handle);
	
	dig_H1 = comp[1];
	
	BME280_Reg_Read(BME280_COMPENS_HUMINITY, 8, comp, handle);
	dig_H2 = comp[1] | (comp[2]<<8);
	dig_H3 = comp[3];
	dig_H4 = (comp[4]<<4) | (comp[5] & 0x0F);
	dig_H5 = (comp[6] & 0x0F) | (comp[7]<<4); 
	dig_H6 = comp[8];
	
	printf("Humidity compensate value: dig_H1:%X,\tdig_H2:%X,\tdig_H3:%X,\tdig_H4:%X,\tdig_H5:%X,\tdig_H6:%x\n\r",dig_H1,dig_H2,dig_H3,dig_H4,dig_H5,dig_H6);
	
}



void BME_280_CONFIG(spi_device_handle_t handle)
{
	uint8_t config = 0;
	
	config = (OVERSAMPLING_2 << 5) | (OVERSAMPLING_2 << 2) | BME280_NORMAL_MODE;
	printf("BME280_CTRL_MEAS value: %X\n\r",config);
	BME280_Reg_Write(BME280_CTRL_MEAS, config, handle);
	
	config = 0x80;
	BME280_Reg_Write(BME280_CONFIG,config,handle);
	
	config = OVERSAMPLING_1;
	BME280_Reg_Write(BME280_CTRL_HUM, config, handle);
}

void BME_280_Temperature_Final()
{
	int var1, var2;
	var1 = ((((temperature_raw>>3) - ((int)dig_T1<<1))) * ((int)dig_T2)) >> 11;
	var2 = (((((temperature_raw>>4)-((int)dig_T1)) * ((temperature_raw >> 4) - ((int)dig_T1)))>>12) * ((int)dig_T3))>>14;
	t_fine = var1 + var2;
	
	Temperature = (t_fine * 5 + 128) >> 8;
	Temperature = Temperature/100;
	printf("Final Temperature: %f\n\r",Temperature);
	
}

void BME_280_Pressure_Final()
{
	int64_t var1,var2,p;
	/*Pressure conversion*/
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1 * (int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1)) * ((int64_t)dig_P1)>>33;
	
	if(var1 == 0)
	{
		Pressure = 0;
	}
	else {
		p = 1048576 - pressure_raw;
		p = (((p<<31)-var2)*3125)/var1;
		var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13))>>25;
		var2 = (((int64_t)dig_P8) * p) >> 19;
		p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
		Pressure = ((uint32_t)p);
		Pressure = Pressure / 256.0;
	}
	printf("Final Pressure: %f\n\r",Pressure);	
}

void BME_280_Humidity_Final()
{
	int32_t v_x1_u32r;
	
	v_x1_u32r = (t_fine-((int32_t)76800));
	
	v_x1_u32r = (((((humidity_raw << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * 
	v_x1_u32r)) + ((int32_t)16384))>>15) * (((((((v_x1_u32r * 
	((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +
	((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 
	8192) >> 14));
	
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
		((int32_t)dig_H1)) >> 4));
		
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	Humidity = (float)(v_x1_u32r>>12) / 1024;
	printf("Final Humidity: %f\n\r",Humidity);
}


void BME_280_READ_RAW(spi_device_handle_t handle)
{
	uint8_t raw_value[9] = {0};
//	BME_280_CONFIG(handle);
	printf("\n\r");
	BME280_Reg_Read(BME280_PRESS_MSB, 8, raw_value, handle);
	printf("Pressure raw value:    %X	%X	%X\n\r",raw_value[1],raw_value[2],raw_value[3]);
	printf("Temperature raw value: %X	%X	%X\n\r",raw_value[4],raw_value[5],raw_value[6]);
	printf("Humidity raw value:    %X	%X\n\r",raw_value[7],raw_value[8]);
	
	pressure_raw = (raw_value[1]<<12) | (raw_value[2]<< 4) | (raw_value[3]>>4);
	temperature_raw = (raw_value[4]<<12) | (raw_value[5]<< 4) | (raw_value[6]>>4);
	humidity_raw = (raw_value[7]<<8) | (raw_value[8]);
	printf("pressure_raw:%X\n\r",pressure_raw);
	printf("temperature_raw:%X\n\r",temperature_raw);
	printf("humidity_raw:%X\n\r",humidity_raw);
}