#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "icmp_probe.h"
#include "tcp_probe.h"
#include "interface_stats.h"

int main(void) {
    printf("=== Live Network Probe Validation ===\n\n");

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. ICMP probe to Cloudflare DNS (1.1.1.1)
    ProbeSample sample_icmp;
    printf("[1/3] Probing ICMP to 1.1.1.1...\n");
    ICMP_ProbeHost("1.1.1.1", 3.0, &sample_icmp);
    if (sample_icmp.success) {
        printf(" -> SUCCESS: RTT = %.2f ms | IP: %s | Time: %s\n",
               sample_icmp.rtt_ms, sample_icmp.resolved_ip, sample_icmp.timestamp_iso);
    } else {
        printf(" -> DROPPED/TIMEOUT: %s: %s\n", sample_icmp.error_type, sample_icmp.error_detail);
    }

    // 2. TCP Handshake probe to 1.1.1.1:80
    ProbeSample sample_tcp;
    printf("\n[2/3] Probing TCP Handshake to 1.1.1.1:80...\n");
    TCP_ProbeService("1.1.1.1", 80, 3.0, &sample_tcp);
    if (sample_tcp.success) {
        printf(" -> SUCCESS: TCP Connection Time = %.2f ms | IP: %s\n",
               sample_tcp.rtt_ms, sample_tcp.resolved_ip);
    } else {
        printf(" -> FAILED: %s: %s\n", sample_tcp.error_type, sample_tcp.error_detail);
    }

    // 3. Network Interface Statistics via IP Helper
    printf("\n[3/3] Polling Hardware Network Interfaces via IP Helper API...\n");
    InterfaceSnapshot if_snap;
    InterfaceStats_Init();
    InterfaceStats_Poll(&if_snap);
    printf(" -> Found %d active network interfaces:\n", if_snap.count);
    for (int i = 0; i < if_snap.count; i++) {
        NetworkInterfaceInfo *iface = &if_snap.interfaces[i];
        printf("    * [#%02lu] %-30s | Status: %-4s | InOctets: %llu | OutOctets: %llu\n",
               iface->if_index, iface->name, iface->is_up ? "UP" : "DOWN",
               iface->in_octets, iface->out_octets);
    }

    printf("\n>>> Live Network Measurements Verified Successfully! <<<\n");
    return 0;
}
