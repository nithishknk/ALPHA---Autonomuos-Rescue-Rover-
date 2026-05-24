/**
 * @file    lidar.h
 * @author  saurash automations
 * @brief   vl53l0x lidar stm32 firmware
 * @version 0.1
 * @date    2025-03-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _LIDAR_H
#define _LIDAR_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"
#include "../src/i2c/main.h"
#include <stdlib.h>

//Dont forget to update build_flags= -DMAX_NO_LIDAR=x on ini file

#define SYSRANGE_START                              0x00
#define SYSTEM_THRESH_HIGH                          0x0C
#define SYSTEM_THRESH_LOW                           0x0E
#define SYSTEM_SEQUENCE_CONFIG                      0x01
#define SYSTEM_RANGE_CONFIG                         0x09
#define SYSTEM_INTERMEASUREMENT_PERIOD              0x04
#define SYSTEM_INTERRUPT_CONFIG_GPIO                0x0A
#define GPIO_HV_MUX_ACTIVE_HIGH                     0x84
#define SYSTEM_INTERRUPT_CLEAR                      0x0B
#define RESULT_INTERRUPT_STATUS                     0x13
#define RESULT_RANGE_STATUS                         0x14
#define RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       0xBC
#define RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        0xC0
#define RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       0xD0
#define RESULT_CORE_RANGING_TOTAL_EVENTS_REF        0xD4
#define RESULT_PEAK_SIGNAL_RATE_REF                 0xB6
#define ALGO_PART_TO_PART_RANGE_OFFSET_MM           0x28
#define I2C_SLAVE_DEVICE_ADDRESS                    0x8A
#define MSRC_CONFIG_CONTROL                         0x60
#define PRE_RANGE_CONFIG_MIN_SNR                    0x27
#define PRE_RANGE_CONFIG_VALID_PHASE_LOW            0x56
#define PRE_RANGE_CONFIG_VALID_PHASE_HIGH           0x57
#define PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          0x64
#define FINAL_RANGE_CONFIG_MIN_SNR                  0x67
#define FINAL_RANGE_CONFIG_VALID_PHASE_LOW          0x47
#define FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         0x48
#define FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT 0x44
#define PRE_RANGE_CONFIG_SIGMA_THRESH_HI            0x61
#define PRE_RANGE_CONFIG_SIGMA_THRESH_LO            0x62
#define PRE_RANGE_CONFIG_VCSEL_PERIOD               0x50
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          0x51
#define PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          0x52
#define SYSTEM_HISTOGRAM_BIN                        0x81
#define HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       0x33
#define HISTOGRAM_CONFIG_READOUT_CTRL               0x55
#define FINAL_RANGE_CONFIG_VCSEL_PERIOD             0x70
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        0x71
#define FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        0x72
#define CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       0x20
#define MSRC_CONFIG_TIMEOUT_MACROP                  0x46
#define SOFT_RESET_GO2_SOFT_RESET_N                 0xBF
#define IDENTIFICATION_MODEL_ID                     0xC0
#define IDENTIFICATION_REVISION_ID                  0xC2
#define OSC_CALIBRATE_VAL                           0xF8
#define GLOBAL_CONFIG_VCSEL_WIDTH                   0x32
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_0            0xB0
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_1            0xB1
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_2            0xB2
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_3            0xB3
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_4            0xB4
#define GLOBAL_CONFIG_SPAD_ENABLES_REF_5            0xB5
#define GLOBAL_CONFIG_REF_EN_START_SELECT           0xB6
#define DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         0x4E
#define DYNAMIC_SPAD_REF_EN_START_OFFSET            0x4F
#define POWER_MANAGEMENT_GO1_POWER_FORCE            0x80
#define VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           0x89
#define ALGO_PHASECAL_LIM                           0x30
#define ALGO_PHASECAL_CONFIG_TIMEOUT                0x30

#define VCSELPERIODPRERANGE                         0x00
#define VCSELPERIODFINALRANGE                       0x01

typedef struct
{
    uint16_t rawdistance;
    uint16_t signalcount;
    uint16_t ambientcount;
    uint16_t spadcount;
    uint8_t rangestatus;
}
lidar_state_info_t;

typedef struct 
{
    uint8_t tcc;
    uint8_t msrc;
    uint8_t dss;
    uint8_t prerange;
    uint8_t finalrange;
}
lidar_step_config_t;

typedef struct 
{
    uint16_t prerangevcselperiodclocks;
    uint16_t finalrangevcselperiodclocks;
    uint16_t msrcdsstccmclks;
    uint16_t prerangemclks;
    uint16_t finalrangemmclks;
    uint32_t msrcdsstccus;
    uint32_t prerangeus;
    uint32_t finalrangeus;
}
lidar_timeout_config_t;

typedef struct 
{
    i2c_inst_t *i2c;
    gpio_pinmapping_t shutdown;
    uint8_t address;

    lidar_state_info_t state;
    lidar_step_config_t step;
    lidar_timeout_config_t timeout;

    uint8_t buffer[6];
    uint16_t iotimeout;             /* g_iotimeout */
    bool istimeout;                 /* g_istimeout */
    uint16_t timeoutstart;          /* g_timeoutstartms */
    uint8_t stopvariablemillis;     /* g_stopvariable   */
    uint32_t stopvariablemicros;    /* g_meastimbudus */
}
lidar_pin_config_t;

extern lidar_pin_config_t lidar[MAX_NO_LIDAR];
extern bool lidar_config(lidar_pin_config_t *lidar, i2c_inst_t *i2c, gpio_pinmapping_t shut, uint8_t address);
extern bool lidar_initialize(void);
extern void lidar_start_continuous(lidar_pin_config_t *lidar, uint32_t timeout);
extern void lidar_stop_continuous(lidar_pin_config_t *lidar);
extern uint16_t lidar_read_oneshot(lidar_pin_config_t *lidar);
extern uint16_t lidar_read_continuous(lidar_pin_config_t *lidar);

static inline void writereg8bit(lidar_pin_config_t *lidar, uint8_t reg, uint8_t value)
{
    lidar->buffer[0] = reg; lidar->buffer[1] = value;
    i2c_write_sequence(lidar->i2c, lidar->address, lidar->buffer, 2, true);
}

static inline void writereg16bit(lidar_pin_config_t *lidar, uint8_t reg, uint16_t value)
{
    lidar->buffer[0] = reg; memcpy(&lidar->buffer[1], &value, 2);
    i2c_write_sequence(lidar->i2c, lidar->address, lidar->buffer, 3, true);
}

static inline void writereg32bit(lidar_pin_config_t *lidar, uint8_t reg, uint32_t value)
{
    lidar->buffer[0] = reg; memcpy(&lidar->buffer[1], &value, 4);
    i2c_write_sequence(lidar->i2c, lidar->address, lidar->buffer, 5, true);
}

static inline void writesequence(lidar_pin_config_t *lidar, uint8_t reg, uint8_t *src, uint8_t count)
{
    uint8_t *ptr = (uint8_t*)malloc((count + 1) * sizeof(char));
    ptr[0] = reg; memcpy(&ptr[1], src, count);
    i2c_write_sequence(lidar->i2c, lidar->address, ptr, count + 1, true);
    free(ptr);
}

static inline uint8_t readreg8bit(lidar_pin_config_t *lidar, uint8_t reg)
{
    uint8_t value = 0;
    i2c_write_sequence(lidar->i2c, lidar->address, &reg, 1, false);
    i2c_read_sequence(lidar->i2c, lidar->address, lidar->buffer, 1);
    value = lidar->buffer[0];
    return value;
}

static inline uint16_t readreg16bit(lidar_pin_config_t *lidar, uint8_t reg)
{
    uint16_t value = 0;
    i2c_write_sequence(lidar->i2c, lidar->address, &reg, 1, false);
    i2c_read_sequence(lidar->i2c, lidar->address, lidar->buffer, 2);
    memcpy(&value, lidar->buffer, 2);
    return value;
}

static inline uint32_t readreg32bit(lidar_pin_config_t *lidar, uint8_t reg)
{
    uint32_t value = 0;
    i2c_write_sequence(lidar->i2c, lidar->address, &reg, 1, false);
    i2c_read_sequence(lidar->i2c, lidar->address, lidar->buffer, 4);
    memcpy(&value, lidar->buffer, 4);
    return value;
}

static inline void readsequence(lidar_pin_config_t *lidar, uint8_t reg, uint8_t *dst, uint8_t count)
{
    i2c_write_sequence(lidar->i2c, lidar->address, &reg, 1, false);
    i2c_read_sequence(lidar->i2c, lidar->address, dst, count);
}

static inline void starttimeout(lidar_pin_config_t *lidar) 
{ 
    lidar->timeoutstart = millis(); 
}

static inline bool checktimeoutexpired(lidar_pin_config_t *lidar) 
{ 
    return (lidar->iotimeout > 0 && ((uint16_t)millis() - lidar->timeoutstart) > lidar->iotimeout); 
}

static inline uint8_t decodevscelperiod(uint8_t val) 
{ 
    return (((val) + 1) >> 1); 
}

static inline uint8_t encodevscelperiod(uint8_t val) 
{ 
    return (((val) >> 1) - 1); 
}

static inline uint32_t calcmacroperiod(uint32_t val) 
{ 
    return ((((uint32_t)2304 * (val) * 1655) + 500) / 1000); 
}

static inline uint32_t lidar_timeout_us(uint16_t timeoutperiodmclks, uint8_t vcselperiodclocks)
{
    uint32_t macroperiodns = calcmacroperiod(vcselperiodclocks);
    return ((timeoutperiodmclks * macroperiodns) + (macroperiodns / 2)) / 1000;
}

static inline uint32_t lidar_timeout_mclk(uint32_t timeoutperiodus, uint8_t vcselperiodclocks)
{
    uint32_t macroperiodns = calcmacroperiod(vcselperiodclocks);
    return (((timeoutperiodus * 1000) + (macroperiodns / 2)) / macroperiodns);
}
#endif 