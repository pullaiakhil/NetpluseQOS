import React, { useState } from 'react';
import { PageHeader } from '@/components/layout/PageHeader';
import { Card, CardHeader, CardTitle, CardDescription } from '@/components/ui/Card';
import { Button } from '@/components/ui/Button';
import { MetricCard } from '@/components/ui/MetricCard';
import { StatusBadge } from '@/components/ui/StatusBadge';
import { EmptyState } from '@/components/ui/EmptyState';
import { LatencyChart } from '@/components/charts/LatencyChart';
import { useMonitoring } from '@/hooks/useMonitoring';
import { useQuery } from '@tanstack/react-query';
import { api } from '@/lib/api';
import {
  Download,
  Calendar,
  FileSpreadsheet,
  Clock,
  Layers,
  ChevronRight,
  BarChart3,
} from 'lucide-react';
import { formatMs, formatPct, formatDateTime, formatDuration } from '@/lib/utils';

export const Reports: React.FC = () => {
  const { sessions, isLoadingSessions } = useMonitoring();
  const [selectedSessionId, setSelectedSessionId] = useState<string | null>(null);

  // Set default selected session if available
  React.useEffect(() => {
    if (sessions.length > 0 && !selectedSessionId) {
      setSelectedSessionId(sessions[0].id);
    }
  }, [sessions, selectedSessionId]);

  const activeSessionId = selectedSessionId || (sessions[0]?.id ?? '');

  const { data: summaryData, isLoading: isLoadingSummary } = useQuery({
    queryKey: ['report-summary', activeSessionId],
    queryFn: () => api.getReportSummary(activeSessionId),
    enabled: !!activeSessionId,
  });

  const { data: historyData } = useQuery({
    queryKey: ['report-history', activeSessionId],
    queryFn: () => api.getHistory({ session_id: activeSessionId, limit: 100 }),
    enabled: !!activeSessionId,
  });

  const selectedSession = sessions.find((s) => s.id === activeSessionId);
  const probes = historyData || [];
  const stats = summaryData as any;

  const handleExportCsv = () => {
    if (!activeSessionId) return;
    const downloadUrl = api.exportCsv(activeSessionId);
    window.open(downloadUrl, '_blank');
  };

  return (
    <div className="space-y-6">
      <PageHeader
        title="Session Reports & Data Export"
        description="Audit telemetry archives, analyze statistical distributions, and download CSV logs."
        actions={
          <Button
            variant="primary"
            size="md"
            icon={<Download className="w-4 h-4" />}
            disabled={!activeSessionId}
            onClick={handleExportCsv}
          >
            Export CSV Log
          </Button>
        }
      />

      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column: Sessions List */}
        <div className="space-y-4">
          <Card noPadding>
            <div className="p-4 border-b border-border">
              <CardTitle>Completed Monitoring Runs</CardTitle>
              <CardDescription>Select a diagnostic session to load reports</CardDescription>
            </div>

            {sessions.length === 0 ? (
              <div className="p-6">
                <EmptyState
                  title="No historical sessions"
                  description="Complete a monitoring probe session to generate statistical archives."
                />
              </div>
            ) : (
              <div className="divide-y divide-border max-h-[560px] overflow-y-auto">
                {sessions.map((sess) => {
                  const isSelected = sess.id === activeSessionId;
                  return (
                    <div
                      key={sess.id}
                      onClick={() => setSelectedSessionId(sess.id)}
                      className={`p-3.5 cursor-pointer transition-colors flex items-center justify-between ${
                        isSelected
                          ? 'bg-accent/10 border-l-4 border-accent'
                          : 'hover:bg-surface-secondary/40'
                      }`}
                    >
                      <div className="space-y-1">
                        <div className="flex items-center gap-2">
                          <span className="font-semibold text-xs text-text-primary">
                            {sess.target_name || sess.target_host || 'Target'}
                          </span>
                          <StatusBadge status={sess.status} size="sm" />
                        </div>
                        <div className="text-[11px] text-text-muted flex items-center gap-1">
                          <Clock className="w-3 h-3" />
                          {formatDateTime(sess.start_time)}
                        </div>
                      </div>
                      <ChevronRight className="w-4 h-4 text-text-muted" />
                    </div>
                  );
                })}
              </div>
            )}
          </Card>
        </div>

        {/* Right Columns: Session Summary & Analytics (2 Cols) */}
        <div className="lg:col-span-2 space-y-6">
          {selectedSession ? (
            <>
              {/* Stats Row */}
              <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
                <MetricCard
                  title="Average Latency"
                  value={
                    (stats?.mean_rtt_ms ?? stats?.avg_rtt_ms) !== undefined
                      ? formatMs(stats?.mean_rtt_ms ?? stats?.avg_rtt_ms)
                      : '—'
                  }
                  subtitle={
                    stats?.min_rtt_ms !== undefined && stats?.max_rtt_ms !== undefined
                      ? `Min: ${stats.min_rtt_ms.toFixed(1)}ms · Max: ${stats.max_rtt_ms.toFixed(1)}ms`
                      : 'Sample telemetry'
                  }
                  status={
                    (stats?.mean_rtt_ms ?? stats?.avg_rtt_ms) && (stats?.mean_rtt_ms ?? stats?.avg_rtt_ms) < 60
                      ? 'good'
                      : (stats?.mean_rtt_ms ?? stats?.avg_rtt_ms) && (stats?.mean_rtt_ms ?? stats?.avg_rtt_ms) < 150
                      ? 'degraded'
                      : 'poor'
                  }
                  loading={isLoadingSummary}
                />

                <MetricCard
                  title="Average Jitter"
                  value={stats?.jitter_ms !== undefined ? formatMs(stats.jitter_ms) : '—'}
                  subtitle="Standard deviation of delay"
                  status={stats?.jitter_ms && stats.jitter_ms < 30 ? 'good' : 'degraded'}
                  loading={isLoadingSummary}
                />

                <MetricCard
                  title="Total Packet Loss"
                  value={stats?.packet_loss_pct !== undefined ? formatPct(stats.packet_loss_pct) : '—'}
                  subtitle={`${stats?.successful_probes ?? 0} of ${stats?.total_probes ?? 0} acknowledged`}
                  status={stats?.packet_loss_pct === 0 ? 'good' : 'poor'}
                  loading={isLoadingSummary}
                />
              </div>

              {/* Latency History Chart */}
              <Card>
                <CardHeader>
                  <div>
                    <CardTitle>Session Latency Timeline</CardTitle>
                    <CardDescription>
                      Full historical probe sequence ({probes.length} recorded points)
                    </CardDescription>
                  </div>
                </CardHeader>
                <LatencyChart data={probes} height={280} showDots={probes.length < 40} />
              </Card>

              {/* Quantiles & Percentiles Breakdown */}
              <Card>
                <CardHeader>
                  <CardTitle>SLA Statistical Quantiles</CardTitle>
                  <CardDescription>Percentiles & extreme response outliers</CardDescription>
                </CardHeader>
                <div className="grid grid-cols-2 sm:grid-cols-4 gap-4 text-center">
                  <div className="p-3 bg-surface-secondary/50 rounded-md">
                    <span className="text-xs text-text-muted">Median (50th %)</span>
                    <p className="text-base font-bold text-text-primary mt-1">
                      {stats?.median_rtt_ms ? formatMs(stats.median_rtt_ms) : '—'}
                    </p>
                  </div>
                  <div className="p-3 bg-surface-secondary/50 rounded-md">
                    <span className="text-xs text-text-muted">95th Percentile</span>
                    <p className="text-base font-bold text-text-primary mt-1">
                      {stats?.p95_rtt_ms ? formatMs(stats.p95_rtt_ms) : '—'}
                    </p>
                  </div>
                  <div className="p-3 bg-surface-secondary/50 rounded-md">
                    <span className="text-xs text-text-muted">99th Percentile</span>
                    <p className="text-base font-bold text-text-primary mt-1">
                      {stats?.p99_rtt_ms ? formatMs(stats.p99_rtt_ms) : '—'}
                    </p>
                  </div>
                  <div className="p-3 bg-surface-secondary/50 rounded-md">
                    <span className="text-xs text-text-muted">Alerts Raised</span>
                    <p className="text-base font-bold text-text-primary mt-1">
                      {summaryData?.alerts_count ??
                        (summaryData?.alert_counts
                          ? Object.values(summaryData.alert_counts).reduce((a: any, b: any) => a + b, 0)
                          : 0)}
                    </p>
                  </div>
                </div>
              </Card>
            </>
          ) : (
            <Card className="py-12">
              <EmptyState
                icon={<BarChart3 className="w-8 h-8 text-accent" />}
                title="No Session Selected"
                description="Select any recorded session from the left list to review detailed SLA statistics."
              />
            </Card>
          )}
        </div>
      </div>
    </div>
  );
};
