#ifndef QOS_METRICS_H
#define QOS_METRICS_H

#include <stdbool.h>

#define MAX_SAMPLES_BUFFER 1024

typedef struct {
    int sequence;
    bool success;
    double rtt_ms;
    char error_type[64];
    char error_detail[256];
    char resolved_ip[64];
    char timestamp_iso[32];
} ProbeSample;

typedef struct {
    int transmitted;
    int received;
    int failed;
    double packet_loss_pct;
    double min_rtt_ms;
    double max_rtt_ms;
    double mean_rtt_ms;
    double median_rtt_ms;
    double stddev_rtt_ms;
    double p95_rtt_ms;
    double jitter_ms; /* Mean absolute difference between consecutive valid samples (RFC 1889 / 3550 style) */
} QoSMetrics;

typedef enum {
    QOS_STATUS_INSUFFICIENT_DATA = 0,
    QOS_STATUS_EXCELLENT,
    QOS_STATUS_GOOD,
    QOS_STATUS_DEGRADED,
    QOS_STATUS_POOR
} QoSEvaluationStatus;

typedef struct {
    char name[32];
    int min_samples;
    double latency_good_ms;
    double latency_degraded_ms;
    double jitter_good_ms;
    double jitter_degraded_ms;
    double loss_good_pct;
    double loss_degraded_pct;
} QoSProfile;

typedef struct {
    QoSEvaluationStatus overall_status;
    char overall_status_str[24];
    char profile_name[32];
    char summary_text[256];
    char latency_status_str[16];
    char jitter_status_str[16];
    char loss_status_str[16];
} QoSAssessmentResult;

/* QoS Calculations */
void QoS_ComputeMetrics(const ProbeSample *samples, int count, QoSMetrics *out_metrics);
void QoS_EvaluateProfile(const QoSMetrics *metrics, const char *profile_name, QoSAssessmentResult *out_result);
const char* QoS_StatusToString(QoSEvaluationStatus status);

#endif /* QOS_METRICS_H */
