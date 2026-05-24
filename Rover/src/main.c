#include "../src/utils/main.h"
#include "../src/gpio/main.h"
#include "../src/alarm/main.h"
#include "../src/i2c/main.h"
#include "../src/spi/main.h"
#include "../src/extint/main.h"

#include "../src/hcsr04/main.h"
#include "../src/servo/main.h"
#include "../src/uart/main.h"
#include "../src/oled/main.h"

#include "../src/lidar/main.h"
#include "../src/nrf24/main.h"
#include "../src/gps/main.h"

#include <string.h>
#include <math.h>

/*! Arduino bluetooth control command codes !*/
/*
 * 
 *      F           T
 *    L   R       S   C
 *      B           X
 *          P   S
 *      
 */
/*! Those are command sequences to control !*/
#define FORWARD   "F"
#define BACKWARD  "B"
#define LEFT      "L"
#define RIGHT     "R"
#define CIRCLE    "C"
#define CROSS     "X"
#define TRIANGLE  "T"
#define SQUARE    "S"
#define START     "A"
#define PAUSE     "P"
#define RELEASE   "0"

#ifndef servopinconfig
#define servopinconfig
#define servomotor PA0
#endif 

#ifndef flamepinconfig
#define flamepinconfig
#define flamesensor PA1
#endif 

#ifndef gpspinconfig
#define gpspinconfig
#define gpstype         uart2
#define gpstransmitter  PA2
#define gpsreceiver     PA3
#define gpsbaudrate     9600
#endif 

#ifndef nrfpinconfig
#define nrfpinconfig
#define nrftype spi1
#define nrfcsn  PA4
#define nrfsck  PA5
#define nrfmiso PA6
#define nrfmosi PA7
#define nrfce   PB0
#define nrffreq SPI1_FREQ_2MHZ
#endif 

#ifndef pumppinconfig
#define pumppinconfig
#define pump PB1
#endif 

#ifndef oledpinconfig
#define oledpinconfig
#define oledtype        i2c2
#define oledheight      32
#define oledwidth       128
#define oleddata        PB11
#define oledclock       PB10
#define oledaddress     0x78
#define oledfrequency   400
#endif 

#ifndef motorpinconfig
#define motorpinconfig
#define motor1a PB12
#define motor1b PB13
#define motor2a PB14
#define motor2b PB15
#endif 

#ifndef alarmpinconfig
#define alarmpinconfig
#define alarm   PA8
#endif 

#ifndef btpinconfig
#define btpinconfig
#define bttype          uart1
#define bttransmitter   PA9
#define btreceiver      PA10
#define btbaudrate      9600
#endif 

#ifndef radarpinconfig
#define radarpinconfig
#define radartrigger    PA11
#define radarecho       PA12
#endif 

#ifndef lidarpinconfig
#define lidarpinconfig
#define lidartype       i2c1
#define lidarclock      PB8
#define lidardata       PB9
#define lidarfreq       100
#endif

#ifndef lidarshutconfig
#define lidarshutconfig
#define lidarshut1      PB7
#define lidarshut2      PB6
#endif 

#ifndef buildledconfig
#define buildledconfig
#define buildled    PC13
#define heartrate   500
#endif 

typedef enum
{
    rover_motors_stopped = 0,
    rover_motors_forward = 1,
    rover_motors_reverse = 2,
    rover_motors_right   = 3,
    rover_motors_left    = 4
}
rover_motor_position_t;

gps_result_variables_t gps;
rover_motor_position_t motor;

float lattitude, longitude;
uint16_t frontdistance = 0;
uint16_t rightdistance = 0;
uint16_t leftdistance = 0;

bool isflamedetected;
bool isremotetriggered;

volatile uint8_t heartbeatalarm = 0;
uint8_t address[] = "PIPE1";
uint8_t address1[] = "DUMMY";
uint8_t nrfarray[32], nrfcount;
uint32_t motorupdateinterval;

static inline void forward()
{
    motor = rover_motors_forward;
    gpio_put_high(motor1a);
    gpio_put_low(motor1b);
    gpio_put_high(motor2a);
    gpio_put_low(motor2b);
}

static inline void reverse()
{
    motor = rover_motors_reverse;
    gpio_put_low(motor1a);
    gpio_put_high(motor1b);
    gpio_put_low(motor2a);
    gpio_put_high(motor2b);
}

static inline void right()
{
    motor = rover_motors_right;
    gpio_put_high(motor1a);
    gpio_put_low(motor1b);
    gpio_put_low(motor2a);
    gpio_put_high(motor2b);
}

static inline void left()
{
    motor = rover_motors_left;
    gpio_put_low(motor1a);
    gpio_put_high(motor1b);
    gpio_put_high(motor2a);
    gpio_put_low(motor2b);
}

static inline void stop()
{
    motor = rover_motors_stopped;
    gpio_put_low(motor1a);
    gpio_put_low(motor1b);
    gpio_put_low(motor2a);
    gpio_put_low(motor2b);
}

void armheartbeat(void)
{
    TIM_TypeDef *beat = getregister(heartbeatalarm);
    if(beat->SR & (1 << 0))
    {
        gpio_toggle(buildled);
        beat->SR &=~ (1 << 0);
    }
}

static inline void writeservo(uint8_t pos)
{
    uint64_t angle = mapdecimal(pos, 0, 180, 544, 2400) + micros();
    do gpio_high(servomotor); while(angle >= micros());
    gpio_low(servomotor); delay_ms(20);
}

static float distbwtwopoints(float lat1, float lon1, float lat2, float lon2)
{
    float distcalc1 = 0, distcalc2 = 0, diflat = 0, diflon = 0;

    diflat = (lat2 - lat1) * (M_PI / 180.0F);
    lat1 = lat1 * (M_PI / 180.0F);
    lat2 = lat2 * (M_PI / 180.0F);
    diflon = (lon2 - lon1) * (M_PI / 180.0F);

    distcalc1 = (sin(diflat / 2.0F) * sin(diflat / 2.0F));
    distcalc2 = cos(lat1) * cos(lat2);
    distcalc2 = distcalc2 * (sin(diflon / 2.0F) * sin(diflon / 2.0F));
    distcalc1 = distcalc1 + distcalc2;

    distcalc1 = (2 * atan2(sqrt(distcalc1), sqrt(1.0F - distcalc1)));
    distcalc1 = distcalc1 * 6371000.0F; 

    return distcalc1;
}

int main()
{
    stdio_init_all();
    gpio_set_input(flamesensor);

    gpio_set_output(buildled); gpio_put_low(buildled);
    gpio_set_output(alarm); gpio_put_low(alarm);
    gpio_set_output(pump); gpio_put_low(pump);
    gpio_set_output(servomotor); gpio_put_low(servomotor);

    gpio_set_output(motor1a); gpio_put_low(motor1a);
    gpio_set_output(motor1b); gpio_put_low(motor1b);
    gpio_set_output(motor2a); gpio_put_low(motor2b);
    gpio_set_output(motor2b); gpio_put_low(motor2b);

    heartbeatalarm = add_repeating_alarm_ms(heartrate);
    start_repeating_alarm(heartbeatalarm, armheartbeat);
    radar_initialize(radartrigger, radarecho);

    i2c_initialize(oledtype, oledclock, oleddata, oledfrequency);
    i2c_initialize(lidartype, lidarclock, lidardata, lidarfreq);
    serial_initialize(gpstype, gpstransmitter, gpsreceiver, gpsbaudrate);
    serial_initialize(bttype, bttransmitter, btreceiver, btbaudrate);
    spi_initialize(nrftype, nrfsck, nrfmosi, nrfmiso, nrffreq);

    oled_initialize(oledtype, oledheight, oledwidth);
    oled_print("AUTONOMOUS ROBOT");
    oled_print("FOR FOREST FIRE ");
    oled_print("DETECT & QUENCH ");
    oled_print("ROVER USING STM ");
    oled_display(); delay_ms(2500);
    oled_fill_screen(black);

    lidar_config(&lidar[0], lidartype, lidarshut1, 0x40);
    lidar_config(&lidar[1], lidartype, lidarshut2, 0x42);

    oled_set_cursor(0, 0);
    if(lidar_initialize()) oled_print("LIDAR INITIALIZE");
    else oled_print("LIDAR ERROR");
    oled_display(); delay_ms(500);

    oled_set_cursor(0, 8);
    if(nrf_initialize(nrftype, nrfcsn, nrfce, 90)) oled_print("NRF INITIALIZED");
    else oled_print("NRF ERROR");
    oled_display(); delay_ms(500);

    oled_set_cursor(0, 16);
    oled_print("RADAR-FLAME DONE");
    oled_display(); delay_ms(500);

    oled_set_cursor(0, 24);
    gps_initialize(gpstype);
    oled_print("GPS INITIALIZED");
    oled_display(); delay_ms(500);
    oled_fill_screen(black);

    serial_enable(bttype, NULL, true);
    serial_flush(bttype);
    motor = rover_motors_stopped;
    nrf_rxr_address(1, address);

    while(1)
    {
        gps_update(&gps);
        frontdistance = radar_fetch();
        rightdistance = lidar_read_oneshot(&lidar[0]);
        leftdistance  = lidar_read_oneshot(&lidar[1]);
        isflamedetected = gpio_get(flamesensor);

        oled_set_cursor(0, 0);
        oled_float(lattitude, 7);
        oled_write(',');
        oled_float(longitude, 7);

        oled_set_cursor(0, 8);
        oled_print("F:");
        if(isflamedetected) oled_print("DET");
        else oled_print("NOR");

        oled_set_cursor(64, 8);
        oled_print("P:");
        if(isremotetriggered) oled_print("TRI");
        else oled_print("NOR");

        oled_set_cursor(0, 16);
        oled_print("L:");
        oled_decimal(leftdistance, 4, DEC);

        oled_set_cursor(64, 16);
        oled_print("R:");
        oled_decimal(rightdistance, 4, DEC);
        oled_print("mm");

        oled_set_cursor(0, 24);
        oled_print("F:");
        oled_decimal(frontdistance, 3, DEC);
        oled_print("cm");

        oled_set_cursor(72, 24);
        switch(motor)
        {
            case rover_motors_stopped: oled_print("STP"); break;
            case rover_motors_forward: oled_print("FWD"); break;
            case rover_motors_reverse: oled_print("REV"); break;
            case rover_motors_right:   oled_print("RIG"); break;
            case rover_motors_left:    oled_print("LEF"); break;
        }
        oled_display();

        if(serialcount1 > 0)
        {
            if(strstr((char*)serialarray1, FORWARD) != NULL) forward();
            else if(strstr((char*)serialarray1, BACKWARD) != NULL) reverse();
            else if(strstr((char*)serialarray1, RIGHT) != NULL) right();
            else if(strstr((char*)serialarray1, LEFT) != NULL) left();
            else if(strstr((char*)serialarray1, RELEASE) != NULL) stop();
            gpio_high(alarm); delay_ms(250); gpio_low(alarm); serial_flush(bttype);
        }

        if(nrf_receive(nrfarray, &nrfcount))
        {
            oled_set_cursor(0, 0); 
            char buffer[12]; memset(buffer, '\0', sizeof(buffer));
            if(split((char*)nrfarray, buffer, "{LAT:", ',') != NULL) lattitude = atof((char*)buffer);
            if(split((char*)nrfarray, buffer, "LON:", '}') != NULL) longitude = atof((char*)buffer);
            memset(nrfarray, '\0', sizeof(nrfarray)); nrfcount = 0;
        }

        if(lattitude && longitude && !isremotetriggered) 
        isremotetriggered = true;

        if(isflamedetected)
        {
            stop(); gpio_high(alarm); gpio_high(pump);
            do
            {
                for(uint8_t k = 20; k <= 160; k++) writeservo(k);
                for(uint8_t k = 160; k >= 20; k--) writeservo(k);
                isflamedetected = gpio_get(flamesensor);
            }
            while(isflamedetected);
            stop(); gpio_low(alarm); gpio_low(pump);
            isremotetriggered = false;
            lattitude = longitude = false;
        }

        if(gps.status && lattitude && longitude && isremotetriggered && (motorupdateinterval < millis()))
        {
            //check whether destination reached or not ..!
            if(distbwtwopoints(gps.lattitude, gps.longitude, lattitude, longitude) > 5.00F) //for more than 10 meter
            {
                if(frontdistance < 30)
                {
                    if(rightdistance > leftdistance)
                    {
                        right(); 
                        delay_ms(1500);
                        forward(); 
                        delay_ms(1500);
                        left();
                    }
                    else 
                    {
                        left();
                        delay_ms(1500);
                        forward();
                        delay_ms(1500);
                        right();
                    }
                }
                else forward();
            }
            else
            {
                right();
            }

            motorupdateinterval = millis() + 1500;
        }
    }
}