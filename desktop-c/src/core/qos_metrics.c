#include "qos_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int compare_doubles(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

void QoS_ComputeMetrics(const ProbeSample *samples, int count, QoSMetrics *out_metrics) {
    if (!out_metrics) return;
    memset(out_metrics, 0, sizeof(QoSMetrics));

    if (!samples || count <= 0) {
        out_metrics->min_rtt_ms = -1.0;
        out_metrics->max_rtt_ms = -1.0;
        out_metrics->mean_rtt_ms = -1.0;
        out_metrics->median_rtt_ms = -1.0;
        out_metrics->stddev_rtt_ms = -1.0;
        out_metrics->p95_rtt_ms = -1.0;
        out_metrics->jitter_ms = -1.0;
        return;
    }

    out_metrics->transmitted = count;

    double valid_rtts[MAX_SAMPLES_BUFFER];
    int valid_count = 0;
    double sum_rtt = 0.0;

    for (int i = 0; i < count; i++) {
        if (samples[i].success && samples[i].rtt_ms >= 0.0) {
            if (valid_count < MAX_SAMPLES_BUFFER) {
                valid_rtts[valid_count++] = samples[i].rtt_ms;
                sum_rtt += samples[i].rtt_ms;
            }
        }
    }

    out_metrics->received = valid_count;
    out_metrics->failed = count - valid_count;
    out_metrics->packet_loss_pct = ((double)out_metrics->failed / (double)count) * 100.0;

    if (valid_count == 0) {
        out_metrics->min_rtt_ms = -1.0;
        out_metrics->max_rtt_ms = -1.0;
        out_metrics->mean_rtt_ms = -1.0;
        out_metrics->median_rtt_ms = -1.0;
        out_metrics->stddev_rtt_ms = -1.0;
        out_metrics->p95_rtt_ms = -1.0;
        out_metrics->jitter_ms = -1.0;
        return;
    }

    /* Mean RTT */
    out_metrics->mean_rtt_ms = sum_rtt / (double)valid_count;

    /* Sort valid RTTs for min, max, median, p95 */
    qsort(valid_rtts, valid_count, sizeof(double), compare_doubles);
    out_metrics->min_rtt_ms = valid_rtts[0];
    out_metrics->max_rtt_ms = valid_rtts[valid_count - 1];

    /* Median */
    if (valid_count % 2 == 1) {
        out_metrics->median_rtt_ms = valid_rtts[valid_count / 2];
    } else {
        out_metrics->median_rtt_ms = (valid_rtts[(valid_count / 2) - 1] + valid_rtts[valid_count / 2]) / 2.0;
    }

    /* Sample Standard Deviation */
    if (valid_count >= 2) {
        double sum_sq = 0.0;
        for (int i = 0; i < valid_count; i++) {
            double diff = valid_rtts[i] - out_metrics->mean_rtt_ms;
            sum_sq += diff * diff;
        }
        out_metrics->stddev_rtt_ms = sqrt(sum_sq / (double)(valid_count - 1));
    } else {
        out_metrics->stddev_rtt_ms = 0.0;
    }

    /* 95th Percentile */
    if (valid_count == 1) {
        out_metrics->p95_rtt_ms = valid_rtts[0];
    } else {
        double k = 0.95 * (double)(valid_count - 1);
        int f = (int)floor(k);
        int c = (int)ceil(k);
        if (f == c) {
            out_metrics->p95_rtt_ms = valid_rtts[f];
        } else {
            out_metrics->p95_rtt_ms = valid_rtts[f] * ((double)c - k) + valid_rtts[c] * (k - (double)f);
        }
    }

    /* Jitter: Mean absolute difference between consecutive valid samples */
    if (valid_count >= 2) {
        double sum_jitter = 0.0;
        int jitter_pairs = 0;
        for (int i = 1; i < count; i++) {
            if (samples[i].success && samples[i - 1].success) {
                double diff = fabs(samples[i].rtt_ms - samples[i - 1].rtt_ms);
                sum_jitter += diff;
                jitter_pairs++;
            }
        }
        if (jitter_pairs > 0) {
            out_metrics->jitter_ms = sum_jitter / (double)jitter_pairs;
        } else {
            out_metrics->jitter_ms = 0.0;
        }
    } else {
        out_metrics->jitter_ms = 0.0;
    }
}

static const QoSProfile s_profiles[] = {
    {"video_conferencing", 3, 150.0, 300.0, 30.0, 50.0, 1.0, 5.0},
    {"voip",               3, 100.0, 200.0, 20.0, 40.0, 0.5, 2.0},
    {"gaming",             3, 50.0,  100.0, 10.0, 25.0, 0.1, 2.0},
    {"browsing",           2, 250.0, 500.0, 50.0, 100.0, 2.0, 8.0},
    {"file_download",      2, 400.0, 800.0, 100.0, 200.0, 3.0, 10.0}
};
static const int s_profile_count = sizeof(s_profiles) / sizeof(s_profiles[0]);

const char* QoS_StatusToString(QoSEvaluationStatus status) {
    switch (status) {
        case QOS_STATUS_EXCELLENT: return "Excellent";
        case QOS_STATUS_GOOD: return "Good";
        case QOS_STATUS_DEGRADED: return "Degraded";
        case QOS_STATUS_POOR: return "Poor";
        default: return "Insufficient Data";
    }
}

void QoS_EvaluateProfile(const QoSMetrics *metrics, const char *profile_name, QoSAssessmentResult *out_result) {
    if (!out_result) return;
    memset(out_result, 0, sizeof(QoSAssessmentResult));

    if (!profile_name || profile_name[0] == '\0') {
        profile_name = "video_conferencing";
    }
    strncpy(out_result->profile_name, profile_name, sizeof(out_result->profile_name) - 1);

    const QoSProfile *prof = &s_profiles[0];
    for (int i = 0; i < s_profile_count; i++) {
        if (strcmp(s_profiles[i].name, profile_name) == 0) {
            prof = &s_profiles[i];
            break;
        }
    }

    if (!metrics || metrics->transmitted < prof->min_samples || metrics->received == 0) {
        out_result->overall_status = QOS_STATUS_INSUFFICIENT_DATA;
        strncpy(out_result->overall_status_str, "Insufficient Data", sizeof(out_result->overall_status_str) - 1);
        snprintf(out_result->summary_text, sizeof(out_result->summary_text),
                 "Need at least %d samples for %s profile (observed: %d)",
                 prof->min_samples, prof->name, metrics ? metrics->transmitted : 0);
        strncpy(out_result->latency_status_str, "N/A", sizeof(out_result->latency_status_str) - 1);
        strncpy(out_result->jitter_status_str, "N/A", sizeof(out_result->jitter_status_str) - 1);
        strncpy(out_result->loss_status_str, "N/A", sizeof(out_result->loss_status_str) - 1);
        return;
    }

    int worst_severity = 1; // 1: Good, 2: Degraded, 3: Poor

    // Latency evaluation
    if (metrics->mean_rtt_ms <= prof->latency_good_ms) {
        strncpy(out_result->latency_status_str, "Good", sizeof(out_result->latency_status_str) - 1);
    } else if (metrics->mean_rtt_ms <= prof->latency_degraded_ms) {
        strncpy(out_result->latency_status_str, "Degraded", sizeof(out_result->latency_status_str) - 1);
        if (worst_severity < 2) worst_severity = 2;
    } else {
        strncpy(out_result->latency_status_str, "Poor", sizeof(out_result->latency_status_str) - 1);
        worst_severity = 3;
    }

    // Jitter evaluation
    if (metrics->jitter_ms <= prof->jitter_good_ms) {
        strncpy(out_result->jitter_status_str, "Good", sizeof(out_result->jitter_status_str) - 1);
    } else if (metrics->jitter_ms <= prof->jitter_degraded_ms) {
        strncpy(out_result->jitter_status_str, "Degraded", sizeof(out_result->jitter_status_str) - 1);
        if (worst_severity < 2) worst_severity = 2;
    } else {
        strncpy(out_result->jitter_status_str, "Poor", sizeof(out_result->jitter_status_str) - 1);
        worst_severity = 3;
    }

    // Loss evaluation
    if (metrics->packet_loss_pct <= prof->loss_good_pct) {
        strncpy(out_result->loss_status_str, "Good", sizeof(out_result->loss_status_str) - 1);
    } else if (metrics->packet_loss_pct <= prof->loss_degraded_pct) {
        strncpy(out_result->loss_status_str, "Degraded", sizeof(out_result->loss_status_str) - 1);
        if (worst_severity < 2) worst_severity = 2;
    } else {
        strncpy(out_result->loss_status_str, "Poor", sizeof(out_result->loss_status_str) - 1);
        worst_severity = 3;
    }

    if (worst_severity == 1) {
        out_result->overall_status = QOS_STATUS_EXCELLENT;
        strncpy(out_result->overall_status_str, "Optimal / SLA Compliant", sizeof(out_result->overall_status_str) - 1);
        snprintf(out_result->summary_text, sizeof(out_result->summary_text),
                 "All network metrics satisfy '%s' profile targets.", prof->name);
    } else if (worst_severity == 2) {
        out_result->overall_status = QOS_STATUS_DEGRADED;
        strncpy(out_result->overall_status_str, "Degraded", sizeof(out_result->overall_status_str) - 1);
        snprintf(out_result->summary_text, sizeof(out_result->summary_text),
                 "Slight degradation observed for '%s'. Latency: %s, Jitter: %s, Loss: %s.",
                 prof->name, out_result->latency_status_str, out_result->jitter_status_str, out_result->loss_status_str);
    } else {
        out_result->overall_status = QOS_STATUS_POOR;
        strncpy(out_result->overall_status_str, "Poor / SLA Violation", sizeof(out_result->overall_status_str) - 1);
        snprintf(out_result->summary_text, sizeof(out_result->summary_text),
                 "Significant QoS impairment for '%s'. Latency: %s, Jitter: %s, Loss: %s.",
                 prof->name, out_result->latency_status_str, out_result->jitter_status_str, out_result->loss_status_str);
    }
}
