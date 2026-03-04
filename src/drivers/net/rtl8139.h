#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

void rtl8139_init(void);
int  rtl8139_send_packet(const void *data, uint16_t len);
int  rtl8139_receive_packet(void *buffer, uint16_t max_len);

#endif
