#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../include/qos_metrics.h"

int main(void) {
    printf("=== Running QoS Metrics Unit Tests ===\n");

    // Test 1: Empty sample set
    QoSMetrics m_empty;
    QoS_ComputeMetrics(NULL, 0, &m_empty);
    assert(m_empty.transmitted == 0);
    assert(m_empty.received == 0);
    assert(m_empty.min_rtt_ms == -1.0);
    assert(m_empty.jitter_ms == -1.0);
    printf("[PASS] Empty sample set correctly handled\n");

    // Test 2: Standard samples with known values
    ProbeSample samples[5] = {
        { .sequence = 1, .success = true,  .rtt_ms = 10.0 },
        { .sequence = 2, .success = true,  .rtt_ms = 20.0 },
        { .sequence = 3, .success = false, .rtt_ms = -1.0 }, // Dropped
        { .sequence = 4, .success = true,  .rtt_ms = 30.0 },
        { .sequence = 5, .success = true,  .rtt_ms = 40.0 }
    };

    QoSMetrics m;
    QoS_ComputeMetrics(samples, 5, &m);

    assert(m.transmitted == 5);
    assert(m.received == 4);
    assert(m.failed == 1);
    assert(fabs(m.packet_loss_pct - 20.0) < 0.001);
    assert(fabs(m.min_rtt_ms - 10.0) < 0.001);
    assert(fabs(m.max_rtt_ms - 40.0) < 0.001);
    assert(fabs(m.mean_rtt_ms - 25.0) < 0.001);
    assert(fabs(m.median_rtt_ms - 25.0) < 0.001);
    printf("[PASS] Basic statistics (mean, min, max, loss, median) verified\n");

    // Test 3: Jitter calculation
    // Valid consecutive pairs:
    // (sample 1, 2) -> |20 - 10| = 10
    // (sample 2, 3) -> sample 3 failed (not paired)
    // (sample 3, 4) -> sample 3 failed (not paired)
    // (sample 4, 5) -> |40 - 30| = 10
    // Average jitter = (10 + 10) / 2 = 10.0
    assert(fabs(m.jitter_ms - 10.0) < 0.001);
    printf("[PASS] Jitter estimation verified\n");

    // Test 4: Profile SLA evaluation
    QoSAssessmentResult assess;
    QoS_EvaluateProfile(&m, "video_conferencing", &assess);
    assert(assess.overall_status == QOS_STATUS_POOR); // Loss 20% > 5% degraded threshold -> POOR
    printf("[PASS] QoS SLA profile assessment verified\n");

    printf("\n>>> ALL QOS METRICS UNIT TESTS PASSED SUCCESSFULLY! <<<\n");
    return 0;
}
