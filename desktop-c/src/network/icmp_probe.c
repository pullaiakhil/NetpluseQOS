#include "icmp_probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

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

static void get_current_iso_time(char *buf, size_t maxlen) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = gmtime(&rawtime);
    if (timeinfo) {
        strftime(buf, maxlen, "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    } else {
        strncpy(buf, "1970-01-01T00:00:00Z", maxlen);
    }
}

void ICMP_ProbeHost(const char *host, double timeout_sec, ProbeSample *out_sample) {
    if (!out_sample) return;
    memset(out_sample, 0, sizeof(ProbeSample));
    out_sample->success = false;
    out_sample->rtt_ms = -1.0;
    get_current_iso_time(out_sample->timestamp_iso, sizeof(out_sample->timestamp_iso));

    if (!host || host[0] == '\0') {
        strncpy(out_sample->error_type, "INVALID_TARGET", sizeof(out_sample->error_type) - 1);
        strncpy(out_sample->error_detail, "Target hostname or IP is empty", sizeof(out_sample->error_detail) - 1);
        return;
    }

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 for IcmpSendEcho
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, NULL, &hints, &res) != 0 || !res) {
        strncpy(out_sample->error_type, "DNS_FAILURE", sizeof(out_sample->error_type) - 1);
        snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Could not resolve hostname '%s'", host);
        return;
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), out_sample->resolved_ip, sizeof(out_sample->resolved_ip));
    unsigned long ip_addr = ipv4->sin_addr.s_addr;
    freeaddrinfo(res);

    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE) {
        strncpy(out_sample->error_type, "ICMP_HANDLE_ERROR", sizeof(out_sample->error_type) - 1);
        snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "IcmpCreateFile failed: %lu", GetLastError());
        return;
    }

    char send_data[32] = "NetPulseQoSEcho";
    DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + sizeof(send_data) + 16;
    void *reply_buffer = malloc(reply_size);
    if (!reply_buffer) {
        IcmpCloseHandle(hIcmp);
        strncpy(out_sample->error_type, "OUT_OF_MEMORY", sizeof(out_sample->error_type) - 1);
        strncpy(out_sample->error_detail, "Memory allocation failed for ICMP reply", sizeof(out_sample->error_detail) - 1);
        return;
    }

    DWORD timeout_ms = (DWORD)(timeout_sec * 1000.0);
    if (timeout_ms < 100) timeout_ms = 100;

    double t_start = get_perf_counter_ms();
    DWORD dwRetVal = IcmpSendEcho(
        hIcmp,
        ip_addr,
        send_data,
        (WORD)sizeof(send_data),
        NULL,
        reply_buffer,
        reply_size,
        timeout_ms
    );
    double elapsed_ms = get_perf_counter_ms() - t_start;

    if (dwRetVal > 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)reply_buffer;
        if (pEchoReply->Status == IP_SUCCESS) {
            out_sample->success = true;
            // RoundTripTime is in milliseconds from the ICMP API; use high-res timer if reported as 0ms (<1ms)
            out_sample->rtt_ms = (pEchoReply->RoundTripTime > 0) ? (double)pEchoReply->RoundTripTime : elapsed_ms;
        } else if (pEchoReply->Status == IP_REQ_TIMED_OUT) {
            strncpy(out_sample->error_type, "TIMEOUT", sizeof(out_sample->error_type) - 1);
            strncpy(out_sample->error_detail, "ICMP Echo Request timed out", sizeof(out_sample->error_detail) - 1);
        } else if (pEchoReply->Status == IP_DEST_HOST_UNREACHABLE) {
            strncpy(out_sample->error_type, "HOST_UNREACHABLE", sizeof(out_sample->error_type) - 1);
            strncpy(out_sample->error_detail, "Destination host unreachable", sizeof(out_sample->error_detail) - 1);
        } else {
            strncpy(out_sample->error_type, "ICMP_ERROR", sizeof(out_sample->error_type) - 1);
            snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "ICMP Echo failed status %lu", pEchoReply->Status);
        }
    } else {
        DWORD err = GetLastError();
        if (err == IP_REQ_TIMED_OUT) {
            strncpy(out_sample->error_type, "TIMEOUT", sizeof(out_sample->error_type) - 1);
            snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Request timed out after %.2fs", timeout_sec);
        } else {
            strncpy(out_sample->error_type, "ICMP_SEND_FAILED", sizeof(out_sample->error_type) - 1);
            snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "IcmpSendEcho error: %lu", err);
        }
    }

    free(reply_buffer);
    IcmpCloseHandle(hIcmp);
}
