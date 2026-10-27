#ifndef ALERTS_H
#define ALERTS_H

#include <stdbool.h>

typedef struct {
    double max_rtt_threshold_ms;
    double max_loss_threshold_pct;
    double max_jitter_threshold_ms;
    int consecutive_violations_limit;
} AlertThresholds;

void Alerts_Init(void);
void Alerts_GetThresholds(AlertThresholds *out_thresh);
void Alerts_SetThresholds(const AlertThresholds *thresh);

// Evaluates probe sample against thresholds and logs alert to DB with hysteresis
void Alerts_EvaluateProbe(int session_id, double rtt_ms, bool success);

#endif /* ALERTS_H */
