#include "../src/gps/main.h"

bool isdataupdated;
uint8_t count, count1;
uart_inst_t *gpstype;
char *codsdata[14];
char *gpsarray;

void gps_routine()
{
    gpsarray[count] = serial_receive(gpstype);
    if(count1 > 60 && gpsarray[count] == '\n') isdataupdated = true;

    if(gpsarray[0] == '$') count = 1;
    if(gpsarray[0] == '$' && gpsarray[1] == 'G') count = 2;
    if(gpsarray[0] == '$' && gpsarray[1] == 'G' && gpsarray[2] == 'P') count = 3;
    if(gpsarray[0] == '$' && gpsarray[1] == 'G' && gpsarray[2] == 'P' && gpsarray[3] == 'R') count = 4;
    if(gpsarray[0] == '$' && gpsarray[1] == 'G' && gpsarray[2] == 'P' && gpsarray[3] == 'R' && gpsarray[4] == 'M') count = 5;
    if(gpsarray[0] == '$' && gpsarray[1] == 'G' && gpsarray[2] == 'P' && gpsarray[3] == 'R' && gpsarray[4] == 'M' && gpsarray[5] == 'C')
    {
        count = count1;
        count1 = (!isdataupdated && count1 < 75 ?count1 + 1 :count1);
    }
}

void gps_initialize(uart_inst_t *guart)
{
    gpstype = guart;
    switch(uartindex(gpstype))
    {
        case 1: gpsarray = (char*)serialarray1; break;
        case 2: gpsarray = (char*)serialarray2; break;
        case 3: gpsarray = (char*)serialarray3; break;
    }

    serial_enable(gpstype, gps_routine, true);
    serial_flush(gpstype); isdataupdated = false;
    count = 0; count1 = 0;
}

void gps_update(gps_result_variables_t *ptr)
{
    if(!isdataupdated) return ;

    unsigned char rcount = 0;
    codsdata[rcount] = strtok(gpsarray, ",");
    while(codsdata[rcount] != NULL)
    codsdata[++rcount] = strtok(NULL, ",");

    gps_status(ptr);
    gps_time(ptr);
    gps_position(ptr);
    gps_speed(ptr);

    count = 0; count1 = 6;
    serial_flush(gpstype);
    isdataupdated = false;
}

void gps_status(gps_result_variables_t *ptr)
{
    char *p = codsdata[gps_phrase_status];
    ptr->status = (p[0] == 'A' ? true : false);
}

void gps_time(gps_result_variables_t *ptr)
{
    char *p = codsdata[gps_phrase_time];
    ptr->time.hour = (((p[0] - '0') * 10) + (p[1] - '0'));
    ptr->time.minute = (((p[2] - '0') * 10) + (p[3] - '0'));
    ptr->time.seconds = (((p[4] - '0') * 10) + (p[5] - '0'));

    p = codsdata[gps_phrase_date];
    ptr->time.day = (((p[0] - '0') * 10) + (p[1] - '0'));
    ptr->time.month = (((p[2] - '0') * 10) + (p[3] - '0'));
    ptr->time.year = (((p[4] - '0') * 10) + (p[5] - '0'));
}


void gps_position(gps_result_variables_t *ptr)
{
    unsigned char localarray[7], degree;
    char *plat = codsdata[gps_phrase_lattitude];
    char *plon = codsdata[gps_phrase_longitude];

    degree = (((plat[0] - '0') * 10) + (plat[1] - '0'));
    memcpy(localarray, &plat[2], 7);
    ptr->lattitude = degree + ((atof((char*)localarray)) / 60);

    degree = (((plon[0] - '0') * 100) + ((plon[1] - '0') * 10) + (plon[2] - '0'));
    memcpy(localarray, &plon[3], 7);
    ptr->longitude = degree + ((atof((char*)localarray)) / 60);
}

void gps_speed(gps_result_variables_t *ptr)
{
    char *pspd = codsdata[gps_phrase_speed];
    char *pang = codsdata[gps_phrase_angle];
    ptr->speed = atof(pspd);
    ptr->angle = atof(pang);
}