#include "interface_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>

static InterfaceSnapshot s_last_snapshot;
static bool s_has_last = false;

static double get_perf_counter_ms(void) {
    static LARGE_INTEGER freq;
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart * 1000.0) / (double)freq.QuadPart;
}

void InterfaceStats_Init(void) {
    memset(&s_last_snapshot, 0, sizeof(s_last_snapshot));
    s_has_last = false;
}

void InterfaceStats_Poll(InterfaceSnapshot *out_snapshot) {
    if (!out_snapshot) return;
    memset(out_snapshot, 0, sizeof(InterfaceSnapshot));
    out_snapshot->timestamp_ms = get_perf_counter_ms();

    PMIB_IF_TABLE2 pIfTable = NULL;
    if (GetIfTable2(&pIfTable) != NO_ERROR || !pIfTable) {
        return;
    }

    int count = 0;
    for (ULONG i = 0; i < pIfTable->NumEntries && count < MAX_INTERFACES; i++) {
        PMIB_IF_ROW2 row = &pIfTable->Table[i];
        // Filter out tunnel and loopback if desired, but keep physical/wifi/ethernet
        if (row->Type == IF_TYPE_SOFTWARE_LOOPBACK) continue;

        NetworkInterfaceInfo *info = &out_snapshot->interfaces[count];
        info->if_index = row->InterfaceIndex;
        info->is_up = (row->OperStatus == IfOperStatusUp);
        info->in_octets = row->InOctets;
        info->out_octets = row->OutOctets;

        // Wide char description to UTF-8
        WideCharToMultiByte(CP_UTF8, 0, row->Description, -1, info->description, sizeof(info->description), NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, row->Alias, -1, info->name, sizeof(info->name), NULL, NULL);
        if (info->name[0] == '\0') {
            strncpy(info->name, info->description, sizeof(info->name) - 1);
        }

        // Calculate rate based on previous sample
        if (s_has_last) {
            double dt_sec = (out_snapshot->timestamp_ms - s_last_snapshot.timestamp_ms) / 1000.0;
            if (dt_sec > 0.1) {
                for (int j = 0; j < s_last_snapshot.count; j++) {
                    if (s_last_snapshot.interfaces[j].if_index == info->if_index) {
                        unsigned long long last_in = s_last_snapshot.interfaces[j].in_octets;
                        unsigned long long last_out = s_last_snapshot.interfaces[j].out_octets;
                        if (info->in_octets >= last_in) {
                            info->rx_rate_bps = (double)(info->in_octets - last_in) / dt_sec;
                            info->rx_rate_mbps = (info->rx_rate_bps * 8.0) / 1000000.0;
                        }
                        if (info->out_octets >= last_out) {
                            info->tx_rate_bps = (double)(info->out_octets - last_out) / dt_sec;
                            info->tx_rate_mbps = (info->tx_rate_bps * 8.0) / 1000000.0;
                        }
                        break;
                    }
                }
            }
        }

        count++;
    }

    out_snapshot->count = count;
    FreeMibTable(pIfTable);

    // Save as last snapshot
    memcpy(&s_last_snapshot, out_snapshot, sizeof(InterfaceSnapshot));
    s_has_last = true;
}
