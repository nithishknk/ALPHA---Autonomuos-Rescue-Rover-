#ifndef _I2C_H
#define _I2C_H

extern void i2c_initialize(unsigned int);
extern void i2c_start(void);
extern void i2c_stop(void);
extern void i2c_restart(void);

extern unsigned char i2c_bus_scan(void);
extern unsigned char i2c_write(unsigned char);
extern unsigned char i2c_read(unsigned char);

extern unsigned char i2c_register_write(unsigned char, unsigned char, unsigned char);
extern unsigned char i2c_register_read(unsigned char, unsigned char);

extern void i2c_sequence_write(unsigned char, unsigned char, unsigned char*, unsigned int);
extern void i2c_sequence_read(unsigned char, unsigned char, unsigned char*, unsigned int);
#endif
