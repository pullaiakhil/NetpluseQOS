import React, { useState } from 'react';
import { PageHeader } from '@/components/layout/PageHeader';
import { Card, CardHeader, CardTitle, CardDescription } from '@/components/ui/Card';
import { MetricCard } from '@/components/ui/MetricCard';
import { ThroughputChart } from '@/components/charts/ThroughputChart';
import { Select } from '@/components/ui/Select';
import { Button } from '@/components/ui/Button';
import { useInterfaces } from '@/hooks/useInterfaces';
import { formatBytes, formatKbps } from '@/lib/utils';
import {
  ArrowDownLeft,
  ArrowUpRight,
  RefreshCw,
  Network,
  Activity,
  Layers,
  HardDrive,
} from 'lucide-react';

export const ThroughputAnalysis: React.FC = () => {
  const { interfaces, throughputData, historicalThroughput, refetch, isLoading } =
    useInterfaces(3000);

  const [selectedInterface, setSelectedInterface] = useState<string>('all');

  const filteredThroughput =
    selectedInterface === 'all'
      ? historicalThroughput
      : historicalThroughput.filter((t) => t.interface_name === selectedInterface);

  // Compute current total Rx/Tx
  const latestReadings = throughputData.filter(
    (t) => selectedInterface === 'all' || t.interface_name === selectedInterface
  );

  const currentRx = latestReadings.reduce((acc, curr) => acc + (curr.rx_kbps || 0), 0);
  const currentTx = latestReadings.reduce((acc, curr) => acc + (curr.tx_kbps || 0), 0);
  const totalRxBytes = interfaces.reduce((acc, curr) => acc + (curr.bytes_recv || 0), 0);
  const totalTxBytes = interfaces.reduce((acc, curr) => acc + (curr.bytes_sent || 0), 0);

  return (
    <div className="space-y-6">
      <PageHeader
        title="Throughput & Interface Bandwidth"
        description="Monitor network interface utilization, transmit/receive bitrates, and packet errors."
        actions={
          <div className="flex items-center gap-3">
            <div className="w-56">
              <Select
                value={selectedInterface}
                onChange={(e) => setSelectedInterface(e.target.value)}
              >
                <option value="all">All Interfaces (Aggregated)</option>
                {interfaces.map((iface) => (
                  <option key={iface.name} value={iface.name}>
                    {iface.name} {iface.is_up ? '(UP)' : '(DOWN)'}
                  </option>
                ))}
              </Select>
            </div>
            <Button
              variant="outline"
              size="sm"
              icon={<RefreshCw className="w-3.5 h-3.5" />}
              onClick={() => refetch()}
            >
              Refresh
            </Button>
          </div>
        }
      />

      {/* Metric Cards Row */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        <MetricCard
          title="Current Inbound Rate"
          value={formatKbps(currentRx)}
          unit=""
          subtitle="Real-time download throughput"
          status="good"
          icon={<ArrowDownLeft className="w-4 h-4 text-success" />}
          loading={isLoading}
        />

        <MetricCard
          title="Current Outbound Rate"
          value={formatKbps(currentTx)}
          unit=""
          subtitle="Real-time upload throughput"
          status="good"
          icon={<ArrowUpRight className="w-4 h-4 text-accent" />}
          loading={isLoading}
        />

        <MetricCard
          title="Total Inbound Volume"
          value={formatBytes(totalRxBytes)}
          unit=""
          subtitle="Aggregated received bytes"
          status="good"
          icon={<HardDrive className="w-4 h-4 text-text-secondary" />}
          loading={isLoading}
        />

        <MetricCard
          title="Total Outbound Volume"
          value={formatBytes(totalTxBytes)}
          unit=""
          subtitle="Aggregated transmitted bytes"
          status="good"
          icon={<HardDrive className="w-4 h-4 text-text-secondary" />}
          loading={isLoading}
        />
      </div>

      {/* Main Bandwidth Stream Chart */}
      <Card>
        <CardHeader>
          <div>
            <CardTitle>Network Bandwidth Telemetry (Rx / Tx)</CardTitle>
            <CardDescription>
              Continuous rate of inbound versus outbound network payloads
            </CardDescription>
          </div>
        </CardHeader>
        <ThroughputChart
          data={
            filteredThroughput.length > 0
              ? filteredThroughput
              : throughputData
          }
          height={320}
        />
      </Card>

      {/* Physical / Virtual Interfaces Table */}
      <Card noPadding>
        <div className="p-5 border-b border-border">
          <CardTitle>Network Interfaces Telemetry</CardTitle>
          <CardDescription>
            System NIC cards, operational state, packets count, and drop counters
          </CardDescription>
        </div>

        <div className="overflow-x-auto">
          <table className="w-full text-left text-sm">
            <thead className="bg-surface-secondary/60 text-xs font-semibold text-text-muted border-b border-border">
              <tr>
                <th className="py-3 px-4">Interface</th>
                <th className="py-3 px-4">Link State</th>
                <th className="py-3 px-4">Speed</th>
                <th className="py-3 px-4">Bytes Recv</th>
                <th className="py-3 px-4">Bytes Sent</th>
                <th className="py-3 px-4">Pkts Recv</th>
                <th className="py-3 px-4">Pkts Sent</th>
                <th className="py-3 px-4 text-right">Errors / Drops</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-border">
              {interfaces.length === 0 ? (
                <tr>
                  <td colSpan={8} className="py-6 text-center text-text-muted text-xs">
                    No physical or virtual network interfaces detected.
                  </td>
                </tr>
              ) : (
                interfaces.map((iface) => (
                  <tr key={iface.name} className="hover:bg-surface-secondary/20 transition-colors">
                    <td className="py-3 px-4 font-mono text-xs font-semibold text-text-primary">
                      {iface.name}
                    </td>
                    <td className="py-3 px-4">
                      <span
                        className={`inline-flex items-center gap-1.5 px-2 py-0.5 rounded-full text-xs font-medium border ${
                          iface.is_up
                            ? 'bg-success-bg text-success-text border-success/30'
                            : 'bg-surface-secondary text-text-secondary border-border'
                        }`}
                      >
                        <span
                          className={`h-1.5 w-1.5 rounded-full ${iface.is_up ? 'bg-success' : 'bg-text-muted'}`}
                        />
                        {iface.is_up ? 'UP' : 'DOWN'}
                      </span>
                    </td>
                    <td className="py-3 px-4 text-xs text-text-secondary">
                      {iface.speed_mbps ? `${iface.speed_mbps} Mbps` : 'Auto'}
                    </td>
                    <td className="py-3 px-4 text-xs font-mono text-text-secondary">
                      {formatBytes(iface.bytes_recv)}
                    </td>
                    <td className="py-3 px-4 text-xs font-mono text-text-secondary">
                      {formatBytes(iface.bytes_sent)}
                    </td>
                    <td className="py-3 px-4 text-xs font-mono text-text-muted">
                      {iface.packets_recv?.toLocaleString() || '0'}
                    </td>
                    <td className="py-3 px-4 text-xs font-mono text-text-muted">
                      {iface.packets_sent?.toLocaleString() || '0'}
                    </td>
                    <td className="py-3 px-4 text-xs text-right font-mono text-text-muted">
                      {(iface.errin || 0) + (iface.errout || 0)} err / {(iface.dropin || 0) + (iface.dropout || 0)} drop
                    </td>
                  </tr>
                ))
              )}
            </tbody>
          </table>
        </div>
      </Card>
    </div>
  );
};
