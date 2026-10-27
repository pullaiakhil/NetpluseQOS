#include "reports.h"
#include "database.h"
#include "qos_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_csv_escaped(FILE *fp, const char *val) {
    if (!val) {
        return;
    }
    bool needs_quotes = false;
    for (const char *p = val; *p; p++) {
        if (*p == ',' || *p == '"' || *p == '\n' || *p == '\r') {
            needs_quotes = true;
            break;
        }
    }
    if (!needs_quotes) {
        fputs(val, fp);
        return;
    }
    fputc('"', fp);
    for (const char *p = val; *p; p++) {
        if (*p == '"') {
            fputc('"', fp);
            fputc('"', fp);
        } else {
            fputc(*p, fp);
        }
    }
    fputc('"', fp);
}

bool Reports_ExportSessionCSV(int session_id, const char *output_filepath) {
    if (!output_filepath) return false;

    ProbeSample samples[1024];
    int count = 0;
    if (!DB_GetSessionProbes(session_id, samples, 1024, &count)) {
        return false;
    }

    FILE *fp = fopen(output_filepath, "w");
    if (!fp) return false;

    // Header matching backend CSV schema
    fprintf(fp, "sequence_number,timestamp_utc,success,rtt_ms,error_type,error_detail,resolved_ip\n");

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,", samples[i].sequence);
        write_csv_escaped(fp, samples[i].timestamp_iso);
        fprintf(fp, ",%s,", samples[i].success ? "1" : "0");
        if (samples[i].success) {
            fprintf(fp, "%.2f,", samples[i].rtt_ms);
        } else {
            fprintf(fp, ",");
        }
        write_csv_escaped(fp, samples[i].error_type);
        fprintf(fp, ",");
        write_csv_escaped(fp, samples[i].error_detail);
        fprintf(fp, ",");
        write_csv_escaped(fp, samples[i].resolved_ip);
        fprintf(fp, "\n");
    }

    fclose(fp);
    return true;
}
