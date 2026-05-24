/**
 * @file    spi header file
 * @author  saurash automations
 * @brief   stm32 spi header file
 * @version 0.1
 * @date    2025-02-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _SPI_H
#define _SPI_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"

typedef enum
{ 
    SPI1_FREQ_36MHZ   = 0,  /*  72 / 2   = 36Mhz       */
    SPI1_FREQ_18MHZ   = 1,  /*  72 / 4   = 18Mhz       */
    SPI1_FREQ_9MHZ    = 2,  /*  72 / 8   = 9Mhz        */
    SPI1_FREQ_4MHZ    = 3,  /*  72 / 16  = 4.5Mhz      */
    SPI1_FREQ_2MHZ    = 4,  /*  72 / 32  = 2.25Mhz     */
    SPI1_FREQ_1MHZ    = 5,  /*  72 / 64  = 1.125Mhz    */
    SPI1_FREQ_500KHZ  = 6,  /*  72 / 128 = 56250Khz    */
    SPI1_FREQ_250KHZ  = 7,  /*  72 / 256 = 28125Khz    */

    SPI2_FREQ_18MHZ   = 0,  /*  36 / 2   = 18MhZ       */
    SPI2_FREQ_9MHZ    = 1,  /*  36 / 4   = 9MhZ        */ 
    SPI2_FREQ_4MHZ    = 2,  /*  36 / 8   = 4.5Mhz      */
    SPI2_FREQ_2MHZ    = 3,  /*  36 / 16  = 2.25Mhz     */
    SPI2_FREQ_1MHZ    = 4,  /*  36 / 32  = 1.125Mhz    */
    SPI2_FREQ_500KHZ  = 5,  /*  36 / 64  = 56250Khz    */
    SPI2_FREQ_250KHZ  = 6,  /*  36 / 128 = 28125Khz    */
    SPI2_FREQ_125KHZ  = 7   /*  36 / 256 = 140625Khz   */
}
spi_freq_t;

typedef SPI_TypeDef spi_inst_t;
#define spi1 (SPI1)
#define spi2 (SPI2)

extern void spi_initialize(spi_inst_t *spi, uint8_t sck, uint8_t mosi, uint8_t miso, spi_freq_t freq);
extern void spi_set_repeated(spi_inst_t *spi, uint8_t data);

extern void spi_write(spi_inst_t *spi, uint8_t data);
extern uint8_t spi_read(spi_inst_t *spi);
extern uint8_t spi_read_write(spi_inst_t *spi, uint8_t data);

extern void spi_write_sequence(spi_inst_t *spi, uint8_t *data, uint16_t length);
extern void spi_read_sequence(spi_inst_t *spi, uint8_t *data, uint16_t length);
extern void spi_read_write_sequence(spi_inst_t *spi, uint8_t *data, uint16_t length);
#endif 