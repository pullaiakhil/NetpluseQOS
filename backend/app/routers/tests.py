from datetime import datetime, timezone
import statistics
from fastapi import APIRouter, HTTPException, status

from app.adapters.c_engine_adapter import CEngineAdapter
from app.adapters.icmp_adapter import IcmpAdapter
from app.adapters.tcp_adapter import TcpAdapter
from app.schemas.schemas import (
    PingItem,
    PingTestRequest,
    PingTestResponse,
    TcpTestRequest,
    TcpTestResponse,
)
from app.services.qos_engine import QoSEngine

router = APIRouter(prefix="/tests", tags=["Diagnostics"])


@router.post("/tcp", response_model=TcpTestResponse)
async def run_tcp_test(payload: TcpTestRequest):
    """Execute on-demand TCP handshake test to a host and port."""
    if CEngineAdapter.is_available():
        c_res = await CEngineAdapter.execute_probe(
            mode="tcp",
            host=payload.host,
            port=payload.port,
            count=1,
            timeout_seconds=payload.timeout,
        )
        if c_res and c_res.probes:
            p = c_res.probes[0]
            return TcpTestResponse(
                host=payload.host,
                port=payload.port,
                success=p.success,
                connect_time_ms=p.rtt_ms,
                error_type=p.error_type,
                error_detail=p.error_detail,
                timestamp=c_res.timestamp,
            )

    result = await TcpAdapter.test_connection(
        host=payload.host,
        port=payload.port,
        timeout_seconds=payload.timeout,
    )
    return TcpTestResponse(
        host=payload.host,
        port=payload.port,
        success=result.success,
        connect_time_ms=result.connect_time_ms,
        error_type=result.error_type,
        error_detail=result.error_detail,
        timestamp=result.timestamp,
    )


@router.post("/ping", response_model=PingTestResponse)
async def run_ping_test(payload: PingTestRequest):
    """Execute bounded on-demand ICMP ping sequence and summarize latency stats."""
    count = payload.count

    # Try C-native measurement engine first
    if CEngineAdapter.is_available():
        c_res = await CEngineAdapter.execute_probe(
            mode="icmp",
            host=payload.host,
            count=count,
            timeout_seconds=payload.timeout,
        )
        if c_res:
            probes = [
                PingItem(
                    sequence=p.sequence,
                    success=p.success,
                    rtt_ms=p.rtt_ms,
                    error_type=p.error_type,
                    error_detail=p.error_detail,
                )
                for p in c_res.probes
            ]
            return PingTestResponse(
                host=c_res.target_host,
                resolved_ip=c_res.resolved_ip,
                count=c_res.count,
                transmitted=c_res.transmitted,
                received=c_res.received,
                packet_loss_pct=c_res.packet_loss_pct,
                min_rtt_ms=c_res.min_rtt_ms,
                max_rtt_ms=c_res.max_rtt_ms,
                mean_rtt_ms=c_res.mean_rtt_ms,
                jitter_ms=c_res.jitter_ms,
                probes=probes,
                timestamp=c_res.timestamp,
            )
    probes = []
    successful_rtts = []
    resolved_ip = None

    for seq in range(1, count + 1):
        res = await IcmpAdapter.ping(
            host=payload.host,
            timeout_seconds=payload.timeout,
        )
        if res.resolved_ip and not resolved_ip:
            resolved_ip = res.resolved_ip

        if res.success and res.rtt_ms is not None:
            successful_rtts.append(res.rtt_ms)

        probes.append(
            PingItem(
                sequence=seq,
                success=res.success,
                rtt_ms=res.rtt_ms,
                error_type=res.error_type,
                error_detail=res.error_detail,
            )
        )

    transmitted = count
    received = len(successful_rtts)
    loss_pct = round(((transmitted - received) / transmitted) * 100.0, 2)

    min_rtt = round(min(successful_rtts), 2) if successful_rtts else None
    max_rtt = round(max(successful_rtts), 2) if successful_rtts else None
    mean_rtt = round(statistics.mean(successful_rtts), 2) if successful_rtts else None
    jitter = QoSEngine.calculate_jitter(successful_rtts)

    return PingTestResponse(
        host=payload.host,
        resolved_ip=resolved_ip,
        count=count,
        transmitted=transmitted,
        received=received,
        packet_loss_pct=loss_pct,
        min_rtt_ms=min_rtt,
        max_rtt_ms=max_rtt,
        mean_rtt_ms=mean_rtt,
        jitter_ms=jitter,
        probes=probes,
        timestamp=datetime.now(timezone.utc),
    )
