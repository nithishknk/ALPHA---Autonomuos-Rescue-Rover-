/**
 * @file    nrf24l01.h
 * @author  saurash automations
 * @brief   nrf24 firmware for stm32
 * @version 0.1
 * @date    2025-03-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _NRF24_H
#define _NRF24_H

#include "../src/utils/main.h"
#include "../src/gpio/main.h"
#include "../src/spi/main.h"

#define CONFIG  		0x00
#define MASK_RX_DR 		(1 << 6)
#define MASK_TX_DS 		(1 << 5)
#define MASK_MAX_RT 	(1 << 4)
#define EN_CRC 			(1 << 3)
#define CRCO			(1 << 2)
#define PWR_UP			(1 << 1)
#define PRIM_RX			(1 << 0)

#define EN_AA 0x01
#define EN_RXADDR 0x02

#define SETUP_AW		0x03
#define AW_3_BYTE 		(1 << 1)
#define AW_4_BYTE		(1 << 2)
#define AW_5_BYTE		(3 << 0)

#define SETUP_RETR		0x04
#define RF_CH			0x05
#define SET_RF_CH 		(1 << 1)

#define RF_SETUP		0x06
#define CONT_WAVE		(1 << 6)
#define PLL_LOCK		(1 << 4)
#define RF_250KB  	    ((1 << 5)|(0 << 3))
#define RF_1MBPS    	((0 << 5)|(0 << 3))
#define RF_2MBPS    	((0 << 5)|(1 << 3))
#define RF_18DBM    	(0 << 1)
#define RF_12DBM    	(1 << 1)
#define RF_6DBM     	(2 << 1)
#define RF_0DBM     	(3 << 1)

#define STATUS 			0x07
#define RX_DR			(1 << 6)
#define TX_DS			(1 << 5)
#define MAX_RT			(1 << 4)
#define RX_P_NO			 1
#define TX_FULL			(1 << 0)

#define OBSERVE_TX		0x08
#define PLOS_CNT		4
#define ARC_CNT			0

#define RPD				0x09
#define CD				0

#define RX_ADDR_P0		0x0A
#define RX_ADDR_P1		0x0B
#define RX_ADDR_P2		0x0C
#define RX_ADDR_P3		0x0D
#define RX_ADDR_P4		0x0E
#define RX_ADDR_P5		0x0F

#define TX_ADDR			0x10
#define RX_PW_P0		0x11
#define RX_PW_P1		0x12
#define RX_PW_P2		0x13
#define RX_PW_P3		0x14
#define RX_PW_P4		0x15
#define RX_RW_P5		0x16

#define FIFO_STATUS		0x17
#define TX_REFUSE		(1 << 6)
#define TX_FULLL		(1 << 5)
#define TX_EMPTY		(1 << 4)
#define RX_FULL			(1 << 1)
#define RX_EMPTY		(1 << 0)

#define DYNPD			0x1C

#define FEATURE			0x1D
#define EN_DPL			(1 << 2)
#define EN_ACK_PAY		(1 << 1)
#define EN_DYN_ACK		(1 << 0)

#define R_REGISTER		0x00
#define W_REGISTER		0x20
#define R_RX_PAYLOAD	0x61
#define W_TX_PAYLOAD	0xA0
#define FLUSH_TX		0xE1
#define FLUSH_RX		0xE2
#define REUSE_TX_PL		0xE3
#define R_RX_PL_WID		0x60
#define W_ACK_PAYLOAD	0xA8
#define W_TX_NOACK		0xB0
#define REGISTER_MASK	0x1F
#define ACTIVATE		0x50
#define NOP 			0xFF

extern bool nrf_initialize(spi_inst_t *spi, gpio_pinmapping_t chipselect, gpio_pinmapping_t chipenable, uint8_t channel);
extern bool nrf_transmit(uint8_t *data, uint8_t *length);
extern bool nrf_receive(uint8_t *data, uint8_t *pipenumber);

extern void nrf_write_register(uint8_t reg, uint8_t *data, size_t length);
extern void nrf_read_register(uint8_t reg, uint8_t *data, size_t length);
extern void nrf_command_register(uint8_t reg, uint8_t *data, size_t length);
extern void nrf_mask_set(uint8_t reg, uint8_t data);
extern void nrf_mask_clear(uint8_t reg, uint8_t data);

extern uint8_t nrf_read_status(void);
extern uint8_t nrf_read_observe(void);
extern void nrf_flush_tx(void);
extern void nrf_flush_rx(void);
extern bool nrf_powerup(void);
extern void nrf_powerdown(void);

extern void nrf_set_autoack(uint8_t pipenumber, bool status);
extern void nrf_dynamic_payload(uint8_t pipenumber, bool status);
extern void nrf_enable_payload(bool status);
extern void nrf_write_payload(uint8_t pipenumber, uint8_t *data, size_t length);
extern void nrf_reuse_payload(void);

extern void nrf_mask_interrupt(bool status);
extern void nrf_clear_interrupt(void);
extern void nrf_transmit_timeout(uint16_t delay, uint8_t count);
extern void nrf_txr_address(uint8_t *address);
extern void nrf_rxr_address(uint8_t pipenumber, uint8_t *address);
#endif 