#include "database.h"
#include "../../third_party/sqlite/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sqlite3 *s_db = NULL;

bool DB_Init(const char *db_path) {
    if (s_db) return true;
    if (!db_path) db_path = DB_FILE_PATH;

    int rc = sqlite3_open(db_path, &s_db);
    if (rc != SQLITE_OK) {
        if (s_db) {
            sqlite3_close(s_db);
            s_db = NULL;
        }
        return false;
    }

    // Enable WAL mode & foreign keys for high concurrent responsiveness
    sqlite3_exec(s_db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    sqlite3_exec(s_db, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);

    const char *schema =
        "CREATE TABLE IF NOT EXISTS monitoring_targets ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  host TEXT NOT NULL,"
        "  target_type TEXT NOT NULL DEFAULT 'icmp',"
        "  port INTEGER DEFAULT 80,"
        "  enabled INTEGER DEFAULT 1,"
        "  created_at TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS monitoring_sessions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  target_id INTEGER NOT NULL,"
        "  start_time TEXT NOT NULL,"
        "  end_time TEXT,"
        "  status TEXT NOT NULL DEFAULT 'running',"
        "  interval_seconds REAL NOT NULL DEFAULT 1.0,"
        "  profile TEXT DEFAULT 'video_conferencing',"
        "  FOREIGN KEY(target_id) REFERENCES monitoring_targets(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS probe_results ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  session_id INTEGER NOT NULL,"
        "  timestamp TEXT NOT NULL,"
        "  sequence_number INTEGER NOT NULL,"
        "  success INTEGER NOT NULL,"
        "  rtt_ms REAL,"
        "  error_type TEXT,"
        "  error_detail TEXT,"
        "  resolved_ip TEXT,"
        "  FOREIGN KEY(session_id) REFERENCES monitoring_sessions(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS alerts ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  session_id INTEGER NOT NULL,"
        "  timestamp TEXT NOT NULL,"
        "  severity TEXT NOT NULL,"
        "  metric TEXT NOT NULL,"
        "  observed_value REAL NOT NULL,"
        "  threshold_value REAL NOT NULL,"
        "  message TEXT NOT NULL,"
        "  acknowledged INTEGER DEFAULT 0,"
        "  FOREIGN KEY(session_id) REFERENCES monitoring_sessions(id) ON DELETE CASCADE"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_probe_sess_time ON probe_results(session_id, timestamp);";

    char *errmsg = NULL;
    rc = sqlite3_exec(s_db, schema, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        return false;
    }

    // Seed default common test targets if table empty
    int target_count = 0;
    DB_GetTargets(NULL, 0, &target_count);
    if (target_count == 0) {
        int id = 0;
        DB_AddTarget("Cloudflare DNS", "1.1.1.1", "icmp", 0, &id);
        DB_AddTarget("Google Public DNS", "8.8.8.8", "icmp", 0, &id);
        DB_AddTarget("Localhost Loopback", "127.0.0.1", "icmp", 0, &id);
        DB_AddTarget("Cloudflare HTTP", "1.1.1.1", "tcp", 80, &id);
        DB_AddTarget("Google HTTPS", "8.8.8.8", "tcp", 443, &id);
    }

    return true;
}

void DB_Close(void) {
    if (s_db) {
        sqlite3_close(s_db);
        s_db = NULL;
    }
}

bool DB_AddTarget(const char *name, const char *host, const char *type, int port, int *out_id) {
    if (!s_db || !name || !host) return false;
    const char *sql = "INSERT INTO monitoring_targets (name, host, target_type, port, enabled, created_at) "
                      "VALUES (?, ?, ?, ?, 1, datetime('now'));";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, host, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type ? type : "icmp", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, port);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (ok && out_id) {
        *out_id = (int)sqlite3_last_insert_rowid(s_db);
    }
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_GetTargets(DbTarget *out_targets, int max_targets, int *out_count) {
    if (!s_db) return false;
    const char *sql = "SELECT id, name, host, target_type, port, enabled, created_at FROM monitoring_targets ORDER BY id ASC;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (out_targets && count < max_targets) {
            DbTarget *t = &out_targets[count];
            t->id = sqlite3_column_int(stmt, 0);
            const char *n = (const char *)sqlite3_column_text(stmt, 1);
            const char *h = (const char *)sqlite3_column_text(stmt, 2);
            const char *tt = (const char *)sqlite3_column_text(stmt, 3);
            t->port = sqlite3_column_int(stmt, 4);
            t->enabled = (sqlite3_column_int(stmt, 5) != 0);
            const char *c = (const char *)sqlite3_column_text(stmt, 6);

            strncpy(t->name, n ? n : "", sizeof(t->name) - 1);
            strncpy(t->host, h ? h : "", sizeof(t->host) - 1);
            strncpy(t->target_type, tt ? tt : "icmp", sizeof(t->target_type) - 1);
            strncpy(t->created_at, c ? c : "", sizeof(t->created_at) - 1);
        }
        count++;
    }
    if (out_count) *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool DB_DeleteTarget(int target_id) {
    if (!s_db) return false;
    const char *sql = "DELETE FROM monitoring_targets WHERE id = ?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, target_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_UpdateTarget(int target_id, const char *name, const char *host, const char *type, int port) {
    if (!s_db) return false;
    const char *sql = "UPDATE monitoring_targets SET name = ?, host = ?, target_type = ?, port = ? WHERE id = ?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, host, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, type ? type : "icmp", -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, port);
    sqlite3_bind_int(stmt, 5, target_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_CreateSession(int target_id, double interval_sec, const char *profile, int *out_session_id) {
    if (!s_db) return false;
    const char *sql = "INSERT INTO monitoring_sessions (target_id, start_time, status, interval_seconds, profile) "
                      "VALUES (?, datetime('now'), 'running', ?, ?);";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, target_id);
    sqlite3_bind_double(stmt, 2, interval_sec);
    sqlite3_bind_text(stmt, 3, profile ? profile : "video_conferencing", -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (ok && out_session_id) {
        *out_session_id = (int)sqlite3_last_insert_rowid(s_db);
    }
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_EndSession(int session_id) {
    if (!s_db) return false;
    const char *sql = "UPDATE monitoring_sessions SET end_time = datetime('now'), status = 'stopped' WHERE id = ?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, session_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_GetSessions(DbSession *out_sessions, int max_sessions, int *out_count) {
    if (!s_db) return false;
    const char *sql = "SELECT id, target_id, start_time, IFNULL(end_time, ''), status, interval_seconds, profile "
                      "FROM monitoring_sessions ORDER BY id DESC;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (out_sessions && count < max_sessions) {
            DbSession *s = &out_sessions[count];
            s->id = sqlite3_column_int(stmt, 0);
            s->target_id = sqlite3_column_int(stmt, 1);
            const char *st = (const char *)sqlite3_column_text(stmt, 2);
            const char *et = (const char *)sqlite3_column_text(stmt, 3);
            const char *stat = (const char *)sqlite3_column_text(stmt, 4);
            s->interval_seconds = sqlite3_column_double(stmt, 5);
            const char *p = (const char *)sqlite3_column_text(stmt, 6);

            strncpy(s->start_time, st ? st : "", sizeof(s->start_time) - 1);
            strncpy(s->end_time, et ? et : "", sizeof(s->end_time) - 1);
            strncpy(s->status, stat ? stat : "stopped", sizeof(s->status) - 1);
            strncpy(s->profile, p ? p : "video_conferencing", sizeof(s->profile) - 1);
        }
        count++;
    }
    if (out_count) *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool DB_InsertProbe(int session_id, const ProbeSample *sample) {
    if (!s_db || !sample) return false;
    const char *sql = "INSERT INTO probe_results (session_id, timestamp, sequence_number, success, rtt_ms, error_type, error_detail, resolved_ip) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, session_id);
    sqlite3_bind_text(stmt, 2, sample->timestamp_iso, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, sample->sequence);
    sqlite3_bind_int(stmt, 4, sample->success ? 1 : 0);

    if (sample->success) {
        sqlite3_bind_double(stmt, 5, sample->rtt_ms);
    } else {
        sqlite3_bind_null(stmt, 5);
    }

    sqlite3_bind_text(stmt, 6, sample->error_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, sample->error_detail, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, sample->resolved_ip, -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_GetSessionProbes(int session_id, ProbeSample *out_samples, int max_samples, int *out_count) {
    if (!s_db) return false;
    const char *sql = "SELECT timestamp, sequence_number, success, rtt_ms, IFNULL(error_type, ''), IFNULL(error_detail, ''), IFNULL(resolved_ip, '') "
                      "FROM probe_results WHERE session_id = ? ORDER BY sequence_number ASC;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, session_id);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (out_samples && count < max_samples) {
            ProbeSample *p = &out_samples[count];
            const char *ts = (const char *)sqlite3_column_text(stmt, 0);
            p->sequence = sqlite3_column_int(stmt, 1);
            p->success = (sqlite3_column_int(stmt, 2) != 0);

            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
                p->rtt_ms = sqlite3_column_double(stmt, 3);
            } else {
                p->rtt_ms = -1.0;
            }

            const char *et = (const char *)sqlite3_column_text(stmt, 4);
            const char *ed = (const char *)sqlite3_column_text(stmt, 5);
            const char *rip = (const char *)sqlite3_column_text(stmt, 6);

            strncpy(p->timestamp_iso, ts ? ts : "", sizeof(p->timestamp_iso) - 1);
            strncpy(p->error_type, et ? et : "", sizeof(p->error_type) - 1);
            strncpy(p->error_detail, ed ? ed : "", sizeof(p->error_detail) - 1);
            strncpy(p->resolved_ip, rip ? rip : "", sizeof(p->resolved_ip) - 1);
        }
        count++;
    }
    if (out_count) *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool DB_InsertAlert(int session_id, const char *severity, const char *metric, double observed, double threshold, const char *msg) {
    if (!s_db) return false;
    const char *sql = "INSERT INTO alerts (session_id, timestamp, severity, metric, observed_value, threshold_value, message, acknowledged) "
                      "VALUES (?, datetime('now'), ?, ?, ?, ?, ?, 0);";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, session_id);
    sqlite3_bind_text(stmt, 2, severity ? severity : "warning", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, metric ? metric : "rtt", -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, observed);
    sqlite3_bind_double(stmt, 5, threshold);
    sqlite3_bind_text(stmt, 6, msg ? msg : "", -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DB_GetAlerts(DbAlert *out_alerts, int max_alerts, int *out_count) {
    if (!s_db) return false;
    const char *sql = "SELECT id, session_id, timestamp, severity, metric, observed_value, threshold_value, message, acknowledged "
                      "FROM alerts ORDER BY id DESC;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (out_alerts && count < max_alerts) {
            DbAlert *a = &out_alerts[count];
            a->id = sqlite3_column_int(stmt, 0);
            a->session_id = sqlite3_column_int(stmt, 1);
            const char *ts = (const char *)sqlite3_column_text(stmt, 2);
            const char *sev = (const char *)sqlite3_column_text(stmt, 3);
            const char *met = (const char *)sqlite3_column_text(stmt, 4);
            a->observed_value = sqlite3_column_double(stmt, 5);
            a->threshold_value = sqlite3_column_double(stmt, 6);
            const char *msg = (const char *)sqlite3_column_text(stmt, 7);
            a->acknowledged = (sqlite3_column_int(stmt, 8) != 0);

            strncpy(a->timestamp, ts ? ts : "", sizeof(a->timestamp) - 1);
            strncpy(a->severity, sev ? sev : "info", sizeof(a->severity) - 1);
            strncpy(a->metric, met ? met : "", sizeof(a->metric) - 1);
            strncpy(a->message, msg ? msg : "", sizeof(a->message) - 1);
        }
        count++;
    }
    if (out_count) *out_count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool DB_AcknowledgeAlert(int alert_id) {
    if (!s_db) return false;
    const char *sql = "UPDATE alerts SET acknowledged = 1 WHERE id = ?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, alert_id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}
