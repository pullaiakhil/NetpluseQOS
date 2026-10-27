#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../include/database.h"
#include "../include/reports.h"

int main(void) {
    printf("=== Running Database & CSV Export Unit Tests ===\n");

    const char *test_db = "test_run.db";
    remove(test_db);

    // Test 1: Initialize DB
    assert(DB_Init(test_db) == true);
    printf("[PASS] Database initialized\n");

    // Test 2: Add target
    int target_id = 0;
    assert(DB_AddTarget("Test DNS", "1.1.1.1", "icmp", 0, &target_id) == true);
    assert(target_id > 0);
    printf("[PASS] Target added with ID %d\n", target_id);

    // Test 3: Create session
    int session_id = 0;
    assert(DB_CreateSession(target_id, 1.0, "gaming", &session_id) == true);
    assert(session_id > 0);
    printf("[PASS] Session created with ID %d\n", session_id);

    // Test 4: Insert probes
    ProbeSample s1 = {
        .sequence = 1,
        .success = true,
        .rtt_ms = 14.5,
        .timestamp_iso = "2026-10-13T10:00:00Z",
        .resolved_ip = "1.1.1.1",
        .error_type = "",
        .error_detail = ""
    };
    ProbeSample s2 = {
        .sequence = 2,
        .success = false,
        .rtt_ms = -1.0,
        .timestamp_iso = "2026-10-13T10:00:01Z",
        .resolved_ip = "1.1.1.1",
        .error_type = "TIMEOUT",
        .error_detail = "Request timed out, simulated drop"
    };
    assert(DB_InsertProbe(session_id, &s1) == true);
    assert(DB_InsertProbe(session_id, &s2) == true);
    printf("[PASS] Probes inserted into SQLite\n");

    // Test 5: Export CSV
    const char *test_csv = "test_export.csv";
    remove(test_csv);
    assert(Reports_ExportSessionCSV(session_id, test_csv) == true);
    printf("[PASS] CSV report exported successfully\n");

    // Verify CSV content
    FILE *fp = fopen(test_csv, "r");
    assert(fp != NULL);
    char buf[512];
    assert(fgets(buf, sizeof(buf), fp) != NULL); // Header
    assert(strstr(buf, "sequence_number") != NULL);
    assert(fgets(buf, sizeof(buf), fp) != NULL); // Row 1
    assert(strstr(buf, "14.50") != NULL);
    assert(fgets(buf, sizeof(buf), fp) != NULL); // Row 2 (with commas in error detail)
    assert(strstr(buf, "TIMEOUT") != NULL);
    fclose(fp);
    printf("[PASS] CSV headers and escaping verified\n");

    // Cleanup
    DB_Close();
    remove(test_db);
    remove(test_csv);

    printf("\n>>> ALL DATABASE AND CSV EXPORT TESTS PASSED SUCCESSFULLY! <<<\n");
    return 0;
}
