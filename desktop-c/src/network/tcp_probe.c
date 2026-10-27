#include "tcp_probe.h"
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

void TCP_ProbeService(const char *host, int port, double timeout_sec, ProbeSample *out_sample) {
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

    if (port <= 0 || port > 65535) {
        strncpy(out_sample->error_type, "INVALID_PORT", sizeof(out_sample->error_type) - 1);
        strncpy(out_sample->error_detail, "TCP Port must be between 1 and 65535", sizeof(out_sample->error_detail) - 1);
        return;
    }

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res) {
        strncpy(out_sample->error_type, "DNS_FAILURE", sizeof(out_sample->error_type) - 1);
        snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Could not resolve hostname '%s'", host);
        return;
    }

    void *addr_ptr = NULL;
    if (res->ai_family == AF_INET) {
        addr_ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    } else if (res->ai_family == AF_INET6) {
        addr_ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
    }
    if (addr_ptr) {
        inet_ntop(res->ai_family, addr_ptr, out_sample->resolved_ip, sizeof(out_sample->resolved_ip));
    }

    SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        strncpy(out_sample->error_type, "SOCKET_ERROR", sizeof(out_sample->error_type) - 1);
        snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Failed creating socket: err=%d", WSAGetLastError());
        freeaddrinfo(res);
        return;
    }

    u_long non_blocking = 1;
    ioctlsocket(sock, FIONBIO, &non_blocking);

    double t_start = get_perf_counter_ms();
    int conn_res = connect(sock, res->ai_addr, (int)res->ai_addrlen);

    if (conn_res == SOCKET_ERROR) {
        int wsa_err = WSAGetLastError();
        if (wsa_err == WSAEWOULDBLOCK) {
            fd_set write_fds, err_fds;
            FD_ZERO(&write_fds);
            FD_ZERO(&err_fds);
            FD_SET(sock, &write_fds);
            FD_SET(sock, &err_fds);

            struct timeval tv;
            tv.tv_sec = (long)timeout_sec;
            tv.tv_usec = (long)((timeout_sec - (double)tv.tv_sec) * 1000000.0);

            int sel_res = select((int)(sock + 1), NULL, &write_fds, &err_fds, &tv);
            if (sel_res > 0) {
                if (FD_ISSET(sock, &write_fds)) {
                    int sock_err = 0;
                    int opt_len = sizeof(sock_err);
                    getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&sock_err, &opt_len);
                    if (sock_err == 0) {
                        out_sample->rtt_ms = get_perf_counter_ms() - t_start;
                        out_sample->success = true;
                    } else if (sock_err == WSAECONNREFUSED) {
                        strncpy(out_sample->error_type, "CONNECTION_REFUSED", sizeof(out_sample->error_type) - 1);
                        strncpy(out_sample->error_detail, "Connection refused by target port", sizeof(out_sample->error_detail) - 1);
                    } else {
                        strncpy(out_sample->error_type, "CONNECT_FAILED", sizeof(out_sample->error_type) - 1);
                        snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Socket connect error %d", sock_err);
                    }
                } else {
                    strncpy(out_sample->error_type, "CONNECT_FAILED", sizeof(out_sample->error_type) - 1);
                    strncpy(out_sample->error_detail, "Socket exception during connection", sizeof(out_sample->error_detail) - 1);
                }
            } else if (sel_res == 0) {
                strncpy(out_sample->error_type, "TIMEOUT", sizeof(out_sample->error_type) - 1);
                snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "TCP connection timed out after %.2fs", timeout_sec);
            } else {
                strncpy(out_sample->error_type, "SELECT_ERROR", sizeof(out_sample->error_type) - 1);
                snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Select failed err=%d", WSAGetLastError());
            }
        } else if (wsa_err == WSAECONNREFUSED) {
            strncpy(out_sample->error_type, "CONNECTION_REFUSED", sizeof(out_sample->error_type) - 1);
            strncpy(out_sample->error_detail, "Connection refused by target port", sizeof(out_sample->error_detail) - 1);
        } else {
            strncpy(out_sample->error_type, "CONNECT_FAILED", sizeof(out_sample->error_type) - 1);
            snprintf(out_sample->error_detail, sizeof(out_sample->error_detail), "Connect failed err=%d", wsa_err);
        }
    } else {
        out_sample->rtt_ms = get_perf_counter_ms() - t_start;
        out_sample->success = true;
    }

    closesocket(sock);
    freeaddrinfo(res);
}
