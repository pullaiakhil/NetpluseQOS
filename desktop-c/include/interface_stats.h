#ifndef INTERFACE_STATS_H
#define INTERFACE_STATS_H

#include <stdbool.h>

#define MAX_INTERFACES 32

typedef struct {
    char name[128];
    char description[256];
    unsigned long if_index;
    bool is_up;
    unsigned long long in_octets;
    unsigned long long out_octets;
    double rx_rate_bps; // Bytes per second
    double tx_rate_bps; // Bytes per second
    double rx_rate_mbps;
    double tx_rate_mbps;
} NetworkInterfaceInfo;

typedef struct {
    int count;
    NetworkInterfaceInfo interfaces[MAX_INTERFACES];
    double timestamp_ms;
} InterfaceSnapshot;

void InterfaceStats_Init(void);
void InterfaceStats_Poll(InterfaceSnapshot *out_snapshot);

#endif /* INTERFACE_STATS_H */
