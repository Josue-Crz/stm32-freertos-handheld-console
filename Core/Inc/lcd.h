#ifndef LCD_H
#define LCD_H
#include "main.h"

// pack the i2c peripherial into a struct, access: i2c peripheral pin, address and backlight (essentially an object)
typedef struct{
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    uint8_t backlight;
} I2C_LCD_HandleTypeDef;

void LCDInit(I2C_LCD_HandleTypeDef *lcd, I2C_HandleTypeDef *hi2cX, uint8_t address);
void LCDSendCmd(I2C_LCD_HandleTypeDef *lcd, uint8_t cmd);
void LCDSendChar(I2C_LCD_HandleTypeDef *lcd, char data);
void LCDSendString(I2C_LCD_HandleTypeDef *lcd, char *str);
void LCDSetCursor(I2C_LCD_HandleTypeDef *lcd, uint8_t row, uint8_t col);
void LCDClear(I2C_LCD_HandleTypeDef *lcd);




#endif