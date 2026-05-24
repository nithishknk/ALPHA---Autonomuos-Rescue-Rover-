#include "../src/nrf24/main.h"

spi_inst_t *nrf;
gpio_pinmapping_t csn;
gpio_pinmapping_t ce;

void nrf_write_register(uint8_t reg, uint8_t *data, size_t length)
{
    gpio_put_low(csn);
    spi_write(nrf, W_REGISTER | (reg & REGISTER_MASK));
    spi_read_write_sequence(nrf, data, length);
    gpio_put_high(csn);
}

void nrf_read_register(uint8_t reg, uint8_t *data, size_t length)
{
    gpio_put_low(csn);
    spi_write(nrf, R_REGISTER | (reg & REGISTER_MASK));
    spi_read_write_sequence(nrf, data, length);
    gpio_put_high(csn);
}

void nrf_command_register(uint8_t reg, uint8_t *data, size_t length)
{
    gpio_put_low(csn);
    spi_write(nrf, reg);
    spi_read_write_sequence(nrf, data, length);
    gpio_put_high(csn);
}

void nrf_mask_set(uint8_t reg, uint8_t data)
{
    uint8_t localvariable = 0;
    nrf_read_register(reg, &localvariable, 1);
    localvariable = localvariable | data;
    nrf_write_register(reg, &localvariable, 1);
}

void nrf_mask_clear(uint8_t reg, uint8_t data)
{
    uint8_t localvariable = 0;
    nrf_read_register(reg, &localvariable, 1);
    localvariable = localvariable & ~data;
    nrf_write_register(reg, &localvariable, 1);
}

void nrf_flush_tx(void)
{
    gpio_put_low(csn);
    spi_write(nrf, FLUSH_TX);
    gpio_put_high(csn);
}

void nrf_flush_rx(void)
{
    gpio_put_low(csn);
    spi_write(nrf, FLUSH_RX);
    gpio_put_high(csn);
}

uint8_t nrf_read_status(void)
{
    uint8_t localvariable = 0;
    gpio_put_low(csn);
    localvariable = spi_read_write(nrf, STATUS);
    gpio_put_high(csn);
    return localvariable;
}

uint8_t nrf_read_observe(void)
{
    uint8_t localvariable = 0;
    gpio_put_low(csn);
    localvariable = spi_read_write(nrf, OBSERVE_TX);
    gpio_put_high(csn);
    return localvariable;
}

bool nrf_powerup(void)
{
    uint8_t localvariable = 0;
    nrf_read_register(CONFIG, &localvariable, 1);

    if(!(localvariable & PWR_UP))
    {
        nrf_mask_set(CONFIG, PWR_UP);
        delay_ms(5);
    }

    nrf_read_register(CONFIG, &localvariable, 1);
    if(localvariable & PWR_UP) return true;
    return false;
}

void nrf_powerdown(void)
{
    uint8_t localvariable = 0;
    nrf_read_register(CONFIG, &localvariable, 1);

    if(localvariable & PWR_UP)
    {
        nrf_mask_clear(CONFIG, PWR_UP);
        delay_ms(5);
    }
    gpio_put_low(ce);
}

void nrf_set_autoack(uint8_t pipenumber, bool status)
{
    if(status) nrf_mask_set(EN_AA, (pipenumber < 5 ? (1 << pipenumber) : 0x3F));
    else nrf_mask_clear(EN_AA, (pipenumber < 5 ? (1 << pipenumber) : 0x3C));
}

void nrf_dynamic_payload(uint8_t pipenumber, bool status)
{
    if(status) nrf_mask_set(DYNPD, (pipenumber < 5 ? (1 << pipenumber) : 0x3F));
    else nrf_mask_set(DYNPD, (pipenumber < 5 ? (1 << pipenumber) : 0x3F));
}

void nrf_enable_payload(bool status)
{
    if(status) nrf_mask_set(FEATURE, EN_ACK_PAY);
    else nrf_mask_clear(FEATURE, EN_ACK_PAY);
}

void nrf_write_payload(uint8_t pipenumber, uint8_t *data, size_t length)
{
    nrf_set_autoack(pipenumber, true);
    nrf_enable_payload(true);
    nrf_mask_set(EN_RXADDR, (1 << pipenumber));
    nrf_command_register(W_ACK_PAYLOAD | pipenumber, data, length);
}

void nrf_reuse_payload(void)
{
    nrf_clear_interrupt();

    gpio_put_low(csn);
    spi_write(nrf, REUSE_TX_PL);
    gpio_put_high(csn);

    gpio_put_low(ce);
    delay_us(15);
    gpio_put_high(ce);
    delay_us(130);
}

void nrf_mask_interrupt(bool status)
{
    if(status) nrf_mask_clear(CONFIG, ((MASK_RX_DR) | (MASK_TX_DS) | (MASK_MAX_RT)));
    else nrf_mask_set(CONFIG, ((MASK_RX_DR) | (MASK_TX_DS) | (MASK_MAX_RT)));
}

void nrf_clear_interrupt(void)
{
    nrf_mask_set(STATUS, ((RX_DR) | (TX_DS) | (MAX_RT)));
}

void nrf_txr_address(uint8_t *address)
{
    nrf_write_register(TX_ADDR, address, 5);
    nrf_write_register(RX_ADDR_P0, address, 5);
    nrf_write_register(RX_PW_P0, &(uint8_t){32}, 1);
}

void nrf_rxr_address(uint8_t pipenumber, uint8_t *address)
{
    nrf_write_register(RX_PW_P0 | pipenumber, &(uint8_t){32}, 1);
    nrf_write_register(RX_ADDR_P0 | pipenumber, address, 5);
    nrf_mask_set(EN_RXADDR, (1 << pipenumber) | 0b11);
}

void nrf_transmit_timeout(uint16_t delay, uint8_t count)
{
    uint8_t localvariable = delay - 250;
    if(localvariable) delay = delay / 250;
    nrf_mask_set(SETUP_RETR, (((localvariable & 0x0F) << 4) | (count & 0x0F)));
}

bool nrf_transmit(uint8_t *data, uint8_t *length)
{
    if(!nrf_powerup()) return false;
    nrf_clear_interrupt();

    nrf_mask_set(EN_RXADDR, 0x00);
    nrf_mask_clear(CONFIG, PRIM_RX);

    nrf_flush_tx();
    nrf_command_register(W_TX_PAYLOAD, data, length[0]);
    gpio_put_high(ce); delay_us(15);
    gpio_put_low(ce); delay_us(130);

    uint8_t localvariable = 100;
    do
    {
        if(nrf_read_status() & TX_DS) break;
        if(nrf_read_status() & MAX_RT) return false;
        delay_ms(1);
    }
    while(localvariable --> 0);
    localvariable = nrf_read_status();

    if(localvariable & RX_DR)
    {
        memset(data, '\0', length[0]);
        *length = ((localvariable & ~0xF1) >> 1);
        nrf_command_register(R_RX_PL_WID, &localvariable, 1);
        nrf_command_register(R_RX_PAYLOAD, data, localvariable);
        nrf_flush_tx();
    }

    nrf_powerdown();
    nrf_flush_tx();
    return true;
}

bool nrf_receive(uint8_t *data, uint8_t *pipenumber)
{
    if(!nrf_powerup()) return false;

    nrf_mask_set(EN_RXADDR, (1 << (pipenumber[0])));
    nrf_mask_set(CONFIG, PRIM_RX);

    gpio_put_high(ce); delay_us(150);
    uint8_t localvariable = nrf_read_status();

    if(localvariable & RX_DR)
    {
        *pipenumber = ((localvariable &~ 0xF1) >> 1);
        nrf_command_register(R_RX_PL_WID, &localvariable, 1);
        nrf_command_register(R_RX_PAYLOAD, data, localvariable);
        nrf_powerdown(); nrf_clear_interrupt(); nrf_flush_rx();
        return true;
    }
    return false;
}

bool nrf_initialize(spi_inst_t *spi, gpio_pinmapping_t chipselect, gpio_pinmapping_t chipenable, uint8_t channel)
{
    nrf = spi; csn = chipselect; ce = chipenable;
    gpio_set_output(csn); gpio_put_high(csn);
    gpio_set_output(ce);  gpio_put_low(ce);
    delay_ms(5);

    nrf_mask_set(RF_CH, (SET_RF_CH | channel));
    nrf_mask_set(RF_SETUP, (RF_250KB | RF_0DBM));

    nrf_mask_interrupt(false);
    nrf_clear_interrupt(); if(!nrf_powerup()) return false;
    nrf_mask_set(CONFIG, (EN_CRC | CRCO | PRIM_RX));
    nrf_flush_rx(); nrf_flush_tx();

    nrf_mask_set(FEATURE, EN_DPL); nrf_mask_set(EN_RXADDR, 0x00);
    nrf_set_autoack(6, true); nrf_dynamic_payload(6, true);
    nrf_enable_payload(true); nrf_transmit_timeout(4000, 15);

    return true;
}