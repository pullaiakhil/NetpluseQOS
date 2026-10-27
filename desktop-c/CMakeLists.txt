cmake_minimum_required(VERSION 3.20)
project(NetPulseQoS LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
)

# SQLite Amalgamation source
set(SQLITE_SOURCES
    third_party/sqlite/sqlite3.c
)

# Core Application Sources
set(NETPULSE_SOURCES
    src/main.c
    src/core/qos_metrics.c
    src/core/monitor.c
    src/core/alerts.c
    src/network/icmp_probe.c
    src/network/tcp_probe.c
    src/network/interface_stats.c
    src/storage/database.c
    src/reports/csv_export.c
    src/ui/main_window.c
    src/ui/charts.c
    ${SQLITE_SOURCES}
)

# Executable
add_executable(netpulse_app ${NETPULSE_SOURCES})

# Windows System Libraries: Winsock2, ICMP API, IP Helper, GDI, User32, Kernel32
target_link_libraries(netpulse_app PRIVATE
    ws2_32
    iphlpapi
    gdi32
    user32
    kernel32
    comctl32
)

# Enable console/gui flags appropriately
if(MSVC)
    target_compile_options(netpulse_app PRIVATE /W3)
else()
    target_compile_options(netpulse_app PRIVATE -Wall -Wextra -O2)
endif()

# Unit Tests Executable
add_executable(test_qos_metrics
    tests/test_qos_metrics.c
    src/core/qos_metrics.c
)
if(NOT MSVC)
    target_link_libraries(test_qos_metrics PRIVATE m)
endif()

add_executable(test_database_reports
    tests/test_database_reports.c
    src/storage/database.c
    src/reports/csv_export.c
    src/core/qos_metrics.c
    ${SQLITE_SOURCES}
)

add_executable(test_live_network
    tests/test_live_network.c
    src/network/icmp_probe.c
    src/network/tcp_probe.c
    src/network/interface_stats.c
)
target_link_libraries(test_live_network PRIVATE
    ws2_32
    iphlpapi
)
