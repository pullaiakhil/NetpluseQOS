import React, { useState } from 'react';
import { PageHeader } from '@/components/layout/PageHeader';
import { Card, CardHeader, CardTitle, CardDescription } from '@/components/ui/Card';
import { Button } from '@/components/ui/Button';
import { Select } from '@/components/ui/Select';
import { StatusBadge } from '@/components/ui/StatusBadge';
import { EmptyState } from '@/components/ui/EmptyState';
import { LatencyChart } from '@/components/charts/LatencyChart';
import { JitterChart } from '@/components/charts/JitterChart';
import { PacketLossChart } from '@/components/charts/PacketLossChart';
import { useMonitoring } from '@/hooks/useMonitoring';
import { useTargets } from '@/hooks/useTargets';
import {
  Play,
  Square,
  Activity,
  Radio,
  Clock,
  Layers,
  CheckCircle2,
  AlertTriangle,
} from 'lucide-react';
import { formatMs, formatPct, formatDateTime, formatDuration } from '@/lib/utils';

export const LiveMonitoring: React.FC = () => {
  const { data: targets = [] } = useTargets();
  const {
    isRunning,
    status,
    metrics,
    startMonitoring,
    stopMonitoring,
    isStarting,
    isStopping,
  } = useMonitoring(3000);

  const [selectedTargetId, setSelectedTargetId] = useState<string>('');
  const [selectedInterval, setSelectedInterval] = useState<number>(3);
  const [errorNotice, setErrorNotice] = useState<string | null>(null);

  // Set default selected target once loaded
  React.useEffect(() => {
    if (targets.length > 0 && !selectedTargetId) {
      setSelectedTargetId(targets[0].id);
    }
  }, [targets, selectedTargetId]);

  const activeSessionInfo = (status as any)?.sessions?.[0];
  const activeTarget = isRunning
    ? status?.active_target ||
      (activeSessionInfo
        ? {
            id: String(activeSessionInfo.target_id),
            name: activeSessionInfo.target_name,
            host: activeSessionInfo.target_host,
            type: 'icmp' as const,
            enabled: true,
          }
        : targets.find((t) => t.id === selectedTargetId))
    : targets.find((t) => t.id === selectedTargetId);

  const probes = metrics?.probes || [];
  const latestProbe = probes.length > 0 ? probes[probes.length - 1] : null;

  const handleStart = async () => {
    setErrorNotice(null);
    if (!selectedTargetId) {
      setErrorNotice('Please select a target first.');
      return;
    }
    try {
      await startMonitoring({
        target_id: selectedTargetId,
        interval_seconds: selectedInterval,
      });
    } catch (err: any) {
      setErrorNotice(err.message || 'Failed to start monitoring session');
    }
  };

  const handleStop = async () => {
    setErrorNotice(null);
    try {
      const activeSid =
        status?.current_session?.id ||
        (status as any)?.sessions?.[0]?.session_id?.toString();
      await stopMonitoring({ session_id: activeSid });
    } catch (err: any) {
      setErrorNotice(err.message || 'Failed to stop monitoring session');
    }
  };

  return (
    <div className="space-y-6">
      <PageHeader
        title="Live QoS Telemetry"
        description="Continuously ping and diagnose round-trip times, delay jitter, and loss rates."
        badge={<StatusBadge status={isRunning ? 'running' : 'stopped'} />}
      />

      {errorNotice && (
        <div className="p-3 rounded-md bg-critical-bg border border-critical/30 text-critical-text text-sm flex items-center gap-2">
          <AlertTriangle className="w-4 h-4 shrink-0" />
          <span>{errorNotice}</span>
        </div>
      )}

      {/* Control Bar */}
      <Card>
        <div className="flex flex-col md:flex-row items-center justify-between gap-4">
          <div className="flex flex-wrap items-center gap-4 w-full md:w-auto">
            <div className="min-w-[220px]">
              <Select
                label="Monitoring Target"
                disabled={isRunning}
                value={
                  isRunning
                    ? String(status?.active_target?.id ?? activeSessionInfo?.target_id ?? selectedTargetId)
                    : String(selectedTargetId || '')
                }
                onChange={(e) => setSelectedTargetId(e.target.value)}
              >
                {targets.length === 0 ? (
                  <option value="">No targets configured</option>
                ) : (
                  targets.map((t) => (
                    <option key={t.id} value={t.id}>
                      {t.name} ({t.host} - {t.type.toUpperCase()})
                    </option>
                  ))
                )}
              </Select>
            </div>

            <div className="w-[140px]">
              <Select
                label="Sampling Interval"
                disabled={isRunning}
                value={selectedInterval}
                onChange={(e) => setSelectedInterval(Number(e.target.value))}
                options={[
                  { value: 1, label: '1 second' },
                  { value: 2, label: '2 seconds' },
                  { value: 3, label: '3 seconds' },
                  { value: 5, label: '5 seconds' },
                  { value: 10, label: '10 seconds' },
                  { value: 30, label: '30 seconds' },
                  { value: 60, label: '60 seconds' },
                ]}
              />
            </div>
          </div>

          <div className="flex items-center gap-3 w-full md:w-auto justify-end">
            {isRunning ? (
              <Button
                variant="danger"
                size="md"
                icon={<Square className="w-4 h-4" />}
                loading={isStopping}
                onClick={handleStop}
              >
                Stop Live Session
              </Button>
            ) : (
              <Button
                variant="primary"
                size="md"
                icon={<Play className="w-4 h-4" />}
                loading={isStarting}
                disabled={targets.length === 0}
                onClick={handleStart}
              >
                Start Live Monitoring
              </Button>
            )}
          </div>
        </div>
      </Card>

      {/* Monitoring Content */}
      {isRunning ? (
        <div className="space-y-6">
          {/* Real-time Status Card & Latest Probe stats */}
          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <Card className="flex flex-col justify-between">
              <span className="text-xs font-semibold text-text-muted uppercase">Target Host</span>
              <div className="my-2">
                <p className="text-base font-bold text-text-primary truncate">
                  {activeTarget?.name || 'Target'}
                </p>
                <p className="text-xs font-mono text-text-secondary mt-0.5">
                  {activeTarget?.host} ({activeTarget?.type?.toUpperCase()})
                </p>
              </div>
              <div className="text-[11px] text-text-muted pt-2 border-t border-border flex items-center gap-1">
                <Clock className="w-3.5 h-3.5 text-accent" />
                Started: {formatDateTime(status?.current_session?.start_time)}
              </div>
            </Card>

            <Card className="flex flex-col justify-between">
              <span className="text-xs font-semibold text-text-muted uppercase">Latest RTT</span>
              <div className="my-2">
                <span className="text-2xl font-bold tabular-nums text-text-primary">
                  {latestProbe?.rtt_ms !== null && latestProbe?.rtt_ms !== undefined
                    ? formatMs(latestProbe.rtt_ms)
                    : '—'}
                </span>
              </div>
              <div className="text-[11px] text-text-muted pt-2 border-t border-border">
                {latestProbe?.success ? (
                  <span className="text-success flex items-center gap-1">
                    <CheckCircle2 className="w-3 h-3" /> Echo reply successful
                  </span>
                ) : (
                  <span className="text-critical flex items-center gap-1">
                    <AlertTriangle className="w-3 h-3" /> Packet lost / timeout
                  </span>
                )}
              </div>
            </Card>

            <Card className="flex flex-col justify-between">
              <span className="text-xs font-semibold text-text-muted uppercase">Latest Jitter</span>
              <div className="my-2">
                <span className="text-2xl font-bold tabular-nums text-text-primary">
                  {latestProbe?.jitter_ms !== null && latestProbe?.jitter_ms !== undefined
                    ? formatMs(latestProbe.jitter_ms)
                    : '—'}
                </span>
              </div>
              <div className="text-[11px] text-text-muted pt-2 border-t border-border">
                Variation: Packet delta delay
              </div>
            </Card>

            <Card className="flex flex-col justify-between">
              <span className="text-xs font-semibold text-text-muted uppercase">Probes Dispatched</span>
              <div className="my-2 flex items-baseline gap-2">
                <span className="text-2xl font-bold tabular-nums text-text-primary">
                  {probes.length}
                </span>
                <span className="text-xs text-text-secondary">samples</span>
              </div>
              <div className="text-[11px] text-text-muted pt-2 border-t border-border flex items-center justify-between">
                <span>Loss Rate:</span>
                <span className="font-semibold text-text-primary">
                  {probes.length > 0
                    ? formatPct((probes.filter((p) => !p.success).length / probes.length) * 100)
                    : '0.0%'}
                </span>
              </div>
            </Card>
          </div>

          {/* Charts Row */}
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            <Card>
              <CardHeader>
                <div>
                  <CardTitle>Latency Telemetry (ms)</CardTitle>
                  <CardDescription>Live real-time round-trip latency stream</CardDescription>
                </div>
              </CardHeader>
              <LatencyChart data={probes} height={260} showDots={probes.length < 30} />
            </Card>

            <Card>
              <CardHeader>
                <div>
                  <CardTitle>Jitter Trend (ms)</CardTitle>
                  <CardDescription>Statistical variation in packet arrival delay</CardDescription>
                </div>
              </CardHeader>
              <JitterChart data={probes} height={260} showDots={probes.length < 30} />
            </Card>
          </div>

          {/* Loss Chart */}
          <Card>
            <CardHeader>
              <div>
                <CardTitle>Packet Loss Ratio (%)</CardTitle>
                <CardDescription>Loss percentage evaluated per sample window</CardDescription>
              </div>
            </CardHeader>
            <PacketLossChart data={probes} height={200} />
          </Card>
        </div>
      ) : (
        <Card className="py-12">
          <EmptyState
            icon={<Radio className="w-8 h-8 text-accent" />}
            title="Live Monitoring Inactive"
            description="Select a configured destination endpoint from the selector above and click 'Start Live Monitoring' to begin streaming probe packets."
            action={
              targets.length === 0 ? (
                <Button
                  variant="primary"
                  size="sm"
                  onClick={() => (window.location.href = '/targets')}
                >
                  Create Target First
                </Button>
              ) : undefined
            }
          />
        </Card>
      )}
    </div>
  );
};
