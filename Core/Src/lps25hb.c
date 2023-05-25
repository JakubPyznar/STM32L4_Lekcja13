#include "lps25hb.h"
#include "i2c.h"

//adres czujnika
#define LPS25HB_ADDR        	0xBA
//adresy poszczegolnych rejestrow czujnika
#define LPS25HB_WHO_AM_I 		0x0F
#define LPS25HB_CTRL_REG1 		0x20
#define LPS25HB_CTRL_REG2 		0x21
#define LPS25HB_CTRL_REG3 		0x22
#define LPS25HB_CTRL_REG4 		0x23
#define LPS25HB_PRESS_OUT_XL 	0x28
#define LPS25HB_PRESS_OUT_L 	0x29
#define LPS25HB_PRESS_OUT_H 	0x2A
#define LPS25HB_TEMP_OUT_L 		0x2B
#define LPS25HB_TEMP_OUT_H 		0x2C
#define LPS25HB_RPDS_L 			0x39
#define LPS25HB_RPDS_H 			0x3A
#define LPS25HB_FIFO_CTRL		0x2E
//timeout dla komunikacji przez I2C
#define TIMEOUT                 100

//funkcja do odczytywania wartosci z wybranego rejestru czujnika przez I2C
static uint8_t lps_read_reg(uint8_t reg)
{
	uint8_t value = 0;
	HAL_I2C_Mem_Read(&hi2c1, LPS25HB_ADDR, reg, 1, &value, 1, TIMEOUT);
	return value;
}

//funkcja do zapisu wartosci do wybranego rejestru czujnika przez I2C
static void lps_write_reg(uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(&hi2c1, LPS25HB_ADDR, reg, 1, &value, 1, TIMEOUT);
}

//sprawdzenie czy czujnik sie komunikuje
//wybudzenie i ustawienie czestotliwosci pomiarow cisnienia i temp. na 25Hz (max czestotliwosc)
//ustawienie usredniania odczytow za 32 probki (w kolejce FIFO)
HAL_StatusTypeDef lps25hb_init(void)
{
	if (lps_read_reg(LPS25HB_WHO_AM_I) != 0xBD)
		return HAL_ERROR;
	else
	{
		lps_write_reg(LPS25HB_CTRL_REG1, 0xC0);
		lps_write_reg(LPS25HB_CTRL_REG2, 0x40);
		lps_write_reg(LPS25HB_FIFO_CTRL, 0xDF);
		return HAL_OK;
	}
}

//odczyt temperatury, przez dodanie jedynki na poczatku adresu rejestru czujnik zwroci wartosci
//dwoch kolejnych rejestrow czyli w tym przypadku TEMP_OUT_L i TEMP_OUT_H
float lps25hb_read_temp(void)
{
	int16_t temp;
	if (HAL_I2C_Mem_Read(&hi2c1, LPS25HB_ADDR, LPS25HB_TEMP_OUT_L | 0x80, 1, (uint8_t*)&temp, sizeof(temp), TIMEOUT) != HAL_OK)
		Error_Handler();

	return 42.5f + temp/480.0f;
}

//odczyt cisnienia w postaci 3 bajtow do zmiennej int32_t, dlatego wymagana jest inicjalizacja zmiennej zerem
//zeby na najstarszym bajcie nie zostal syf, bo tylko 3 bajty sa nadpisywane
float lps25hb_read_pressure(void)
{
	int32_t pressure = 0;
	if (HAL_I2C_Mem_Read(&hi2c1, LPS25HB_ADDR, LPS25HB_PRESS_OUT_XL | 0x80, 1, (uint8_t*)&pressure, 3, TIMEOUT) != HAL_OK)
		Error_Handler();

	return pressure / 4096.0f;
}

//kalibracja czujnika cisnienia
void lps25hb_set_calib(uint16_t value)
{
	lps_write_reg(LPS25HB_RPDS_L, value);
	lps_write_reg(LPS25HB_RPDS_H, value >> 8);
}



