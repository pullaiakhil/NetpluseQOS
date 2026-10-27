#ifndef TCP_PROBE_H
#define TCP_PROBE_H

#include "qos_metrics.h"
#include <stdbool.h>

void TCP_ProbeService(const char *host, int port, double timeout_sec, ProbeSample *out_sample);

#endif /* TCP_PROBE_H */
