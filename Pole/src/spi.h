#ifndef _SPI_H
#define _SPI_H

extern void spi_initialize(char, char, char);
extern void spi_write(unsigned char);
extern unsigned char spi_read();
extern unsigned char spi_read_write(unsigned char);

extern void spi_write_sequence(unsigned char*, unsigned int);
extern void spi_read_sequence(unsigned char*, unsigned int);
extern void spi_read_write_sequence(unsigned char*, unsigned int);
#endif
