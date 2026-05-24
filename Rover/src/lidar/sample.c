#include "../src/lidar/main.h"

lidar_pin_config_t lidar[MAX_NO_LIDAR];

bool lidar_spad_info(lidar_pin_config_t *lidar, uint8_t *count, bool *typeisaperture);
uint32_t lidar_get_timing(lidar_pin_config_t *lidar);
bool lidar_set_timing(lidar_pin_config_t *lidar, uint32_t budgetus);
void lidar_get_step(lidar_pin_config_t *lidar);
void lidar_get_timeout(lidar_pin_config_t *lidar);
uint16_t lidar_time_encode(uint16_t timeoutmclks);
uint16_t lidar_time_decode(uint16_t tinmeoutclks);
bool lidar_set_pulseperiod(lidar_pin_config_t *lidar, uint8_t type, uint8_t periodpclks);
uint8_t lidar_get_pulseperiod(lidar_pin_config_t *lidar, uint8_t type);
bool lidar_single_calibrate(lidar_pin_config_t *lidar, uint8_t vhvinitbyte);

bool lidar_config(lidar_pin_config_t *lidar, i2c_inst_t *i2c, gpio_pinmapping_t shut, uint8_t address)
{
    if(address < 0x40 || address > 0x7E) return false; //address out of range
    if((address % 2)) address = address - 1; //Not 7bit addressing mode
    if(address == 0x00) address = 0x52; //default address handler

    if(shut)
    {
        gpio_set_output(shut);
        gpio_put_low(shut);
        delay_ms(10);
    } 

    lidar->i2c = i2c;
    lidar->shutdown = shut;
    lidar->address = address;

    return true;
}

bool lidar_initialize(void)
{
    /* run a loop again and set a new address with out resetting pin */
    for(size_t k = 0; k < MAX_NO_LIDAR; k++)
    {
        if(lidar[k].i2c == I2C1 || lidar[k].i2c == I2C2)
        {
            /* reset registers before start into process */
            memset(lidar[k].buffer, '\0', sizeof(lidar[k].buffer));
            lidar[k].iotimeout = false; lidar[k].istimeout = false;
            lidar[k].timeoutstart = false; lidar[k].stopvariablemicros = false;
            lidar[k].stopvariablemicros = false;

            /* don't worry about default addressing that will do the same */
            if(lidar[k].shutdown) 
            {
                gpio_put_high(lidar[k].shutdown); 
                delay_ms(10);
            }
            uint8_t newaddress = lidar[k].address; lidar[k].address = 0x52; //default address initially..!
            writereg8bit(&lidar[k], I2C_SLAVE_DEVICE_ADDRESS, (newaddress >> 1) & 0x7F);
            lidar[k].address = newaddress; //Fuse address only if it's valid
            delay_ms(25); delay_ms(25); delay_ms(25); delay_ms(25);

            if(readreg8bit(&lidar[k], IDENTIFICATION_MODEL_ID) != 0xEE) 
            return false;
            
            /* for 2v8 reference, default is 1v8 */
            uint8_t value = readreg8bit(&lidar[k], VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01;
            writereg8bit(&lidar[k], VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, value);

            // set i2c standard mode
            writereg8bit(&lidar[k], 0x88, 0x00);
            writereg8bit(&lidar[k], 0x80, 0x01);
            writereg8bit(&lidar[k], 0xFF, 0x01);
            writereg8bit(&lidar[k], 0x00, 0x00);
            lidar[k].stopvariablemillis = readreg8bit(&lidar[k], 0x91);
            writereg8bit(&lidar[k], 0x00, 0x01);
            writereg8bit(&lidar[k], 0xFF, 0x00);
            writereg8bit(&lidar[k], 0x80, 0x00);

            //disable signalratemsrc and signal rate prerange 
            value = readreg8bit(&lidar[k], MSRC_CONFIG_CONTROL) | 0x12;
            writereg8bit(&lidar[k], MSRC_CONFIG_CONTROL, value);

            //set signal rate limit (default 0.25)
            writereg16bit(&lidar[k], FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0.25F * (1 << 7));
            writereg8bit(&lidar[k], SYSTEM_SEQUENCE_CONFIG, 0xFF);

            //spad info & config 
            uint8_t spadcount, refspadmap[6];
            bool spadtypeisaperture = false;

            if(!lidar_spad_info(&lidar[k], &spadcount, &spadtypeisaperture)) return false;
            readsequence(&lidar[k], GLOBAL_CONFIG_SPAD_ENABLES_REF_0, refspadmap, 6);

            writereg8bit(&lidar[k], 0xFF, 0x01);
            writereg8bit(&lidar[k], DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
            writereg8bit(&lidar[k], DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
            writereg8bit(&lidar[k], 0xFF, 0x00);
            writereg8bit(&lidar[k], GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

            uint8_t firstspadtoenable = spadtypeisaperture ? 12 : 0;
            uint8_t spadsenabled = 0;
            for(uint8_t i = 0; i < 48; i++)
            {
                if(i < firstspadtoenable || spadsenabled == spadcount)
                refspadmap[i / 8] &=~ (1 << (i % 8));
                else if((refspadmap[i / 8] >> (i % 8)) & 0x1)
                spadsenabled++;
            }
            writesequence(&lidar[k], GLOBAL_CONFIG_SPAD_ENABLES_REF_0, refspadmap, 6);

            //default settings according to stm 
            writereg8bit(&lidar[k], 0xFF, 0x01); writereg8bit(&lidar[k], 0x00, 0x00);
            writereg8bit(&lidar[k], 0xFF, 0x00); writereg8bit(&lidar[k], 0x09, 0x00);
            writereg8bit(&lidar[k], 0x10, 0x00); writereg8bit(&lidar[k], 0x11, 0x00);
            writereg8bit(&lidar[k], 0x24, 0x01); writereg8bit(&lidar[k], 0x25, 0xFF);
            writereg8bit(&lidar[k], 0x75, 0x00); writereg8bit(&lidar[k], 0xFF, 0x01);
            writereg8bit(&lidar[k], 0x4E, 0x2C); writereg8bit(&lidar[k], 0x48, 0x00);
            writereg8bit(&lidar[k], 0x30, 0x20); writereg8bit(&lidar[k], 0xFF, 0x00);
            writereg8bit(&lidar[k], 0x30, 0x09); writereg8bit(&lidar[k], 0x54, 0x00);
            writereg8bit(&lidar[k], 0x31, 0x04); writereg8bit(&lidar[k], 0x32, 0x03);
            writereg8bit(&lidar[k], 0x40, 0x83); writereg8bit(&lidar[k], 0x46, 0x25);
            writereg8bit(&lidar[k], 0x60, 0x00); writereg8bit(&lidar[k], 0x27, 0x00);
            writereg8bit(&lidar[k], 0x50, 0x06); writereg8bit(&lidar[k], 0x51, 0x00);
            writereg8bit(&lidar[k], 0x52, 0x96); writereg8bit(&lidar[k], 0x56, 0x08);
            writereg8bit(&lidar[k], 0x57, 0x30); writereg8bit(&lidar[k], 0x61, 0x00);
            writereg8bit(&lidar[k], 0x62, 0x00); writereg8bit(&lidar[k], 0x64, 0x00);
            writereg8bit(&lidar[k], 0x65, 0x00); writereg8bit(&lidar[k], 0x66, 0xA0);
            writereg8bit(&lidar[k], 0xFF, 0x01); writereg8bit(&lidar[k], 0x22, 0x32);
            writereg8bit(&lidar[k], 0x47, 0x14); writereg8bit(&lidar[k], 0x49, 0xFF);
            writereg8bit(&lidar[k], 0x4A, 0x00); writereg8bit(&lidar[k], 0xFF, 0x00);
            writereg8bit(&lidar[k], 0x7A, 0x0A); writereg8bit(&lidar[k], 0x7B, 0x00);
            writereg8bit(&lidar[k], 0x78, 0x21); writereg8bit(&lidar[k], 0xFF, 0x01);
            writereg8bit(&lidar[k], 0x23, 0x34); writereg8bit(&lidar[k], 0x42, 0x00);
            writereg8bit(&lidar[k], 0x44, 0xFF); writereg8bit(&lidar[k], 0x45, 0x26);
            writereg8bit(&lidar[k], 0x46, 0x05); writereg8bit(&lidar[k], 0x40, 0x40);
            writereg8bit(&lidar[k], 0x0E, 0x06); writereg8bit(&lidar[k], 0x20, 0x1A);
            writereg8bit(&lidar[k], 0x43, 0x40); writereg8bit(&lidar[k], 0xFF, 0x00);
            writereg8bit(&lidar[k], 0x34, 0x03); writereg8bit(&lidar[k], 0x35, 0x44);
            writereg8bit(&lidar[k], 0xFF, 0x01); writereg8bit(&lidar[k], 0x31, 0x04);
            writereg8bit(&lidar[k], 0x4B, 0x09); writereg8bit(&lidar[k], 0x4C, 0x05);
            writereg8bit(&lidar[k], 0x4D, 0x04); writereg8bit(&lidar[k], 0xFF, 0x00);
            writereg8bit(&lidar[k], 0x44, 0x00); writereg8bit(&lidar[k], 0x45, 0x20);
            writereg8bit(&lidar[k], 0x47, 0x08); writereg8bit(&lidar[k], 0x48, 0x28);
            writereg8bit(&lidar[k], 0x67, 0x00); writereg8bit(&lidar[k], 0x70, 0x04);
            writereg8bit(&lidar[k], 0x71, 0x01); writereg8bit(&lidar[k], 0x72, 0xFE);
            writereg8bit(&lidar[k], 0x76, 0x00); writereg8bit(&lidar[k], 0x77, 0x00);
            writereg8bit(&lidar[k], 0xFF, 0x01); writereg8bit(&lidar[k], 0x0D, 0x01);
            writereg8bit(&lidar[k], 0xFF, 0x00); writereg8bit(&lidar[k], 0x80, 0x01); 
            writereg8bit(&lidar[k], 0x01, 0xF8); writereg8bit(&lidar[k], 0xFF, 0x01);
            writereg8bit(&lidar[k], 0x8E, 0x01); writereg8bit(&lidar[k], 0x00, 0x01);
            writereg8bit(&lidar[k], 0xFF, 0x00); writereg8bit(&lidar[k], 0x80, 0x00);

            //lidar gpio config 
            writereg8bit(&lidar[k], SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
            value = readreg8bit(&lidar[k], GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10;
            writereg8bit(&lidar[k], GPIO_HV_MUX_ACTIVE_HIGH, value);
            writereg8bit(&lidar[k], SYSTEM_INTERRUPT_CLEAR, 0x01);

            lidar[k].stopvariablemicros = lidar_get_timing(&lidar[k]);
            writereg8bit(&lidar[k], SYSTEM_SEQUENCE_CONFIG, 0xE8);
            lidar_set_timing(&lidar[k], lidar[k].stopvariablemicros);

            writereg8bit(&lidar[k], SYSTEM_SEQUENCE_CONFIG, 0x01);
            if(!lidar_single_calibrate(&lidar[k], 0x40)) return false;

            writereg8bit(&lidar[k], SYSTEM_SEQUENCE_CONFIG, 0x02);
            if(!lidar_single_calibrate(&lidar[k], 0x00)) return false;

            writereg8bit(&lidar[k], SYSTEM_SEQUENCE_CONFIG, 0xE8);

            /* static config for lidar modules */
            // writereg16bit(&lidar[k], FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0.1 * (1 << 7)); 
            // lidar_set_pulseperiod(&lidar[k], VCSELPERIODFINALRANGE, 14);
            // lidar_set_pulseperiod(&lidar[k], VCSELPERIODPRERANGE, 18);
            lidar_set_timing(&lidar[k], 300 * 1000UL); 
            lidar->iotimeout = 100;
        }
        else continue;
    }
    return true;
}

bool lidar_spad_info(lidar_pin_config_t *lidar, uint8_t *count, bool *typeisaperture)
{
    uint8_t tmp = 0x00;

    writereg8bit(lidar, 0x80, 0x01);
    writereg8bit(lidar, 0xFF, 0x01);
    writereg8bit(lidar, 0x00, 0x00);

    writereg8bit(lidar, 0xFF, 0x06);
    tmp = readreg8bit(lidar, 0x83) | 0x04;
    writereg8bit(lidar, 0x83, tmp);
    writereg8bit(lidar, 0xFF, 0x07);
    
    writereg8bit(lidar, 0x81, 0x01);
    writereg8bit(lidar, 0x80, 0x01);
    writereg8bit(lidar, 0x94, 0x6B);
    writereg8bit(lidar, 0x83, 0x00);
    
    starttimeout(lidar);
    do
    {
        if(checktimeoutexpired(lidar))
        return false; 
    } 
    while (readreg8bit(lidar, 0x83) == 0x00);
    writereg8bit(lidar, 0x83, 0x01);
    tmp = readreg8bit(lidar, 0x92);
    *count = tmp & 0x7F;
    *typeisaperture = (tmp >> 7) & 0x01;

    writereg8bit(lidar, 0x81, 0x00);
    writereg8bit(lidar, 0xFF, 0x06);
    tmp = readreg8bit(lidar, 0x83) & ~0x04;
    writereg8bit(lidar, 0x83, tmp);
    writereg8bit(lidar, 0xFF, 0x01);
    
    writereg8bit(lidar, 0x00, 0x01);
    writereg8bit(lidar, 0xFF, 0x00);
    writereg8bit(lidar, 0x80, 0x00);

    return true;
}

uint32_t lidar_get_timing(lidar_pin_config_t *lidar)
{
    uint16_t const startoverhead        = 1910;
    uint16_t const endoverhead          = 960;
    uint16_t const msrcoverhead         = 660;
    uint16_t const tccoverhead          = 590;
    uint16_t const dssoverhead          = 690;
    uint16_t const prerangeoverhead     = 660;
    uint16_t const finalrangeoverhead   = 550;

    uint32_t budgetus = startoverhead + endoverhead;
    lidar_get_step(lidar); lidar_get_timeout(lidar);

    if(lidar->step.tcc) budgetus += (lidar->timeout.msrcdsstccus + tccoverhead);
    if(lidar->step.dss) budgetus += 2 * (lidar->timeout.msrcdsstccus + dssoverhead);
    else if(lidar->step.msrc) budgetus += (lidar->timeout.msrcdsstccus + msrcoverhead);
    if(lidar->step.prerange) budgetus += (lidar->timeout.prerangeus + prerangeoverhead);
    if(lidar->step.finalrange) budgetus += (lidar->timeout.finalrangeus + finalrangeoverhead);

    lidar->stopvariablemicros = budgetus;
    return budgetus;
}

bool lidar_set_timing(lidar_pin_config_t *lidar, uint32_t budgetus)
{
    uint16_t const startoverhead        = 1320;
    uint16_t const endofoverhead        = 960;
    uint16_t const msrcoverhead         = 660;
    uint16_t const tccoverhead          = 590;
    uint16_t const dssoverhead          = 690;
    uint16_t const prerangeoverhead     = 660;
    uint16_t const finalrangeoverhead   = 550;

    uint32_t const mintimingbudget = 20000;
    if(budgetus < mintimingbudget) return false;
    uint32_t usedbudgetus = startoverhead + endofoverhead;
    lidar_get_step(lidar); lidar_get_timeout(lidar);

    if(lidar->step.tcc) budgetus += (lidar->timeout.msrcdsstccus + tccoverhead);
    if(lidar->step.dss) budgetus += 2 * (lidar->timeout.msrcdsstccus + dssoverhead);
    else if(lidar->step.msrc) budgetus += (lidar->timeout.msrcdsstccus + msrcoverhead);
    if(lidar->step.prerange) budgetus += (lidar->timeout.prerangeus + prerangeoverhead);
    
    if(lidar->step.finalrange) 
    {
        budgetus += finalrangeoverhead;
        if(usedbudgetus > budgetus) return false;

        uint32_t finalrangetimeoutus    = budgetus - usedbudgetus;
        uint16_t finalrangetimeoutmclks = lidar_timeout_mclk(finalrangetimeoutus, lidar->timeout.finalrangevcselperiodclocks);

        if(lidar->step.prerange) finalrangetimeoutmclks += lidar->timeout.prerangemclks;
        uint16_t atimetoencode = lidar_time_encode(finalrangetimeoutmclks);
        writereg16bit(lidar, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, atimetoencode);

        lidar->stopvariablemicros = budgetus;
    }

    return true;
}

void lidar_get_step(lidar_pin_config_t *lidar)
{
    uint8_t sequenceconfig = readreg8bit(lidar, SYSTEM_SEQUENCE_CONFIG);

    lidar->step.tcc         = (sequenceconfig >> 4) & 0x01;
    lidar->step.dss         = (sequenceconfig >> 3) & 0x01;
    lidar->step.msrc        = (sequenceconfig >> 2) & 0x01;
    lidar->step.prerange    = (sequenceconfig >> 6) & 0x01;
    lidar->step.finalrange  = (sequenceconfig >> 7) & 0x01;
}

void lidar_get_timeout(lidar_pin_config_t *lidar)
{
    lidar->timeout.prerangevcselperiodclocks    = lidar_get_pulseperiod(lidar, VCSELPERIODPRERANGE);
    lidar->timeout.msrcdsstccmclks              = readreg8bit(lidar, MSRC_CONFIG_TIMEOUT_MACROP) + 1;
    lidar->timeout.msrcdsstccus                 = lidar_timeout_us(lidar->timeout.msrcdsstccmclks, lidar->timeout.prerangevcselperiodclocks);
    lidar->timeout.prerangemclks                = lidar_time_decode(readreg16bit(lidar, PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
    lidar->timeout.prerangeus                   = lidar_timeout_us(lidar->timeout.prerangemclks, lidar->timeout.prerangevcselperiodclocks);
    lidar->timeout.finalrangevcselperiodclocks  = lidar_get_pulseperiod(lidar, VCSELPERIODFINALRANGE);
    lidar->timeout.finalrangemmclks             = lidar_time_decode(readreg16bit(lidar, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

    if(lidar->step.prerange) lidar->timeout.finalrangemmclks -= lidar->timeout.prerangemclks;
    lidar->timeout.finalrangeus = lidar_timeout_us(lidar->timeout.finalrangemmclks, lidar->timeout.finalrangevcselperiodclocks);
}

uint16_t lidar_time_encode(uint16_t timeoutmclks)
{
    uint32_t lsbyte = 0;
    uint16_t msbyte = 0;

    if(timeoutmclks > 0)
    {
        lsbyte = timeoutmclks - 1;

        while((lsbyte & 0xFFFFFF00) > 0)
        {
            lsbyte >>= 1;
            msbyte++;
        }

        return (msbyte << 8) | (lsbyte & 0xFF);
    }
    return false;
}

uint16_t lidar_time_decode(uint16_t timeoutmclks)
{
    return (uint16_t)((timeoutmclks & 0x00FF) << 
    (uint16_t)((timeoutmclks & 0xFF00) >> 8)) + 1;
}

bool lidar_set_pulseperiod(lidar_pin_config_t *lidar, uint8_t type, uint8_t periodpclks)
{
    uint8_t vcselperiodreg = encodevscelperiod(periodpclks);
    lidar_get_step(lidar); lidar_get_timeout(lidar);

    if(type == VCSELPERIODPRERANGE)
    {
        switch(periodpclks)
        {
            case 12: writereg8bit(lidar, PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18); break;
            case 14: writereg8bit(lidar, PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30); break;
            case 16: writereg8bit(lidar, PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40); break;
            case 18: writereg8bit(lidar, PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50); break;
            default: return false; 
        }

        writereg8bit(lidar, PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
        writereg8bit(lidar, PRE_RANGE_CONFIG_VCSEL_PERIOD, vcselperiodreg);
        uint16_t newprerangetimeoutmclks = lidar_timeout_mclk(lidar->timeout.prerangeus, periodpclks);
        writereg16bit(lidar, PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, lidar_time_encode(newprerangetimeoutmclks));
        uint16_t newmsrctimeoutmclks = lidar_timeout_mclk(lidar->timeout.msrcdsstccus, periodpclks);
        writereg8bit(lidar, MSRC_CONFIG_TIMEOUT_MACROP, (newmsrctimeoutmclks > 256) ? 255 : (newmsrctimeoutmclks - 1));
    }
    else if(type == VCSELPERIODFINALRANGE)
    {
        switch(periodpclks)
        {
            case 8:
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
            writereg8bit(lidar, GLOBAL_CONFIG_VCSEL_WIDTH, 0x12);
            writereg8bit(lidar, ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
            writereg8bit(lidar, 0xFF, 0x01);
            writereg8bit(lidar, ALGO_PHASECAL_LIM, 0x30);
            writereg8bit(lidar, 0xFF, 0x00);
            break;

            case 10:
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
            writereg8bit(lidar, GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
            writereg8bit(lidar, ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
            writereg8bit(lidar, 0xFF, 0x01);
            writereg8bit(lidar, ALGO_PHASECAL_LIM, 0x20);
            writereg8bit(lidar, 0xFF, 0x00);
            break;

            case 12:
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
            writereg8bit(lidar, GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
            writereg8bit(lidar, ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
            writereg8bit(lidar, 0xFF, 0x01);
            writereg8bit(lidar, ALGO_PHASECAL_LIM, 0x20);
            writereg8bit(lidar, 0xFF, 0x00);
            break;

            case 14:
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
            writereg8bit(lidar, FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
            writereg8bit(lidar, GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
            writereg8bit(lidar, ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
            writereg8bit(lidar, 0xFF, 0x01);
            writereg8bit(lidar, ALGO_PHASECAL_LIM, 0x20);
            writereg8bit(lidar, 0xFF, 0x00);
            break;

            default: return false; 
        }

        writereg8bit(lidar, FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcselperiodreg);
        uint16_t newfinalrangetimeoutmclks = lidar_timeout_mclk(lidar->timeout.finalrangeus, periodpclks);
        if(lidar->step.prerange) newfinalrangetimeoutmclks += lidar->timeout.prerangemclks;
        writereg16bit(lidar, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, lidar_time_encode(newfinalrangetimeoutmclks));
    }
    else return false;

    lidar_set_timing(lidar, lidar->stopvariablemicros);
    uint8_t sequenceconfig = readreg8bit(lidar, SYSTEM_SEQUENCE_CONFIG);
    writereg8bit(lidar, SYSTEM_SEQUENCE_CONFIG, 0x02);
    lidar_single_calibrate(lidar, 0x00);
    writereg8bit(lidar, SYSTEM_SEQUENCE_CONFIG, sequenceconfig);

    return true;
}

uint8_t lidar_get_pulseperiod(lidar_pin_config_t *lidar, uint8_t type)
{
    switch(type)
    {
        case VCSELPERIODPRERANGE:
        return decodevscelperiod(readreg8bit(lidar, PRE_RANGE_CONFIG_VCSEL_PERIOD));

        case VCSELPERIODFINALRANGE:
        return decodevscelperiod(readreg8bit(lidar, FINAL_RANGE_CONFIG_VCSEL_PERIOD));

        default:
        return 255;
    }
}

bool lidar_single_calibrate(lidar_pin_config_t *lidar, uint8_t vhvinitbyte)
{
    writereg8bit(lidar, SYSRANGE_START, 0x01 | vhvinitbyte);
    
    starttimeout(lidar);
    do if(checktimeoutexpired(lidar)) return false;
    while((readreg8bit(lidar, RESULT_INTERRUPT_STATUS) & 0x07) == 0);

    writereg8bit(lidar, SYSTEM_INTERRUPT_CLEAR, 0x01);
    writereg8bit(lidar, SYSRANGE_START, 0x00);
    return true;
}

void lidar_start_continuous(lidar_pin_config_t *lidar, uint32_t periodms)
{
    writereg8bit(lidar, 0x80, 0x01);
    writereg8bit(lidar, 0xFF, 0x01);
    writereg8bit(lidar, 0x00, 0x00);
    writereg8bit(lidar, 0x91, lidar->stopvariablemillis);
    writereg8bit(lidar, 0x00, 0x01);
    writereg8bit(lidar, 0xFF, 0x00);
    writereg8bit(lidar, 0x80, 0x00);

    if(periodms != 0)
    {
        uint16_t osccalibrateval = readreg16bit(lidar, OSC_CALIBRATE_VAL);
        if(osccalibrateval != 0) periodms *= osccalibrateval;
        writereg32bit(lidar, SYSTEM_INTERMEASUREMENT_PERIOD, periodms);
        writereg8bit(lidar, SYSRANGE_START, 0x04);
    }
    else writereg8bit(lidar, SYSRANGE_START, 0x02);
}

void lidar_stop_continuous(lidar_pin_config_t *lidar)
{
    writereg8bit(lidar, SYSRANGE_START, 0x01);
    writereg8bit(lidar, 0xFF, 0x01);
    writereg8bit(lidar, 0x00, 0x00);
    writereg8bit(lidar, 0x91, 0x00);
    writereg8bit(lidar, 0x00, 0x01);
    writereg8bit(lidar, 0xFF, 0x00);
}

uint16_t lidar_read_oneshot(lidar_pin_config_t *lidar)
{
    writereg8bit(lidar, 0x80, 0x01);
    writereg8bit(lidar, 0xFF, 0x01);
    writereg8bit(lidar, 0x00, 0x00);
    writereg8bit(lidar, 0x91, lidar->stopvariablemillis);
    writereg8bit(lidar, 0x00, 0x01);
    writereg8bit(lidar, 0xFF, 0x00);
    writereg8bit(lidar, 0x80, 0x01);
    writereg8bit(lidar, SYSRANGE_START, 0x01);

    starttimeout(lidar);
    while(readreg8bit(lidar, SYSRANGE_START) & 0x01)
    {
        if(checktimeoutexpired(lidar))
        {
            lidar->istimeout = true;
            //return __UINT16_MAX__;
            return lidar->state.rawdistance;
            // return last known distance instead error distance 
        }
    }

    return lidar_read_continuous(lidar);
}

uint16_t lidar_read_continuous(lidar_pin_config_t *lidar)
{   
    starttimeout(lidar);
    while((readreg8bit(lidar, RESULT_INTERRUPT_STATUS) & 0x07) == 0)
    {
        if(checktimeoutexpired(lidar))
        {
            lidar->istimeout = true;
            //return __UINT16_MAX__;
            return lidar->state.rawdistance;
            // return last known distance instead error distance 
        }
    }

    uint8_t  tempbuffer[12];
    readsequence(lidar, 0x14, tempbuffer, 12);
    lidar->state.rangestatus  = tempbuffer[0x00] >> 3;
    lidar->state.spadcount    = (tempbuffer[0x02] << 8) | tempbuffer[0x03];
    lidar->state.signalcount  = (tempbuffer[0x06] << 8) | tempbuffer[0x07];
    lidar->state.ambientcount = (tempbuffer[0x08] << 8) | tempbuffer[0x09];
    lidar->state.rawdistance  = (tempbuffer[0x0A] << 8) | tempbuffer[0x0B];

    writereg8bit(lidar, SYSTEM_INTERRUPT_CLEAR, 0x01);
    return lidar->state.rawdistance;
}