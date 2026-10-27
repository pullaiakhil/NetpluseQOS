#include "monitor.h"
#include "icmp_probe.h"
#include "tcp_probe.h"
#include "alerts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h>

MonitorController g_monitor;
static HWND s_hwndNotify = NULL;
static HANDLE s_hThread = NULL;
static HANDLE s_hStopEvent = NULL;

static unsigned __stdcall MonitorWorkerThread(void *param) {
    (void)param;
    int seq = 1;

    while (WaitForSingleObject(s_hStopEvent, 0) == WAIT_TIMEOUT) {
        DbTarget target;
        int session_id = 0;
        double interval_sec = 1.0;
        char profile[32] = {0};

        EnterCriticalSection(&g_monitor.cs);
        target = g_monitor.active_target;
        session_id = g_monitor.active_session_id;
        interval_sec = g_monitor.interval_seconds;
        strncpy(profile, g_monitor.qos_profile, sizeof(profile) - 1);
        LeaveCriticalSection(&g_monitor.cs);

        if (session_id <= 0) break;

        // Perform genuine probe outside lock
        ProbeSample sample;
        sample.sequence = seq++;
        double timeout_sec = (interval_sec > 1.5) ? (interval_sec * 0.8) : 1.5;

        if (strcmp(target.target_type, "tcp") == 0) {
            TCP_ProbeService(target.host, target.port, timeout_sec, &sample);
        } else {
            ICMP_ProbeHost(target.host, timeout_sec, &sample);
        }

        // Poll interface statistics
        InterfaceSnapshot if_snap;
        InterfaceStats_Poll(&if_snap);

        // Store sample in DB
        DB_InsertProbe(session_id, &sample);

        // Evaluate alerts
        Alerts_EvaluateProbe(session_id, sample.rtt_ms, sample.success);

        // Lock and update shared state
        EnterCriticalSection(&g_monitor.cs);
        if (g_monitor.sample_count < MAX_SAMPLES_BUFFER) {
            g_monitor.samples[g_monitor.sample_count++] = sample;
        } else {
            // Shift left
            memmove(&g_monitor.samples[0], &g_monitor.samples[1], sizeof(ProbeSample) * (MAX_SAMPLES_BUFFER - 1));
            g_monitor.samples[MAX_SAMPLES_BUFFER - 1] = sample;
        }

        g_monitor.latest_rtt_ms = sample.rtt_ms;
        g_monitor.latest_success = sample.success;
        strncpy(g_monitor.last_sample_time, sample.timestamp_iso, sizeof(g_monitor.last_sample_time) - 1);
        if (!sample.success) {
            snprintf(g_monitor.latest_error, sizeof(g_monitor.latest_error), "%s: %s", sample.error_type, sample.error_detail);
        } else {
            g_monitor.latest_error[0] = '\0';
        }

        // Recalculate metrics & QoS assessment
        QoS_ComputeMetrics(g_monitor.samples, g_monitor.sample_count, &g_monitor.current_metrics);
        QoS_EvaluateProfile(&g_monitor.current_metrics, profile, &g_monitor.current_assessment);
        g_monitor.current_if_stats = if_snap;
        LeaveCriticalSection(&g_monitor.cs);

        // Safely notify UI window thread via PostMessage
        if (s_hwndNotify && IsWindow(s_hwndNotify)) {
            PostMessage(s_hwndNotify, WM_NETPULSE_SAMPLE_UPDATE, (WPARAM)session_id, 0);
        }

        // Sleep with interruptible wait on stop event
        DWORD wait_ms = (DWORD)(interval_sec * 1000.0);
        if (wait_ms < 200) wait_ms = 200;
        if (WaitForSingleObject(s_hStopEvent, wait_ms) != WAIT_TIMEOUT) {
            break;
        }
    }

    return 0;
}

void Monitor_Init(HWND hwndNotify) {
    s_hwndNotify = hwndNotify;
    memset(&g_monitor, 0, sizeof(MonitorController));
    InitializeCriticalSection(&g_monitor.cs);
    s_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    InterfaceStats_Init();
    Alerts_Init();
}

void Monitor_Cleanup(void) {
    Monitor_Stop();
    if (s_hStopEvent) {
        CloseHandle(s_hStopEvent);
        s_hStopEvent = NULL;
    }
    DeleteCriticalSection(&g_monitor.cs);
}

bool Monitor_Start(const DbTarget *target, double interval_sec, const char *profile) {
    if (!target) return false;
    Monitor_Stop();

    int session_id = 0;
    if (!DB_CreateSession(target->id, interval_sec, profile, &session_id)) {
        return false;
    }

    EnterCriticalSection(&g_monitor.cs);
    g_monitor.is_monitoring = true;
    g_monitor.active_session_id = session_id;
    g_monitor.active_target = *target;
    g_monitor.interval_seconds = (interval_sec < 0.2) ? 1.0 : interval_sec;
    strncpy(g_monitor.qos_profile, profile ? profile : "video_conferencing", sizeof(g_monitor.qos_profile) - 1);
    g_monitor.sample_count = 0;
    memset(&g_monitor.current_metrics, 0, sizeof(QoSMetrics));
    memset(&g_monitor.current_assessment, 0, sizeof(QoSAssessmentResult));
    g_monitor.latest_rtt_ms = -1.0;
    g_monitor.latest_success = false;
    g_monitor.latest_error[0] = '\0';
    LeaveCriticalSection(&g_monitor.cs);

    ResetEvent(s_hStopEvent);
    s_hThread = (HANDLE)_beginthreadex(NULL, 0, MonitorWorkerThread, NULL, 0, NULL);
    if (!s_hThread) {
        g_monitor.is_monitoring = false;
        DB_EndSession(session_id);
        return false;
    }

    if (s_hwndNotify && IsWindow(s_hwndNotify)) {
        PostMessage(s_hwndNotify, WM_NETPULSE_SESSION_STATE, (WPARAM)session_id, 1);
    }

    return true;
}

void Monitor_Stop(void) {
    if (!g_monitor.is_monitoring && !s_hThread) return;

    if (s_hStopEvent) {
        SetEvent(s_hStopEvent);
    }

    if (s_hThread) {
        WaitForSingleObject(s_hThread, 3000);
        CloseHandle(s_hThread);
        s_hThread = NULL;
    }

    EnterCriticalSection(&g_monitor.cs);
    int session_id = g_monitor.active_session_id;
    g_monitor.is_monitoring = false;
    g_monitor.active_session_id = 0;
    LeaveCriticalSection(&g_monitor.cs);

    if (session_id > 0) {
        DB_EndSession(session_id);
    }

    if (s_hwndNotify && IsWindow(s_hwndNotify)) {
        PostMessage(s_hwndNotify, WM_NETPULSE_SESSION_STATE, (WPARAM)session_id, 0);
    }
}

bool Monitor_IsRunning(void) {
    EnterCriticalSection(&g_monitor.cs);
    bool r = g_monitor.is_monitoring;
    LeaveCriticalSection(&g_monitor.cs);
    return r;
}

void Monitor_GetSnapshot(ProbeSample *out_samples, int max_samples, int *out_count,
                         QoSMetrics *out_metrics, QoSAssessmentResult *out_assessment,
                         InterfaceSnapshot *out_if_stats, DbTarget *out_target,
                         bool *out_is_running, int *out_session_id) {
    EnterCriticalSection(&g_monitor.cs);

    if (out_count) *out_count = g_monitor.sample_count;
    if (out_samples && max_samples > 0) {
        int n = (g_monitor.sample_count < max_samples) ? g_monitor.sample_count : max_samples;
        memcpy(out_samples, g_monitor.samples, sizeof(ProbeSample) * n);
    }
    if (out_metrics) *out_metrics = g_monitor.current_metrics;
    if (out_assessment) *out_assessment = g_monitor.current_assessment;
    if (out_if_stats) *out_if_stats = g_monitor.current_if_stats;
    if (out_target) *out_target = g_monitor.active_target;
    if (out_is_running) *out_is_running = g_monitor.is_monitoring;
    if (out_session_id) *out_session_id = g_monitor.active_session_id;

    LeaveCriticalSection(&g_monitor.cs);
}
