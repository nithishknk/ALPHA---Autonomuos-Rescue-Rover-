#include <avr/io.h>
#include <string.h>
#include <Arduino.h>

#include "gpio.h"
#include "utils.h"
#include "spi.h"
#include "nrf24.h"

unsigned char chipenable, chipselectnot;

void nrf_write_register(unsigned char reg, unsigned char *data, unsigned int length)
{
  gpio_put_low(chipselectnot);
  spi_write(w_register | (reg & register_mask));
  spi_write_sequence(data, length);
  gpio_put_high(chipselectnot);
}

void nrf_read_register(unsigned char reg, unsigned char *data, unsigned int length)
{
  gpio_put_low(chipselectnot);
  spi_write(r_register | (reg & register_mask));
  spi_read_write_sequence(data, length);
  gpio_put_high(chipselectnot);
}

void nrf_command_register(unsigned char reg, unsigned char *data, unsigned int length)
{
  gpio_put_low(chipselectnot);
  spi_write(reg);
  spi_read_write_sequence(data, length);
  gpio_put_high(chipselectnot);
}

void nrf_write_mask_set(unsigned char reg, unsigned char data)
{
  unsigned char localvariable;
  nrf_read_register(reg, &localvariable, 1);
  localvariable = localvariable | data;
  nrf_write_register(reg, &localvariable, 1);
}

void nrf_write_mask_clr(unsigned char reg, unsigned char data)
{
  unsigned char localvariable;
  nrf_read_register(reg, &localvariable, 1);
  localvariable = localvariable &~ data;
  nrf_write_register(reg, &localvariable, 1);
}

void nrf_flush_tx(void)
{
  gpio_put_low(chipselectnot);
  spi_write(flush_tx);
  gpio_put_high(chipselectnot);
}

void nrf_flush_rx(void)
{
  gpio_put_low(chipselectnot);
  spi_write(flush_rx);
  gpio_put_high(chipselectnot);
}

unsigned char nrf_read_status(void)
{
  unsigned char localvariable;
  gpio_put_low(chipselectnot);
  localvariable = spi_read_write(status_reg);
  gpio_put_high(chipselectnot);
  return localvariable;
}

unsigned char nrf_read_observe(void)
{
  unsigned char localvariable;
  gpio_put_low(chipselectnot);
  localvariable = spi_read_write(status_reg);
  gpio_put_high(chipselectnot);
  return localvariable;
}

unsigned char nrf_power_up(void)
{
  unsigned char localvariable;
  nrf_read_register(config_1, &localvariable, 1);
  if(!(localvariable & pwr_up)) 
  {
    nrf_write_mask_set(config_1, pwr_up);
    delay_us(130);
  }
  nrf_read_register(config_1, &localvariable, 1);
  if(localvariable & pwr_up) return true;
  return false;
}

void nrf_power_down(void)
{
  gpio_put_low(chipenable);
  nrf_write_mask_clr(config_1, pwr_up);
  delay_us(130);
}

void nrf_set_auto_ack(unsigned char pipe, const char status)
{
  if(status) nrf_write_mask_set(en_aa, (1 << pipe));
  else nrf_write_mask_clr(en_aa, (1 << pipe));
}

void nrf_dynamic_payload(unsigned char pipe, const char status)
{
  if(status) nrf_write_mask_set(dynpd, (1 << pipe));
  else nrf_write_mask_clr(dynpd, (1 << pipe));
}

void nrf_enable_ack_payload(const char status)
{
  if(status) nrf_write_mask_set(feature, en_ack_pay);
  else nrf_write_mask_clr(feature, en_ack_pay);
}

void nrf_mask_interrupt(const char status)
{
  unsigned char localvariable = (mask_rx_dr | mask_tx_ds | mask_max_rt);
  if(status) nrf_write_mask_clr(config_1, localvariable);
  else nrf_write_mask_set(config_1, localvariable);
}

void nrf_clear_interrupt(void)
{
  nrf_write_mask_set(status_reg, rx_dr);
  nrf_write_mask_set(status_reg, tx_ds);
  nrf_write_mask_set(status_reg, max_rt);
}

void nrf_write_ack_payload(unsigned char pipe, unsigned char *data, unsigned int length)
{
  nrf_set_auto_ack(pipe, true);
  nrf_enable_ack_payload(true);
  nrf_write_mask_set(en_rxaddr, (1 << pipe));
  nrf_command_register(w_ack_payload | pipe, data, length);
}

void nrf_reuse_payload(void)
{
  nrf_clear_interrupt();
  gpio_put_low(chipselectnot);
  spi_write(reuse_tx_pl);
  gpio_put_high(chipselectnot);
  gpio_put_low(chipenable);
  delay_us(10);
  gpio_put_high(chipenable);
}

void nrf_retransmit(unsigned int delay, unsigned char count)
{
  uint8_t localvariable = delay - 250;
  if(localvariable) delay = delay / 250;
  nrf_write_mask_set(setup_retr, (((localvariable & 0x0F) << 4) | (count & 0x0F)));
}

void nrf_txr_address(unsigned char *address)
{
  unsigned char localvariable = 32;
  nrf_write_register(tx_addr, address, 5);
  nrf_write_register(rx_addr_p0, address, 5);
  nrf_write_register(rx_pw_p0, &localvariable, 1);
}

void nrf_rxr_address(unsigned char pipe, unsigned char *address)
{
  unsigned char localvariable = 32;
  nrf_write_register(rx_addr_p0 | pipe, address, 5);
  nrf_write_register(rx_pw_p0 | pipe, &localvariable, 1);
  nrf_write_mask_set(en_rxaddr, (1 << pipe));
}

unsigned char nrf_transmit(unsigned char *data, unsigned char *length)
{
  if(!nrf_power_up()) return false;
  nrf_clear_interrupt();
  unsigned char flength = length[0];
  unsigned char status = false;

  nrf_write_mask_set(en_rxaddr, 0x00);
  nrf_write_mask_clr(config_1, prim_rx);
  nrf_flush_tx();
  nrf_command_register(w_tx_payload, data, flength);

  gpio_put_high(chipenable);
  _delay_us(15);
  gpio_put_low(chipenable);
  _delay_us(130);

  unsigned int localtimer = 100;
  do
  {
    if(nrf_read_status() & tx_ds) break;
    if(nrf_read_status() & max_rt) return false;
    _delay_ms(1);
  }
  while(localtimer --> 0);
  if(!localtimer) return false;

  unsigned char localvariable = nrf_read_status();
  if(localvariable & rx_dr)
  {
    memset(data, '\0', flength);
    *length = ((localvariable &~ 0xF1) >> 1);
    nrf_command_register(r_rx_pl_wid, &localvariable, 1);
    nrf_command_register(r_rx_payload, data, localvariable);
    nrf_flush_rx(); 
  }
  nrf_power_down();
  nrf_flush_tx();
  return true;
}

unsigned char nrf_receive(unsigned char *data, unsigned char *pipenumber)
{
  if(!nrf_power_up()) return false;
  unsigned char pipe = pipenumber[0];
  nrf_write_mask_set(en_rxaddr, (1 << pipe));
  nrf_write_mask_set(config_1, prim_rx);
  gpio_put_high(chipenable); _delay_us(130);

  unsigned char localvariable = nrf_read_status();
  if(localvariable & rx_dr)
  {
    *pipenumber = ((localvariable &~ 0xF1) >> 1);
    nrf_command_register(r_rx_pl_wid, &localvariable, 1);
    nrf_command_register(r_rx_payload, data, localvariable);
    nrf_power_down();
    nrf_clear_interrupt();
    nrf_flush_tx();
    nrf_flush_rx();
    return true;
  }
  return false;
}

unsigned char nrf_initialize(unsigned char cs, unsigned char csn, unsigned char freq)
{
  chipenable = cs;
  chipselectnot = csn;
  gpio_set_output(chipenable);
  gpio_put_low(chipenable);
  gpio_set_output(chipselectnot);
  gpio_put_high(chipselectnot); 
  delay_ms(5); 

  nrf_write_mask_set(rf_ch, (set_rf_ch | freq));
  nrf_write_mask_set(rf_setup, (rf_250kb | rf_0dbm));
  nrf_mask_interrupt(false);
  nrf_clear_interrupt();
  
  if(!nrf_power_up()) return false;
  nrf_write_mask_set(config_1, (en_crc | crco | prim_rx));
  nrf_flush_tx();
  nrf_flush_rx();

  nrf_write_mask_set(feature, en_dpl);
  nrf_retransmit(4000, 15);
  nrf_write_mask_set(en_rxaddr, 0x00);
  nrf_write_mask_set(en_aa, 0x3F);
  nrf_write_mask_set(dynpd, 0x3F);
  nrf_enable_ack_payload(true);
  return true;
}
