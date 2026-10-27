#ifndef MONITOR_H
#define MONITOR_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <stdbool.h>
#include "qos_metrics.h"
#include "database.h"
#include "interface_stats.h"

#define WM_NETPULSE_SAMPLE_UPDATE (WM_USER + 101)
#define WM_NETPULSE_SESSION_STATE  (WM_USER + 102)

typedef struct {
    bool is_monitoring;
    int active_session_id;
    DbTarget active_target;
    double interval_seconds;
    char qos_profile[32];

    // Live state protected by critical section
    CRITICAL_SECTION cs;
    ProbeSample samples[MAX_SAMPLES_BUFFER];
    int sample_count;
    QoSMetrics current_metrics;
    QoSAssessmentResult current_assessment;
    InterfaceSnapshot current_if_stats;
    char last_sample_time[32];
    double latest_rtt_ms;
    bool latest_success;
    char latest_error[128];
} MonitorController;

extern MonitorController g_monitor;

void Monitor_Init(HWND hwndNotify);
void Monitor_Cleanup(void);

bool Monitor_Start(const DbTarget *target, double interval_sec, const char *profile);
void Monitor_Stop(void);
bool Monitor_IsRunning(void);

// Safe thread snapshot
void Monitor_GetSnapshot(ProbeSample *out_samples, int max_samples, int *out_count,
                         QoSMetrics *out_metrics, QoSAssessmentResult *out_assessment,
                         InterfaceSnapshot *out_if_stats, DbTarget *out_target,
                         bool *out_is_running, int *out_session_id);

#endif /* MONITOR_H */
