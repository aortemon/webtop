declare const Chart: any;

interface DashboardData {
    cpu: {
        cores_percent: number[];
        idle_percent: number;
        iowait_percent: number;
        system_percent: number;
        usage_percent: number;
        user_percent: number;
    };
    disks: {
        device: string;
        free_mb: number;
        mount_point: string;
        total_mb: number;
        used_mb: number;
        used_percent: number;
    }[];
    loadavg: {
        load_15min: number;
        load_1min: number;
        load_5min: number;
    };
    memory: {
        available_mb: number;
        swap_total_mb: number;
        swap_used_mb: number;
        swap_used_percent: number;
        total_mb: number;
        used_mb: number;
        used_percent: number;
    };
    network: {
        rx_bytes_total: number;
        rx_mbps: number;
        tx_bytes_total: number;
        tx_mbps: number;
    };
    processes: {
        thread_count: number;
        total_count: number;
    };
    timestamp: number;
    uptime_formatted: string;
}

class Dashboard {
    private cpuChart: any = null;
    private memoryChart: any = null;
    private loadChart: any = null;
    private networkChart: any = null;
    private eventSource: EventSource | null = null;

    private readonly colors = {
        blue: 'rgb(59, 130, 246)',
        green: 'rgb(34, 197, 94)',
        yellow: 'rgb(234, 179, 8)',
        red: 'rgb(239, 68, 68)',
        purple: 'rgb(168, 85, 247)',
        cyan: 'rgb(6, 182, 212)',
    };

    init(): void {
        this.initCharts();
        this.connectSSE();
        this.setupNavigation();
    }

    private initCharts(): void {
        const cpuCtx = document.getElementById('cpuChart') as HTMLCanvasElement;
        if (cpuCtx) {
            this.cpuChart = new Chart(cpuCtx, {
                type: 'doughnut',
                data: {
                    labels: ['User', 'System', 'IOWait', 'Idle'],
                    datasets: [{
                        data: [0, 0, 0, 100],
                        backgroundColor: [
                            this.colors.blue,
                            this.colors.yellow,
                            this.colors.red,
                            this.colors.green,
                        ],
                        borderColor: '#1e1e2e',
                        borderWidth: 2,
                    }],
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: { duration: 0 },
                    plugins: {
                        legend: {
                            position: 'bottom',
                            labels: {
                                color: '#cdd6f4',
                                padding: 10,
                                font: { size: 11 },
                            },
                        },
                    },
                },
            });
        }

        const memoryCtx = document.getElementById('memoryChart') as HTMLCanvasElement;
        if (memoryCtx) {
            this.memoryChart = new Chart(memoryCtx, {
                type: 'doughnut',
                data: {
                    labels: ['Used', 'Available'],
                    datasets: [{
                        data: [0, 100],
                        backgroundColor: [this.colors.blue, this.colors.green],
                        borderColor: '#1e1e2e',
                        borderWidth: 2,
                    }],
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: { duration: 0 },
                    plugins: {
                        legend: {
                            position: 'bottom',
                            labels: {
                                color: '#cdd6f4',
                                padding: 10,
                                font: { size: 11 },
                            },
                        },
                    },
                },
            });
        }

        const loadCtx = document.getElementById('loadChart') as HTMLCanvasElement;
        if (loadCtx) {
            this.loadChart = new Chart(loadCtx, {
                type: 'bar',
                data: {
                    labels: ['1 min', '5 min', '15 min'],
                    datasets: [{
                        label: 'Load',
                        data: [0, 0, 0],
                        backgroundColor: [this.colors.blue, this.colors.purple, this.colors.cyan],
                        borderColor: '#1e1e2e',
                        borderWidth: 2,
                    }],
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: { duration: 0 },
                    plugins: {
                        legend: { display: false },
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            grid: { color: 'rgba(205, 214, 244, 0.1)' },
                            ticks: { color: '#cdd6f4' },
                        },
                        x: {
                            grid: { display: false },
                            ticks: { color: '#cdd6f4' },
                        },
                    },
                },
            });
        }

        const networkCtx = document.getElementById('networkChart') as HTMLCanvasElement;
        if (networkCtx) {
            this.networkChart = new Chart(networkCtx, {
                type: 'bar',
                data: {
                    labels: ['RX', 'TX'],
                    datasets: [{
                        label: 'Mbps',
                        data: [0, 0],
                        backgroundColor: [this.colors.green, this.colors.blue],
                        borderColor: '#1e1e2e',
                        borderWidth: 2,
                    }],
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: { duration: 0 },
                    plugins: {
                        legend: { display: false },
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            grid: { color: 'rgba(205, 214, 244, 0.1)' },
                            ticks: { color: '#cdd6f4' },
                        },
                        x: {
                            grid: { display: false },
                            ticks: { color: '#cdd6f4' },
                        },
                    },
                },
            });
        }
    }

    private setupNavigation(): void {
        const navLinks = document.querySelectorAll('.nav-link');
        navLinks.forEach(link => {
            link.addEventListener('click', (e) => {
                e.preventDefault();
                const href = (link as HTMLAnchorElement).getAttribute('href');
                this.disconnectSSE();
                if (href) {
                    window.location.href = href;
                }
            });
        });
    }

    private connectSSE(): void {
        if (this.eventSource) {
            this.eventSource.close();
        }

        this.eventSource = new EventSource('/api/dashboard');

        this.eventSource.onopen = () => {
            console.log('Dashboard SSE connection opened');
        };

        this.eventSource.onmessage = (event) => {
            if (event.data && event.data.trim()) {
                try {
                    const data: DashboardData = JSON.parse(event.data);
                    this.updateDashboard(data);
                } catch (error) {
                    console.error('Failed to parse SSE data:', error);
                }
            }
        };

        this.eventSource.onerror = () => {
            console.error('Dashboard SSE connection error, reconnecting...');
            this.eventSource?.close();
            this.eventSource = null;
            setTimeout(() => this.connectSSE(), 3000);
        };
    }

    public disconnectSSE(): void {
        if (this.eventSource) {
            this.eventSource.close();
            this.eventSource = null;
            console.log('Dashboard SSE connection closed');
        }
    }

    private updateDashboard(data: DashboardData): void {
        this.updateUptime(data.uptime_formatted, data.timestamp);
        this.updateCpuChart(data.cpu);
        this.updateMemoryChart(data.memory);
        this.updateLoadChart(data.loadavg);
        this.updateNetworkChart(data.network);
        this.updateDisks(data.disks);
        this.updateProcesses(data.processes);
    }

    private updateUptime(uptime: string, timestamp: number): void {
        const uptimeElement = document.getElementById('uptime');
        const timestampElement = document.getElementById('timestamp');
        
        if (uptimeElement) {
            uptimeElement.textContent = `Uptime: ${uptime}`;
        }
        if (timestampElement) {
            const date = new Date(timestamp * 1000);
            timestampElement.textContent = date.toLocaleString();
        }
    }

    private updateCpuChart(cpu: DashboardData['cpu']): void {
        if (!this.cpuChart) return;

        this.cpuChart.data.datasets[0].data = [
            cpu.user_percent,
            cpu.system_percent,
            cpu.iowait_percent,
            cpu.idle_percent
        ];
        this.cpuChart.update('none');

        const statsElement = document.getElementById('cpuStats');
        if (statsElement) {
            statsElement.innerHTML = `
                <div class="stat">
                    <span class="stat-label">Usage</span>
                    <span class="stat-value">${cpu.usage_percent.toFixed(1)}%</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Cores</span>
                    <span class="stat-value">${cpu.cores_percent.length}</span>
                </div>
            `;
        }
    }

    private updateMemoryChart(memory: DashboardData['memory']): void {
        if (!this.memoryChart) return;

        this.memoryChart.data.datasets[0].data = [
            memory.used_mb,
            memory.available_mb
        ];
        this.memoryChart.update('none');

        const statsElement = document.getElementById('memoryStats');
        if (statsElement) {
            const totalGB = (memory.total_mb / 1024).toFixed(1);
            const swapUsed = memory.swap_used_percent.toFixed(1);
            
            statsElement.innerHTML = `
                <div class="stat">
                    <span class="stat-label">Total</span>
                    <span class="stat-value">${totalGB} GB</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Used</span>
                    <span class="stat-value">${memory.used_percent.toFixed(1)}%</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Swap</span>
                    <span class="stat-value">${swapUsed}%</span>
                </div>
            `;
        }
    }

    private updateLoadChart(loadavg: DashboardData['loadavg']): void {
        if (!this.loadChart) return;

        this.loadChart.data.datasets[0].data = [
            loadavg.load_1min,
            loadavg.load_5min,
            loadavg.load_15min
        ];
        this.loadChart.update('none');

        const statsElement = document.getElementById('loadStats');
        if (statsElement) {
            statsElement.innerHTML = `
                <div class="stat">
                    <span class="stat-label">1 min</span>
                    <span class="stat-value">${loadavg.load_1min.toFixed(2)}</span>
                </div>
                <div class="stat">
                    <span class="stat-label">5 min</span>
                    <span class="stat-value">${loadavg.load_5min.toFixed(2)}</span>
                </div>
                <div class="stat">
                    <span class="stat-label">15 min</span>
                    <span class="stat-value">${loadavg.load_15min.toFixed(2)}</span>
                </div>
            `;
        }
    }

    private updateNetworkChart(network: DashboardData['network']): void {
        if (!this.networkChart) return;

        this.networkChart.data.datasets[0].data = [
            network.rx_mbps,
            network.tx_mbps
        ];
        this.networkChart.update('none');

        const statsElement = document.getElementById('networkStats');
        if (statsElement) {
            const rxMB = (network.rx_bytes_total / 1024 / 1024).toFixed(2);
            const txMB = (network.tx_bytes_total / 1024 / 1024).toFixed(2);
            
            statsElement.innerHTML = `
                <div class="stat">
                    <span class="stat-label">RX Total</span>
                    <span class="stat-value">${rxMB} MB</span>
                </div>
                <div class="stat">
                    <span class="stat-label">TX Total</span>
                    <span class="stat-value">${txMB} MB</span>
                </div>
            `;
        }
    }

    private updateDisks(disks: DashboardData['disks']): void {
        const container = document.getElementById('disksContainer');
        if (!container) return;

        container.innerHTML = disks.map(disk => `
            <div class="disk-item">
                <div class="disk-header">
                    <span class="disk-mount">${disk.mount_point}</span>
                    <span class="disk-device">${disk.device}</span>
                </div>
                <div class="disk-bar">
                    <div class="disk-bar-fill" style="width: ${disk.used_percent}%"></div>
                </div>
                <div class="disk-info">
                    <span>${(disk.used_mb / 1024).toFixed(1)} GB / ${(disk.total_mb / 1024).toFixed(1)} GB</span>
                    <span>${disk.used_percent.toFixed(1)}%</span>
                </div>
            </div>
        `).join('');
    }

    private updateProcesses(processes: DashboardData['processes']): void {
        const element = document.getElementById('processesInfo');
        if (!element) return;

        element.innerHTML = `
            <div class="process-stat">
                <span class="process-label">Total Processes</span>
                <span class="process-value">${processes.total_count}</span>
            </div>
            <div class="process-stat">
                <span class="process-label">Total Threads</span>
                <span class="process-value">${processes.thread_count}</span>
            </div>
        `;
    }
}

let dashboard: Dashboard;

document.addEventListener('DOMContentLoaded', () => {
    dashboard = new Dashboard();
    dashboard.init();
});

window.addEventListener('beforeunload', () => {
    if (dashboard) {
        dashboard.disconnectSSE();
    }
});