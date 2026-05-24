#include "../src/servo/main.h"

servo_pin_config_t servo[MAX_NO_SERVO];
volatile int16_t routinefiredcount;
volatile uint8_t servoalarm;

void servoisrroutine()
{
    TIM_TypeDef *timer = getregister(servoalarm);

    if(timer->SR & (1 << 0))
    {
        timer->SR |= (1 << 0); //restart immediately

        if(++routinefiredcount > 1999) routinefiredcount = 0; //10us * 2000 = 20ms refresh rate

        for(uint8_t k = 0; k < MAX_NO_SERVO; k++) //populate servo number
        {
            if(servo[k].isactive) //Do further when it is activated
            {
                if(routinefiredcount == 0) //Always start pulse at start of frequency
                {
                    gpio_high(servo[k].pin); //set high on gpio pin
                    servo[k].activatedtime = micros(); //fetch current micros
                }
                else if(micros() >= (servo[k].activatedtime + servo[k].pulseperiod)) 
                {
                    gpio_low(servo[k].pin);
                    servo[k].activatedtime = false;
                }
            }
            else continue;
        }
    }
}


void servo_attach(servo_pin_config_t *servo, gpio_pinmapping_t pin)
{
    servo->pin = pin;                        //assign pin on structure
    servo->isactive = true;                  //assign activity status
    servo->activatedtime = false;            //clear activated time 
    servo_write(servo, SERVO_MIN_ANGLE);

    gpio_set_output(servo->pin);
    gpio_put_low(servo->pin);
}

void servo_initialize()
{
    routinefiredcount = -1;
    servoalarm = add_repeating_alarm_us(10); 
    start_repeating_alarm(servoalarm, servoisrroutine);
    //10us firing range for a routine
}

void servo_dettach(servo_pin_config_t *servo)
{
    servo->isactive = false;
    servo_write(servo, SERVO_MIN_ANGLE);
}

void servo_write(servo_pin_config_t *servo, uint8_t position)
{
    servo->position     = position;
    servo->pulseperiod  = mapdecimal(servo->position, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
}