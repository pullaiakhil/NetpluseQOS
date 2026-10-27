#ifndef ICMP_PROBE_H
#define ICMP_PROBE_H

#include "qos_metrics.h"
#include <stdbool.h>

void ICMP_ProbeHost(const char *host, double timeout_sec, ProbeSample *out_sample);

#endif /* ICMP_PROBE_H */
