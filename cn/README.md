# NetPulse QoS — Quality of Service Monitoring System

> Real-time Quality of Service monitoring for network applications  
> B.Tech CSE | R22 Regulation | Computer Networks Project

## Overview

NetPulse QoS is a full-stack network monitoring application that performs **genuine network measurements** and provides real-time QoS assessment for network applications. It combines OS-level network diagnostics with a professional analytics dashboard.

### Key Features

- **C Networking Engine** — High-precision C binary for native ICMP echo and non-blocking TCP handshake latency
- **ICMP Latency Monitoring** — Real ping measurements with RTT statistics (min, max, mean, median, σ, P95)
- **Jitter Analysis** — Calculated from consecutive RTT variation: `Jitter_i = |RTT_i − RTT_(i−1)|`
- **Packet Loss Tracking** — Accurate loss percentage from actual probe success/failure counts
- **Interface Throughput** — Live Rx/Tx rates via psutil network counters
- **TCP Connectivity Tests** — Measure TCP connection establishment time to any host:port
- **QoS Assessment Profiles** — Video conferencing, VoIP, gaming, browsing, file download
- **Threshold-Based Alerts** — Configurable rules with consecutive-violation hysteresis
- **Historical Analysis** — Persistent database with charts, filters, and CSV export
- **Multiple Target Management** — Monitor multiple hosts simultaneously

### Architecture

```
React/TypeScript Frontend (Vite + Tailwind CSS)
    ↕ REST API (Proxy: /api → localhost:8000)
FastAPI Backend (Python 3.11+)
    ↕ SQLAlchemy 2.0 (aiosqlite)
SQLite Database (netpulse.db)
    ↕
Measurement Adapters:
  ├── C Native Engine (netpulse_engine.exe: Winsock2 / POSIX sockets)
  ├── Python Fallback Adapters (asyncio sockets, OS ping)
  └── psutil Interface Hardware Counters
```

## C Networking Engine Compilation (Optional but Recommended)

The core C engine is located under `c_engine/`. If a C compiler (`gcc` from MinGW/MSYS2 or MSVC `cl.exe`) is installed:

```powershell
# Using the automated Windows build script:
cd c_engine
.\build.bat

# Or compile manually with GCC:
gcc -O2 -Wall netpulse_engine.c -o netpulse_engine.exe -lws2_32 -liphlpapi
```

*Note: If no C compiler is installed, NetPulse automatically falls back to its built-in Python asyncio and OS ping adapters seamlessly without any configuration changes.*

## Quick Start (Windows)

### Prerequisites

- **Python 3.11+** — [python.org/downloads](https://www.python.org/downloads/)
- **Node.js 18+** — [nodejs.org](https://nodejs.org/)
- **Git** (optional) — for version control

### 1. Backend Setup

```powershell
cd backend
py -m venv .venv
.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
python -m uvicorn app.main:app --reload --host 127.0.0.1 --port 8000
```

The backend starts at **http://localhost:8000**. API docs available at **http://localhost:8000/docs**.

### 2. Frontend Setup

Open a **new terminal**:

```powershell
cd frontend
npm install
npm run dev
```

The frontend starts at **http://localhost:5173**. Open this URL in your browser.

### 3. Start Monitoring

1. Navigate to **Targets** → Add a target (e.g., `8.8.8.8` or `localhost`)
2. Go to **Live Monitoring** → Select target → Click **Start**
3. View real-time metrics on the **Overview** dashboard

## Quick Start (Linux / macOS)

```bash
# Backend
cd backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn app.main:app --reload --host 127.0.0.1 --port 8000

# Frontend (new terminal)
cd frontend
npm install
npm run dev
```

> **Note:** On Linux, ICMP ping may require root privileges or `cap_net_raw` capability.

## Running Tests

```powershell
# Backend tests
cd backend
.venv\Scripts\Activate.ps1
python -m pytest tests/ -v

# Frontend type check
cd frontend
npx tsc --noEmit
```

## Building for Production

```powershell
cd frontend
npm run build
# Built files are in frontend/dist/
```

## Project Structure

```
cn/
├── backend/
│   ├── app/
│   │   ├── main.py              # FastAPI application entry
│   │   ├── config.py            # Application settings
│   │   ├── database.py          # SQLAlchemy setup
│   │   ├── models/              # Database models
│   │   ├── schemas/             # Pydantic request/response schemas
│   │   ├── routers/             # API route handlers
│   │   ├── services/            # Business logic (monitoring, QoS, alerts)
│   │   └── adapters/            # Network measurement adapters
│   ├── tests/                   # Pytest test suite
│   └── requirements.txt
├── frontend/
│   ├── src/
│   │   ├── components/          # Reusable UI components
│   │   ├── pages/               # Page-level components
│   │   ├── hooks/               # React Query hooks
│   │   ├── lib/                 # API client, types, utilities
│   │   └── App.tsx              # Router and app shell
│   ├── package.json
│   └── vite.config.ts
├── docs/                        # Academic documentation
└── README.md
```

## Technology Stack

| Layer | Technology | Purpose |
|-------|-----------|---------|
| Frontend | React 18, TypeScript, Vite | UI framework |
| Styling | Tailwind CSS | Design system |
| Charts | Recharts | Data visualization |
| Icons | Lucide React | UI iconography |
| State | TanStack Query | Server state management |
| Backend | FastAPI, Uvicorn | REST API server |
| ORM | SQLAlchemy 2.x | Database abstraction |
| Database | SQLite | Persistent storage |
| Validation | Pydantic v2 | Request/response validation |
| Network | OS ping, psutil, sockets | Genuine measurements |
| Testing | Pytest, HTTPX | Automated tests |

## Network Measurement Methods

### ICMP Latency
Uses the operating system's native `ping` command via safe subprocess execution. Parses RTT from command output. Platform-specific arguments are auto-detected.

### Jitter
Calculated as the mean of absolute differences between consecutive successful RTT measurements:
```
Jitter = mean(|RTT_i − RTT_(i−1)|)  for consecutive successful probes
```
This is an RTT-variation estimate, not equivalent to the standardized one-way RTP jitter metric.

### Packet Loss
```
Packet Loss (%) = (Failed Probes / Total Probes) × 100
```
Only reports when probes have actually been sent. Failed probes are not counted as successful RTT measurements.

### Interface Throughput
```
Throughput (Mbps) = (ΔBytes × 8) / (ΔSeconds × 1,000,000)
```
Uses psutil byte counters. This measures traffic through a network interface, not end-to-end internet speed.

## Common Issues

| Problem | Solution |
|---------|----------|
| `ping` not recognized | Ensure system PATH includes Windows\System32 |
| Permission denied (Linux ping) | Run with `sudo` or set `cap_net_raw` |
| Backend won't start | Check Python version: `python --version` (need 3.11+) |
| Frontend can't reach backend | Ensure backend is running on port 8000 |
| Port 8000 in use | Stop other services or use `--port 8001` |
| npm install fails | Clear cache: `npm cache clean --force` and retry |

## Limitations

- ICMP measurements require OS ping utility and may need elevated privileges on Linux
- Interface throughput measures local NIC traffic, not end-to-end bandwidth
- Jitter calculation uses RTT variation, not one-way delay variation
- QoS assessments are threshold-based estimates, not guarantees of application performance
- Application designed for authorized local/lab network testing only

## License

Academic project — B.Tech CSE, R22 Regulation, Computer Networks.

## Contributors

- Akhil Pullai — Development & Design
