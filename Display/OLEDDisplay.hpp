//=========================================================================  
//  OLEDDisplay.h                                                          
//  Header file for OLEDDisplay class.                                     
//  Provides initialization and control of SSD1306 OLED displays over I²C. 
//=========================================================================  

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "pico/stdlib.h"                                                     // Raspberry Pi Pico SDK
#include "hardware/i2c.h"                                                    // I²C interface
#include <cstdint>                                                           // Fixed-width integer types
#include <cstring>                                                           // For memcpy / memset

//-------------------------------------------------------------------------
//  Hardware Configuration                                                  
//-------------------------------------------------------------------------
constexpr uint8_t SSD1306_I2C_ADDR      = 0x3C;                              // Default I²C address
constexpr uint32_t SSD1306_I2C_CLK      = 400 * 1000;                        // I²C clock speed (400 kHz)
constexpr uint16_t SSD1306_WIDTH        = 128;                               // Display width in pixels
constexpr uint16_t SSD1306_HEIGHT       = 32;                                // Display height in pixels

//-------------------------------------------------------------------------
//  SSD1306 Command Set (from datasheet)                                    
//-------------------------------------------------------------------------
constexpr uint8_t SSD1306_SET_MEM_MODE          = 0x20;
constexpr uint8_t SSD1306_SET_COL_ADDR          = 0x21;
constexpr uint8_t SSD1306_SET_PAGE_ADDR         = 0x22;
constexpr uint8_t SSD1306_SET_HORIZ_SCROLL      = 0x26;
constexpr uint8_t SSD1306_SET_SCROLL            = 0x2E;
constexpr uint8_t SSD1306_SET_DISP_START_LINE   = 0x40;
constexpr uint8_t SSD1306_SET_CONTRAST          = 0x81;
constexpr uint8_t SSD1306_SET_CHARGE_PUMP       = 0x8D;
constexpr uint8_t SSD1306_SET_SEG_REMAP         = 0xA0;
constexpr uint8_t SSD1306_SET_ENTIRE_ON         = 0xA4;
constexpr uint8_t SSD1306_SET_ALL_ON            = 0xA5;
constexpr uint8_t SSD1306_SET_NORM_DISP         = 0xA6;
constexpr uint8_t SSD1306_SET_INV_DISP          = 0xA7;
constexpr uint8_t SSD1306_SET_MUX_RATIO         = 0xA8;
constexpr uint8_t SSD1306_SET_DISP              = 0xAE;
constexpr uint8_t SSD1306_SET_COM_OUT_DIR       = 0xC0;
constexpr uint8_t SSD1306_SET_DISP_OFFSET       = 0xD3;
constexpr uint8_t SSD1306_SET_DISP_CLK_DIV      = 0xD5;
constexpr uint8_t SSD1306_SET_PRECHARGE         = 0xD9;
constexpr uint8_t SSD1306_SET_COM_PIN_CFG       = 0xDA;
constexpr uint8_t SSD1306_SET_VCOM_DESEL        = 0xDB;

constexpr uint8_t SSD1306_PAGE_HEIGHT           = 8;
constexpr uint8_t SSD1306_NUM_PAGES             = SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT;
constexpr uint16_t SSD1306_BUF_LEN              = SSD1306_NUM_PAGES * SSD1306_WIDTH;

//-------------------------------------------------------------------------
//  OLEDDisplay class                                                      
//  Encapsulates all functionality for communicating with SSD1306 OLED.    
//-------------------------------------------------------------------------
class OLEDDisplay {
public:
    OLEDDisplay(i2c_inst_t* i2c, uint8_t addr = SSD1306_I2C_ADDR,
                uint width = SSD1306_WIDTH, uint height = SSD1306_HEIGHT);   // Constructor

    void init();                                                             // Initialize display
    void clear();                                                            // Clear display buffer
    void writeText(int x, int y, const char* text);                          // Write text string
    void render();                                                           // Send buffer to OLED
    void invert(bool on);                                                    // Invert display colors

private:
    void sendCommand(uint8_t cmd);                                           // Send one command
    void sendCommandList(const uint8_t* cmds, int count);                    // Send command list
    void sendBuffer(uint8_t* buf, int len);                                  // Send data buffer
    void writeChar(int x, int y, char c);                                    // Write a single character

    i2c_inst_t* _i2c;                                                        // I²C instance (i2c0 / i2c1)
    uint8_t _addr;                                                           // I²C address
    uint _width;                                                             // Display width
    uint _height;                                                            // Display height
    uint8_t* _buffer;                                                        // Frame buffer pointer
};

#endif
