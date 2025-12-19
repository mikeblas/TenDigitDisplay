#pragma once

extern uint8_t my_ip[4];

extern void (*on_wifi_address)(void);

void wifi_connection();
