#include <Arduino.h>

#include "utils.h"
#include "gpio.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "nrf24.h"
#include "oled.h"

#ifndef spipinconfig
#define spipinconfig
#define sck   13
#define miso  12
#define mosi  11
#define freq  2000
#endif 

#ifndef nrfpinconfig
#define nrfpinconfig
#define chipselect  10
#define chipenable  9
#define channel     4
#endif 

#ifndef flamepinconfig
#define flamepinconfig
#define flamesensor A0
#endif 

#ifndef alarmpinconfig
#define alarmpinconfig
#define alarm       A1
#endif

#ifndef smokepinconfig
#define smokepinconfig
#define smokesensor A2
#endif 

#ifndef oledpinconfig
#define oledpinconfig
#define oleddata    A4
#define oledclock   A5
#define oledaddress 0x78
#define oledfreq    400
#endif 

const char *lattitude = "9.9187";
const char *longitude = "78.1147";
uint8_t address[] = "PIPE1";

uint8_t flamelevel, smokelevel;
uint8_t nrfarray[32], nrfcount;
bool isflamedetected;

void setup()
{
  pinMode(alarm, OUTPUT);
  digitalWrite(alarm, LOW);

  //adc_initialize();
  spi_initialize(sck, miso, mosi);
  oled_initialize(oledfreq);

  oled_display_cursor(0, 0);
  oled_print("AUTONOMOUS ROBOT");
  oled_print("FOR FOREST FIRE ");
  oled_print("DETECT & QUENCH ");
  oled_print("ROVER USING STM ");
  oled_display(); delay(2500);
  oled_screen_clear();

  oled_display_cursor(0, 0);
  if(nrf_initialize(chipselect, chipenable, 90)) oled_print("NRF found");
  else oled_print("NRF not found");
  nrf_txr_address(address);
  oled_display(); delay(500);
  oled_screen_clear();
}

void loop()
{
  flamelevel = map(analogRead(flamesensor), 0, 1023, 100, 0);
  smokelevel = map(analogRead(smokesensor), 0, 1023, 0, 100);

  oled_display_cursor(0, 0);
  oled_print(lattitude);
  oled_write(',');
  oled_print(longitude);

  oled_display_cursor(0, 8);
  oled_print("F:");
  oled_decimal(flamelevel, 3, DEC);
  oled_write('%');

  oled_display_cursor(64, 8);
  oled_print("S:");
  oled_decimal(smokelevel, 3, DEC);
  oled_write('%');

  if((flamelevel > 70 || smokelevel > 30) && !isflamedetected) isflamedetected = true;
  else if (flamelevel < 70 && smokelevel < 30 && isflamedetected) isflamedetected = false;
  if(isflamedetected) digitalWrite(alarm, HIGH); else digitalWrite(alarm, LOW);

  oled_display_cursor(0, 16);
  if(isflamedetected) oled_print("FLAME DETECTED");
  else oled_print("FLAME NORMAL  ");

  oled_display_cursor(0, 24);
  if(isflamedetected)
  {
    memset(nrfarray, '\0', sizeof(nrfarray)); nrfcount = 0; 
    strcpy((char*)nrfarray, "{LAT:");
    strcat((char*)nrfarray, lattitude);
    strcat((char*)nrfarray, ",LON:");
    strcat((char*)nrfarray, longitude);
    strcat((char*)nrfarray, "}");
    nrfcount = strlen((char*)nrfarray);
  }
  else 
  {
    memcpy(nrfarray, "{NORMAL}", 8); 
    nrfcount = 8;
  }
  nrf_transmit(nrfarray, &nrfcount);
  oled_display();
}