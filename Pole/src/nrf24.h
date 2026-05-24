#ifndef _NRF24_H
#define _NRF24_H

#define config_1      0x00 
#define mask_rx_dr    (1 << 6)
#define mask_tx_ds    (1 << 5)
#define mask_max_rt   (1 << 4)
#define en_crc        (1 << 3)
#define crco          (1 << 2)
#define pwr_up        (1 << 1)
#define prim_rx       (1 << 0)

#define en_aa         0x01
#define en_rxaddr     0x02

#define setup_aw      0x03
#define aw_3_byte     (1 << 1)
#define aw_4_byte     (1 << 2)
#define aw_5_byte     (3 << 0)

#define setup_retr    0x04
#define rf_ch         0x05
#define set_rf_ch     (1 << 1)

#define rf_setup      0x06
#define cont_wave     (1 << 6)
#define pll_lock      (1 << 4)
#define rf_250kb      ((1 << 5) | (0 << 3))
#define rf_1mbps      ((0 << 5) | (0 << 3))
#define rf_2mbps      ((0 << 5) | (1 << 3))
#define rf_18dbm      (0 << 1)
#define rf_12dbm      (1 << 1)
#define rf_6dbm       (2 << 1)
#define rf_0dbm       (3 << 1)

#define status_reg     0x07
#define rx_dr         (1 << 6)
#define tx_ds         (1 << 5)
#define max_rt        (1 << 4)
#define rx_p_no       1
#define tx_full       (1 << 0)

#define observe_tx    0x08
#define plos_cnt      4
#define arc_cnt       0

#define rpd           0x09
#define cd            0

#define rx_addr_p0    0x0A
#define rx_addr_p1    0x0B
#define rx_addr_p2    0x0C
#define rx_addr_p3    0x0D
#define rx_addr_p4    0x0E
#define rx_addr_p5    0x0F

#define tx_addr       0x10
#define rx_pw_p0      0x11
#define rx_pw_p1      0x12
#define rx_pw_p2      0x13
#define rx_pw_p3      0x14
#define rx_pw_p4      0x15
#define rx_pw_p5      0x16

#define fifo_status   0x17
#define tx_reuse      (1 << 6)
#define tx_full       (1 << 5)
#define tx_empty      (1 << 4)
#define rx_full       (1 << 1)
#define rx_empty      (1 << 0)

#define dynpd         0x1C

#define feature       0x1D
#define en_dpl        (1 << 2)
#define en_ack_pay    (1 << 1)
#define en_dyn_ack    (1 << 0)

#define r_register     0x00
#define w_register     0x20
#define r_rx_payload   0x61
#define w_tx_payload   0xA0
#define flush_tx       0xE1
#define flush_rx       0xE2
#define reuse_tx_pl    0xE3
#define r_rx_pl_wid    0x60
#define w_ack_payload  0xA8
#define w_tx_noack     0xB0
#define register_mask  0x1F
#define activate       0x50
#define nop            0xFF

extern void nrf_write_register(unsigned char, unsigned char*, unsigned int);
extern void nrf_read_register(unsigned char, unsigned char*, unsigned int);
extern void nrf_command_register(unsigned char, unsigned char*, unsigned int);
extern void nrf_write_mask_set(unsigned char, unsigned char);
extern void nrf_write_mask_clr(unsigned char, unsigned char);

extern unsigned char nrf_initialize(unsigned char, unsigned char, unsigned char);
extern void nrf_flush_tx(void);
extern void nrf_flush_rx(void);
extern unsigned char nrf_read_status(void);
extern unsigned char nrf_read_observe(void);
extern unsigned char nrf_power_up(void);
extern void nrf_power_down(void);

extern void nrf_set_auto_ack(unsigned char, const char);
extern void nrf_dynamic_payload(unsigned char, const char);
extern void nrf_enable_ack_payload(const char);
extern void nrf_mask_interrupt(const char);
extern void nrf_clear_interrupt(void);
extern void nrf_reuse_payload(void);

extern void nrf_write_ack_payload(unsigned char, unsigned char*, unsigned int);
extern void nrf_retransmit(unsigned int, unsigned char);
extern void nrf_txr_address(unsigned char*);
extern void nrf_rxr_address(unsigned char, unsigned char*);
extern unsigned char nrf_transmit(unsigned char*, unsigned char*);
extern unsigned char nrf_receive(unsigned char*, unsigned char*);
#endif
