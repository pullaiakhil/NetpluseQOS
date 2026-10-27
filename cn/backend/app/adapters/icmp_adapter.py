import asyncio
from dataclasses import dataclass
from datetime import datetime, timezone
import platform
import re
import socket
from typing import Optional

from app.schemas.schemas import validate_hostname_or_ip


@dataclass
class PingResult:
    """Dataclass holding normalized ping execution output."""
    success: bool
    rtt_ms: Optional[float]
    error_type: Optional[str]
    error_detail: Optional[str]
    resolved_ip: Optional[str]
    timestamp: datetime


class IcmpAdapter:
    """Production-grade ICMP ping adapter wrapping native OS ping binaries."""

    # Regex patterns for parsing RTT
    WIN_RTT_PATTERN = re.compile(r"time[=<]\s*([0-9.]+)\s*ms", re.IGNORECASE)
    WIN_IP_PATTERN = re.compile(r"\[([0-9a-fA-F:.]+)\]|from\s+([0-9a-fA-F:.]+):", re.IGNORECASE)
    
    UNIX_RTT_PATTERN = re.compile(r"time=([0-9.]+)\s*ms", re.IGNORECASE)
    UNIX_IP_PATTERN = re.compile(r"\(([0-9a-fA-F:.]+)\)", re.IGNORECASE)

    @classmethod
    def _is_windows(cls) -> bool:
        return platform.system().lower() == "windows"

    @classmethod
    async def resolve_host(cls, host: str) -> Optional[str]:
        """Asynchronously resolve DNS to IPv4/IPv6 without blocking event loop."""
        try:
            loop = asyncio.get_running_loop()
            addr_info = await loop.getaddrinfo(host, None)
            if addr_info and len(addr_info) > 0:
                return addr_info[0][4][0]
        except Exception:
            pass
        return None

    @classmethod
    async def ping(cls, host: str, timeout_seconds: float = 3.0) -> PingResult:
        """Execute a single ICMP echo request using the OS ping binary."""
        # 1. Validation
        try:
            validated_host = validate_hostname_or_ip(host)
        except Exception as e:
            return PingResult(
                success=False,
                rtt_ms=None,
                error_type="INVALID_HOST",
                error_detail=str(e),
                resolved_ip=None,
                timestamp=datetime.now(timezone.utc),
            )

        is_win = cls._is_windows()
        timeout_ms = int(max(100, timeout_seconds * 1000))
        timeout_sec = max(1, int(timeout_seconds))

        if is_win:
            cmd = ["ping", "-n", "1", "-w", str(timeout_ms), validated_host]
        else:
            cmd = ["ping", "-c", "1", "-W", str(timeout_sec), validated_host]

        now_utc = datetime.now(timezone.utc)

        try:
            proc = await asyncio.create_subprocess_exec(
                *cmd,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )

            # Subprocess execution with bounded timeout
            try:
                stdout_bytes, stderr_bytes = await asyncio.wait_for(
                    proc.communicate(),
                    timeout=timeout_seconds + 1.5,
                )
            except asyncio.TimeoutError:
                try:
                    proc.kill()
                    await proc.wait()
                except Exception:
                    pass
                return PingResult(
                    success=False,
                    rtt_ms=None,
                    error_type="TIMEOUT",
                    error_detail=f"Ping command timed out after {timeout_seconds}s",
                    resolved_ip=None,
                    timestamp=now_utc,
                )

            stdout = stdout_bytes.decode(errors="replace")
            stderr = stderr_bytes.decode(errors="replace")
            combined = f"{stdout}\n{stderr}"

            # Extract resolved IP
            resolved_ip = None
            if is_win:
                ip_match = cls.WIN_IP_PATTERN.search(stdout)
                if ip_match:
                    resolved_ip = ip_match.group(1) or ip_match.group(2)
            else:
                ip_match = cls.UNIX_IP_PATTERN.search(stdout)
                if ip_match:
                    resolved_ip = ip_match.group(1)

            if not resolved_ip:
                resolved_ip = await cls.resolve_host(validated_host)

            # Parse Return Code and Output
            if proc.returncode != 0:
                lower_out = combined.lower()
                if "could not find host" in lower_out or "unknown host" in lower_out or "name or service not known" in lower_out:
                    return PingResult(
                        success=False,
                        rtt_ms=None,
                        error_type="DNS_FAILURE",
                        error_detail="Could not resolve host name",
                        resolved_ip=None,
                        timestamp=now_utc,
                    )
                if "request timed out" in lower_out or "100% packet loss" in lower_out or "100% loss" in lower_out:
                    return PingResult(
                        success=False,
                        rtt_ms=None,
                        error_type="TIMEOUT",
                        error_detail="Request timed out",
                        resolved_ip=resolved_ip,
                        timestamp=now_utc,
                    )
                if "destination host unreachable" in lower_out or "network unreachable" in lower_out:
                    return PingResult(
                        success=False,
                        rtt_ms=None,
                        error_type="HOST_UNREACHABLE",
                        error_detail="Destination network or host unreachable",
                        resolved_ip=resolved_ip,
                        timestamp=now_utc,
                    )
                if "operation not permitted" in lower_out or "permission denied" in lower_out:
                    return PingResult(
                        success=False,
                        rtt_ms=None,
                        error_type="PERMISSION_DENIED",
                        error_detail="Insufficient permissions for ICMP socket",
                        resolved_ip=resolved_ip,
                        timestamp=now_utc,
                    )
                
                return PingResult(
                    success=False,
                    rtt_ms=None,
                    error_type="EXECUTION_ERROR",
                    error_detail=f"Ping exited with code {proc.returncode}: {stdout.strip() or stderr.strip()}",
                    resolved_ip=resolved_ip,
                    timestamp=now_utc,
                )

            # Check for Windows request timed out with returncode 0 (rare, but can happen)
            if "request timed out" in combined.lower() and not cls.WIN_RTT_PATTERN.search(stdout):
                return PingResult(
                    success=False,
                    rtt_ms=None,
                    error_type="TIMEOUT",
                    error_detail="Request timed out",
                    resolved_ip=resolved_ip,
                    timestamp=now_utc,
                )

            # Successful probe - parse RTT
            rtt_match = cls.WIN_RTT_PATTERN.search(stdout) if is_win else cls.UNIX_RTT_PATTERN.search(stdout)
            if rtt_match:
                rtt_val = float(rtt_match.group(1))
                return PingResult(
                    success=True,
                    rtt_ms=rtt_val,
                    error_type=None,
                    error_detail=None,
                    resolved_ip=resolved_ip,
                    timestamp=now_utc,
                )

            # Fallback if pattern didn't match standard lines
            return PingResult(
                success=False,
                rtt_ms=None,
                error_type="PARSE_ERROR",
                error_detail="Unable to parse round trip time from ping output",
                resolved_ip=resolved_ip,
                timestamp=now_utc,
            )

        except FileNotFoundError:
            return PingResult(
                success=False,
                rtt_ms=None,
                error_type="UTILITY_NOT_FOUND",
                error_detail="System ping command not found in PATH",
                resolved_ip=None,
                timestamp=now_utc,
            )
        except Exception as e:
            return PingResult(
                success=False,
                rtt_ms=None,
                error_type="UNKNOWN_ERROR",
                error_detail=str(e),
                resolved_ip=None,
                timestamp=now_utc,
            )
