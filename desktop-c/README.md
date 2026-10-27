# NetPulse QoS — Native Windows Desktop Application in C

A native Windows desktop application for real-time Quality of Service (QoS) network monitoring, SLA assessment, and diagnostics, written entirely in **C (C11)** using native Win32 APIs, GDI custom rendering, Winsock2, Windows ICMP, IP Helper API, and SQLite3.

Developed for **Computer Networks (B.Tech CSE)** academic faculty demonstration.

---

## 1. Architectural Highlights

- **100% Native C**: Written completely in C (C11). No React, Node.js, Python, Electron, or WebView runtime required.
- **Genuine Network Probing**:
  - **ICMP Latency**: Native Windows ICMP API (`IcmpCreateFile`, `IcmpSendEcho`) with microsecond-grade monotonic timing via `QueryPerformanceCounter`.
  - **TCP Connection Timing**: Socket-based non-blocking TCP handshakes (`connect`, `select`, `SO_ERROR`) with connection error classification (`DNS_FAILURE`, `CONNECTION_REFUSED`, `TIMEOUT`).
  - **Hardware Interface Counters**: Windows IP Helper API (`GetIfTable2`) polling Rx/Tx octet counters and calculating live transmission rates.
- **QoS Metrics Engine**:
  - Sample count, successful and dropped probes.
  - Min, Max, Mean, Median, Sample Standard Deviation, and 95th Percentile RTT.
  - RFC 1889 / 3550 Inter-Packet Jitter estimation: Mean absolute difference between consecutive valid samples.
  - Accurate packet loss calculation: $(\text{Failed} / \text{Transmitted}) \times 100\%$.
  - Multi-profile SLA assessment: Video Conferencing, VoIP, Gaming, Browsing, and File Transfer.
- **Persistent Local Database**:
  - SQLite 3 Amalgamation integrated directly via C API with WAL mode.
  - Stores targets, monitoring sessions, timestamped probe samples, and alert logs.
- **RFC 4180 CSV Export**:
  - Native C CSV exporter with full delimiter, newline, and quotation escaping.
- **Enterprise-Grade UI/UX**:
  - Custom GDI rendering with double-buffering (no flicker).
  - Clean observability theme with subtle borders, soft neutral backgrounds, and deep navy typography.
  - Real-time custom GDI vector line charts with dynamic Y-axis scaling, gridlines, and drop markers.
  - Asynchronous background worker threads (`_beginthreadex`) communicating safely with the UI loop via Windows messages (`WM_NETPULSE_SAMPLE_UPDATE`), ensuring the window remains 100% responsive.

---

## 2. Directory Structure

```
desktop-c/
├── CMakeLists.txt              # CMake build configuration
├── build.ps1                   # Automated build script
├── run_tests.ps1               # Automated unit test runner
├── run_live_test.ps1           # Automated live probe validation script
├── include/
│   ├── app.h                   # Application constants, styles, context
│   ├── ui.h                    # Win32 GDI custom drawing declarations
│   ├── monitor.h               # Background scheduler and worker thread controller
│   ├── icmp_probe.h            # Windows ICMP API probe
│   ├── tcp_probe.h             # Winsock2 TCP handshake probe
│   ├── interface_stats.h       # IP Helper interface traffic counters
│   ├── qos_metrics.h           # Statistical calculations and SLA profiles
│   ├── database.h              # SQLite3 persistence layer
│   ├── alerts.h                # Threshold rules and hysteresis logic
│   └── reports.h               # CSV report generator
├── src/
│   ├── main.c                  # WinMain, window class, message loop
│   ├── ui/
│   │   ├── main_window.c       # View renderers, navigation, controls
│   │   └── charts.c            # Custom GDI line chart renderer
│   ├── network/
│   │   ├── icmp_probe.c        # IcmpSendEcho implementation
│   │   ├── tcp_probe.c         # Non-blocking socket connect
│   │   └── interface_stats.c   # GetIfTable2 interface counter calculations
│   ├── core/
│   │   ├── monitor.c           # Worker thread and thread-safe snapshot
│   │   ├── qos_metrics.c       # Min/Max/Mean/Median/StdDev/P95/Jitter
│   │   └── alerts.c            # Hysteresis and threshold evaluations
│   ├── storage/
│   │   └── database.c          # SQLite3 CRUD operations
│   └── reports/
│       └── csv_export.c        # Escaped CSV export implementation
├── tests/
│   ├── test_qos_metrics.c      # Unit tests for statistical calculations
│   ├── test_database_reports.c # Unit tests for SQLite DB & CSV escaping
│   └── test_live_network.c     # Live validation of ICMP, TCP & IP Helper
├── third_party/
│   └── sqlite/
│       ├── sqlite3.c           # SQLite 3.46.1 amalgamation C source
│       └── sqlite3.h           # SQLite 3 header
└── build/
    ├── netpulse_app.exe        # Complete Native Desktop Windows Executable (1.25 MB)
    ├── test_qos_metrics.exe    # QoS unit test binary
    ├── test_database_reports.exe# Database & CSV unit test binary
    └── test_live_network.exe   # Live network probe validation binary
```

---

## 3. Prerequisites

- **Operating System**: Windows 10 or Windows 11 (64-bit)
- **Compiler**: GCC (MinGW-w64) or MSVC
- **Build System**: CMake 3.20+
- **Included Dependencies**:
  - SQLite 3 (Amalgamation C source included in `third_party/sqlite/`)
  - Windows SDK system libraries linked automatically: `ws2_32`, `iphlpapi`, `gdi32`, `user32`, `kernel32`, `comctl32`.

---

## 4. How to Build

From PowerShell in the project directory:

```powershell
cd desktop-c
.\build.ps1
```

Or manually with CMake and MinGW:

```powershell
cd desktop-c
$env:PATH = "C:\mingw64\bin;C:\Windows\System32;C:\Windows;C:\Windows\System32\WindowsPowerShell\v1.0"
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER="C:/mingw64/bin/gcc.exe"
cmake --build build --config Release
```

The resulting standalone executable is generated at:
```
desktop-c\build\netpulse_app.exe
```

---

## 5. How to Run the Tests

Run the test suite script to verify calculations, database persistence, and CSV generation:

```powershell
cd desktop-c
.\run_tests.ps1
```

Output:
```
=== Running test_qos_metrics ===
=== Running QoS Metrics Unit Tests ===
[PASS] Empty sample set correctly handled
[PASS] Basic statistics (mean, min, max, loss, median) verified
[PASS] Jitter estimation verified
[PASS] QoS SLA profile assessment verified
>>> ALL QOS METRICS UNIT TESTS PASSED SUCCESSFULLY! <<<

=== Running test_database_reports ===
=== Running Database & CSV Export Unit Tests ===
[PASS] Database initialized
[PASS] Target added with ID 6
[PASS] Session created with ID 1
[PASS] Probes inserted into SQLite
[PASS] CSV report exported successfully
[PASS] CSV headers and escaping verified
>>> ALL DATABASE AND CSV EXPORT TESTS PASSED SUCCESSFULLY! <<<
```

To run the live probe validation against real hosts:

```powershell
cd desktop-c
.\run_live_test.ps1
```

---

## 6. How to Run and Demo the Application

### Launching the Application
Execute the compiled desktop executable:

```powershell
cd desktop-c\build
.\netpulse_app.exe
```

### Performing a Demonstration for Faculty

1. **Overview Dashboard**:
   - The left sidebar displays 10 navigation views: Overview, Live Monitoring, Targets, QoS Analytics, Network Interfaces, Alerts, History, Reports, Settings, and About.
   - The top control bar provides target selection, SLA profile selection, probe interval input, and Start/Stop buttons.

2. **Starting Live Monitoring**:
   - In the top bar, select a target (e.g. `Cloudflare DNS (1.1.1.1 [icmp])`).
   - Choose a QoS Profile (e.g. `video_conferencing`, `gaming`, or `voip`).
   - Click **Start Monitoring**.
   - Notice that the window remains fluid and responsive:
     - The **MEAN RTT LATENCY** KPI updates with live millisecond timing.
     - The **PACKET LOSS** card shows real loss percentage.
     - The **JITTER ESTIMATE** card displays RFC 1889/3550 inter-packet delay variation.
     - The **QoS SLA STATUS** card displays whether the connection meets the profile's SLA standards.
     - The real-time vector chart plots every probe with latency points and dropped packet indicators.
     - The observation log records each sequence number, UTC timestamp, RTT, and resolved IP.

3. **Multi-Target Configuration**:
   - Click **Targets** on the sidebar.
   - Enter a target name, hostname/IP (e.g. `8.8.8.8` or `google.com`), select probe type (`icmp` or `tcp`), and port (`80` or `443`).
   - Click **+ Add Target**.
   - The new target is saved to the SQLite database and is immediately selectable for monitoring.

4. **Network Interface Traffic**:
   - Click **Network Interfaces** on the sidebar.
   - Shows active physical/virtual network adapters retrieved from the IP Helper API with live InOctets, OutOctets, Rx rate (Mbps), and Tx rate (Mbps).

5. **QoS SLA Profiles**:
   - Click **QoS Analytics** to inspect the threshold bounds (good vs. degraded vs. poor) for Video Conferencing, VoIP, Gaming, Browsing, and File Transfer.

6. **History & CSV Export**:
   - Stop monitoring and click **History** to view all persistent sessions.
   - Click **Reports**, enter a filename (e.g., `netpulse_export.csv`), and click **Export CSV**.
   - Inspect the exported CSV in Excel or Notepad to demonstrate real RFC 4180 structured data.

---

## 7. Known Limitations & Academic Notes

- **One-Way vs. Two-Way Jitter**: In accordance with RFC 1889/3550, jitter in this application is estimated as the mean absolute difference between consecutive round-trip observations ($|\text{RTT}_i - \text{RTT}_{i-1}|$). True one-way RTP jitter requires synchronized clocks on both client and server.
- **Interface Throughput**: Interface rates represent total traffic passing through the selected network interface adapter via Windows IP Helper counters, not end-to-end bandwidth to the destination server.
- **Target Authorization**: Measurements should only be run against authorized test servers, local networks, or public DNS providers.
