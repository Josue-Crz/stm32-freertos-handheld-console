/*
    # Author: Josue Cruz
    # Date Created: June 25th, 2026

    # Description: I2C LCD 1602 driver
    # Relevant Pin layout
    # PA8 - SCL Pin
    # PC9 - SDA Pin
    # note: intended to be compatible across bare metal and RTOS applications
*/
// Libraries
#include "lcd.h"
#include "cmsis_os.h"


// 8 bit shift locations
// based off the schematic and datasheet for the Hitachi HD44780U datasheet
// Shifting to the respective data, r/w operation, enabling latch pin and backlight bit
#define PIN_RS (1 << 0)
#define PIN_RW (1 << 1)
#define PIN_E (1 << 2)
#define BACKLIGHT (1 << 3)



static void LCDDelay(uint32_t ms){
    // getting a snapshot the task scheduler and identifying if its running
    if(osKernelGetState() == osKernelRunning){
        osDelay(ms);
    }
    else{ // heurisitic fall back for starting mcu, though stalls entire clock
        HAL_Delay(ms);
    }
}

static void LCDWriteByte(I2C_LCD_HandleTypeDef *lcd, uint8_t data){
    HAL_I2C_Master_Transmit(lcd->hi2c, lcd->address, &data, 1, HAL_MAX_DELAY); // wait as long as need for HAL_MAX_DELAY
}

// latch onto the pin's clock to enable
static void LCDPulseEnable(I2C_LCD_HandleTypeDef *lcd, uint8_t data){
    LCDWriteByte(lcd, data | PIN_E); // set to respective pin
    LCDDelay(1); // 1 millisecond to latch
    LCDWriteByte(lcd, data & ~PIN_E); // sets off
    LCDDelay(1);
}

static void LCDSend4Bits(I2C_LCD_HandleTypeDef *lcd, uint8_t nibble, uint8_t mode){
    uint8_t data;
    data = (nibble & 0xF0) | mode | lcd->backlight;

    LCDWriteByte(lcd, data);
    LCDPulseEnable(lcd, data);
}

static void LCDSend(I2C_LCD_HandleTypeDef *lcd, uint8_t value, uint8_t mode){
    LCDSend4Bits(lcd, value & 0xF0, mode);
    LCDSend4Bits(lcd, (value << 4) & 0xF0, mode);
}

void LCDSendCmd(I2C_LCD_HandleTypeDef *lcd, uint8_t cmd){
    LCDSend(lcd, cmd, 0x00);
    LCDDelay(2);
}

void LCDSendChar(I2C_LCD_HandleTypeDef *lcd, char data){
    LCDSend(lcd, (uint8_t)data, PIN_RS); // turning RS pin on  via sending a byte of memory address associated with pin
    LCDDelay(1);
}

void LCDSendString(I2C_LCD_HandleTypeDef *lcd, char *str){
    while(*str){ // runs until null terminator
        LCDSendChar(lcd, *str++);
    }
}

void LCDClear(I2C_LCD_HandleTypeDef *lcd){
    LCDSendCmd(lcd, 0x01); // data sent of 0x01 clears screen as nibbles are sent through i2c peripheral 
    LCDDelay(2);
}

void LCDSetCursor(I2C_LCD_HandleTypeDef *lcd, uint8_t row, uint8_t col){
    uint8_t address;

    if(row == 0){
        address = 0x00 + col;
    }
    else{
        address = 0x40 + col;
    }

    LCDSendCmd(lcd, 0x80 | address);
}

void LCDInit(I2C_LCD_HandleTypeDef *lcd, I2C_HandleTypeDef *hi2cX, uint8_t address){
    lcd->hi2c = hi2cX;
    lcd->address = address << 1;
    lcd->backlight = BACKLIGHT;

    LCDDelay(50); // give time prior to boot up, subsequent delays are significantly smaller

    LCDSend4Bits(lcd, 0x30, 0);
    LCDDelay(5);

    LCDSend4Bits(lcd, 0x30, 0);
    LCDDelay(1);

    LCDSend4Bits(lcd, 0x30, 0);
    LCDDelay(1);

    LCDSend4Bits(lcd, 0x20, 0);
    LCDDelay(1);

    LCDSendCmd(lcd, 0x28);
    LCDSendCmd(lcd, 0x08);
    LCDClear(lcd);
    LCDSendCmd(lcd, 0x06);
    LCDSendCmd(lcd, 0x0C);
}