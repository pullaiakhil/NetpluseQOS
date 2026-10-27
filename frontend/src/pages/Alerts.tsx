import React, { useState } from 'react';
import { PageHeader } from '@/components/layout/PageHeader';
import { Card, CardHeader, CardTitle, CardDescription } from '@/components/ui/Card';
import { Button } from '@/components/ui/Button';
import { Select } from '@/components/ui/Select';
import { EmptyState } from '@/components/ui/EmptyState';
import { useAlerts } from '@/hooks/useAlerts';
import { AlertSeverity } from '@/lib/types';
import {
  Bell,
  AlertTriangle,
  AlertCircle,
  Info,
  CheckCircle2,
  Check,
  ShieldCheck,
} from 'lucide-react';
import { formatDateTime } from '@/lib/utils';

export const Alerts: React.FC = () => {
  const [severityFilter, setSeverityFilter] = useState<string>('all');
  const { alerts, acknowledgeAlert, resolveAlert, isLoading } = useAlerts(
    severityFilter !== 'all' ? { severity: severityFilter } : undefined
  );

  const filteredAlerts = alerts.filter(
    (a) => severityFilter === 'all' || a.severity === severityFilter
  );

  const getSeverityBadge = (severity: AlertSeverity) => {
    switch (severity) {
      case 'critical':
        return (
          <span className="inline-flex items-center gap-1 px-2.5 py-0.5 rounded-full text-xs font-semibold bg-critical-bg text-critical-text border border-critical/30">
            <AlertCircle className="w-3.5 h-3.5" /> Critical
          </span>
        );
      case 'warning':
        return (
          <span className="inline-flex items-center gap-1 px-2.5 py-0.5 rounded-full text-xs font-semibold bg-warning-bg text-warning-text border border-warning/30">
            <AlertTriangle className="w-3.5 h-3.5" /> Warning
          </span>
        );
      case 'info':
      default:
        return (
          <span className="inline-flex items-center gap-1 px-2.5 py-0.5 rounded-full text-xs font-semibold bg-info-bg text-info-text border border-info/30">
            <Info className="w-3.5 h-3.5" /> Info
          </span>
        );
    }
  };

  return (
    <div className="space-y-6">
      <PageHeader
        title="Alerts & Threshold Violations"
        description="Real-time incident feed triggered by QoS metric breaches and network anomalies."
        actions={
          <div className="flex items-center gap-3">
            <span className="text-xs text-text-secondary font-medium">Filter Severity:</span>
            <div className="w-40">
              <Select
                value={severityFilter}
                onChange={(e) => setSeverityFilter(e.target.value)}
                options={[
                  { value: 'all', label: 'All Severities' },
                  { value: 'critical', label: 'Critical Only' },
                  { value: 'warning', label: 'Warning Only' },
                  { value: 'info', label: 'Informational' },
                ]}
              />
            </div>
          </div>
        }
      />

      <Card noPadding>
        <div className="p-5 border-b border-border flex items-center justify-between">
          <div>
            <CardTitle>Security & SLA Violations</CardTitle>
            <CardDescription>
              {filteredAlerts.length} total active incidents reported
            </CardDescription>
          </div>
        </div>

        {filteredAlerts.length === 0 ? (
          <div className="p-8">
            <EmptyState
              icon={<ShieldCheck className="w-8 h-8 text-success" />}
              title="No alerts recorded. Network is performing within thresholds."
              description="All latency, packet loss, and jitter thresholds are healthy."
            />
          </div>
        ) : (
          <div className="overflow-x-auto">
            <table className="w-full text-left text-sm">
              <thead className="bg-surface-secondary/60 text-xs font-semibold text-text-muted border-b border-border">
                <tr>
                  <th className="py-3 px-4">Severity</th>
                  <th className="py-3 px-4">Timestamp</th>
                  <th className="py-3 px-4">Target</th>
                  <th className="py-3 px-4">Metric</th>
                  <th className="py-3 px-4">Measured vs Limit</th>
                  <th className="py-3 px-4">Message</th>
                  <th className="py-3 px-4 text-right">Actions</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-border">
                {filteredAlerts.map((alt) => (
                  <tr
                    key={alt.id}
                    className={`hover:bg-surface-secondary/20 transition-colors ${
                      alt.resolved ? 'opacity-60 bg-surface-secondary/10' : ''
                    }`}
                  >
                    <td className="py-3 px-4">{getSeverityBadge(alt.severity)}</td>
                    <td className="py-3 px-4 text-xs font-mono text-text-muted whitespace-nowrap">
                      {formatDateTime(alt.timestamp)}
                    </td>
                    <td className="py-3 px-4 font-medium text-text-primary">
                      {alt.target_name || alt.target_id}
                    </td>
                    <td className="py-3 px-4 uppercase text-xs font-semibold text-text-secondary">
                      {alt.metric}
                    </td>
                    <td className="py-3 px-4 text-xs font-mono">
                      <span className="font-semibold text-critical">
                        {alt.actual_value} {alt.unit}
                      </span>{' '}
                      <span className="text-text-muted">/ limit {alt.threshold_value} {alt.unit}</span>
                    </td>
                    <td className="py-3 px-4 text-xs text-text-secondary max-w-xs truncate">
                      {alt.message}
                    </td>
                    <td className="py-3 px-4 text-right space-x-2 whitespace-nowrap">
                      {!alt.acknowledged && (
                        <Button
                          variant="outline"
                          size="sm"
                          onClick={() => acknowledgeAlert(alt.id)}
                        >
                          Ack
                        </Button>
                      )}
                      {!alt.resolved ? (
                        <Button
                          variant="secondary"
                          size="sm"
                          icon={<Check className="w-3.5 h-3.5 text-success" />}
                          onClick={() => resolveAlert(alt.id)}
                        >
                          Resolve
                        </Button>
                      ) : (
                        <span className="text-xs text-success font-medium inline-flex items-center gap-1">
                          <CheckCircle2 className="w-3.5 h-3.5" /> Resolved
                        </span>
                      )}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </Card>
    </div>
  );
};
