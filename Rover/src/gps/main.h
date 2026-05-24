/**
 * @file    gps.h
 * @author  saurash automations
 * @brief   gps header file
 * @version 0.1
 * @date    2024-10-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _GPS_H
#define _GPS_H

#include "../src/utils/main.h"
#include "../src/uart/main.h"

#ifdef __cpluscplus
extern "C"
#endif 

typedef enum
{
    gps_phrase_sentence,
    gps_phrase_time,
    gps_phrase_status,
    gps_phrase_lattitude, 
    gps_phrase_lattitude_direction,
    gps_phrase_longitude,
    gps_phrase_longitude_direction,
    gps_phrase_speed,
    gps_phrase_angle,
    gps_phrase_date,
    gps_phrase_variation,
    gps_phrase_checksum
}
gps_phrase_result_t;

typedef struct 
{
    uint8_t hour;
    uint8_t minute;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint8_t year;
}
gps_clock_variable_config_t;

typedef struct 
{
    bool status;
    gps_clock_variable_config_t time;
    float lattitude;
    float longitude;
    float speed;
    float angle;
}
gps_result_variables_t;

extern uart_inst_t *gpstype;
extern char *gpsarray;
extern void gps_routine(void);

extern void gps_initialize(uart_inst_t *guart);
extern void gps_update(gps_result_variables_t *ptr);
extern void gps_status(gps_result_variables_t *ptr);
extern void gps_time(gps_result_variables_t *ptr);
extern void gps_position(gps_result_variables_t *ptr);
extern void gps_speed(gps_result_variables_t *ptr);
#endif 