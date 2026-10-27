#include "alerts.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AlertThresholds s_thresholds = {
    .max_rtt_threshold_ms = 150.0,
    .max_loss_threshold_pct = 5.0,
    .max_jitter_threshold_ms = 30.0,
    .consecutive_violations_limit = 2
};

static int s_consecutive_failures = 0;
static int s_consecutive_high_rtt = 0;
static int s_last_alerted_session = 0;

void Alerts_Init(void) {
    s_consecutive_failures = 0;
    s_consecutive_high_rtt = 0;
}

void Alerts_GetThresholds(AlertThresholds *out_thresh) {
    if (out_thresh) *out_thresh = s_thresholds;
}

void Alerts_SetThresholds(const AlertThresholds *thresh) {
    if (thresh) s_thresholds = *thresh;
}

void Alerts_EvaluateProbe(int session_id, double rtt_ms, bool success) {
    if (session_id <= 0) return;

    if (!success) {
        s_consecutive_failures++;
        if (s_consecutive_failures >= s_thresholds.consecutive_violations_limit) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Target unreachable or probe timeout (%d consecutive drops)", s_consecutive_failures);
            DB_InsertAlert(session_id, "critical", "reachability", 100.0, s_thresholds.max_loss_threshold_pct, msg);
            // reset counter to prevent flooding every single probe
            s_consecutive_failures = 0;
        }
    } else {
        s_consecutive_failures = 0;
        if (rtt_ms > s_thresholds.max_rtt_threshold_ms) {
            s_consecutive_high_rtt++;
            if (s_consecutive_high_rtt >= s_thresholds.consecutive_violations_limit) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Latency threshold exceeded: %.2f ms (limit: %.2f ms)",
                         rtt_ms, s_thresholds.max_rtt_threshold_ms);
                DB_InsertAlert(session_id, "warning", "latency", rtt_ms, s_thresholds.max_rtt_threshold_ms, msg);
                s_consecutive_high_rtt = 0;
            }
        } else {
            s_consecutive_high_rtt = 0;
        }
    }
}
