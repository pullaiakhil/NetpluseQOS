#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>
#include "qos_metrics.h"

#define DB_FILE_PATH "netpulse_desktop.db"

typedef struct {
    int id;
    char name[128];
    char host[256];
    char target_type[20]; // "icmp" or "tcp"
    int port;
    bool enabled;
    char created_at[32];
} DbTarget;

typedef struct {
    int id;
    int target_id;
    char start_time[32];
    char end_time[32];
    char status[20]; // "running", "stopped"
    double interval_seconds;
    char profile[32];
} DbSession;

typedef struct {
    int id;
    int session_id;
    char timestamp[32];
    char severity[20]; // "info", "warning", "critical"
    char metric[48];
    double observed_value;
    double threshold_value;
    char message[256];
    bool acknowledged;
} DbAlert;

/* Database Operations */
bool DB_Init(const char *db_path);
void DB_Close(void);

// Targets
bool DB_AddTarget(const char *name, const char *host, const char *type, int port, int *out_id);
bool DB_GetTargets(DbTarget *out_targets, int max_targets, int *out_count);
bool DB_DeleteTarget(int target_id);
bool DB_UpdateTarget(int target_id, const char *name, const char *host, const char *type, int port);

// Sessions
bool DB_CreateSession(int target_id, double interval_sec, const char *profile, int *out_session_id);
bool DB_EndSession(int session_id);
bool DB_GetSessions(DbSession *out_sessions, int max_sessions, int *out_count);

// Probes
bool DB_InsertProbe(int session_id, const ProbeSample *sample);
bool DB_GetSessionProbes(int session_id, ProbeSample *out_samples, int max_samples, int *out_count);

// Alerts
bool DB_InsertAlert(int session_id, const char *severity, const char *metric, double observed, double threshold, const char *msg);
bool DB_GetAlerts(DbAlert *out_alerts, int max_alerts, int *out_count);
bool DB_AcknowledgeAlert(int alert_id);

#endif /* DATABASE_H */
