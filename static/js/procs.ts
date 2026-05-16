interface Process {
    cpu_percent: number;
    memory: number;
    name: string;
    user: string;
    pid: number;
    ppid: number;
    state: string;
    threads: number;
    uptime_seconds: number;
    vmem: number;
}

interface MemoryInfo {
    available_mb: number;
    swap_total_mb: number;
    swap_used_mb: number;
    swap_used_percent: number;
    total_mb: number;
    used_mb: number;
    used_percent: number;
}

interface ProcsData {
    cpu: {
        idle_percent: number;
        iowait_percent: number;
        system_percent: number;
        usage_percent: number;
        user_percent: number;
    };
    idle_total: number;
    memory: MemoryInfo;
    processes: Process[];
    running_total: number;
    sleeping_total: number;
    stopped_total: number;
    thread_count: number;
    timestamp: number;
    total_count: number;
    zombie_total: number;
}

class ProcsDashboard {
    private eventSource: EventSource | null = null;
    private searchQuery: string = '';
    private currentSortColumn: string = '';
    private currentSortDirection: 'asc' | 'desc' = 'asc';
    private processes: Process[] = [];

    init(): void {
        this.setupSearch();
        this.setupSorting();
        this.setupNavigation();
        this.connectSSE();
    }

    private setupSearch(): void {
        const searchInput = document.getElementById('searchInput') as HTMLInputElement;
        if (searchInput) {
            searchInput.addEventListener('input', (e) => {
                this.searchQuery = (e.target as HTMLInputElement).value.toLowerCase().trim();
                this.renderTable();
            });
        }
    }

    private setupSorting(): void {
        const headers = document.querySelectorAll('.sortable');
        headers.forEach(header => {
            header.addEventListener('click', () => {
                const column = header.getAttribute('data-column');
                if (!column) return;

                if (this.currentSortColumn === column) {
                    this.currentSortDirection = this.currentSortDirection === 'asc' ? 'desc' : 'asc';
                } else {
                    this.currentSortColumn = column;
                    this.currentSortDirection = 'asc';
                }

                this.updateSortIcons();
                this.renderTable();
            });
        });
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

    private updateSortIcons(): void {
        const headers = document.querySelectorAll('.sortable');
        headers.forEach(header => {
            const column = header.getAttribute('data-column');
            const icon = header.querySelector('.sort-icon');
            
            header.classList.remove('sorted');
            if (icon) {
                icon.textContent = '';
            }

            if (column === this.currentSortColumn) {
                header.classList.add('sorted');
                if (icon) {
                    icon.textContent = this.currentSortDirection === 'asc' ? '▲' : '▼';
                }
            }
        });
    }

    private sortProcesses(processes: Process[]): Process[] {
        if (!this.currentSortColumn) return processes;

        return [...processes].sort((a, b) => {
            let aValue: any = a[this.currentSortColumn as keyof Process];
            let bValue: any = b[this.currentSortColumn as keyof Process];

            if (typeof aValue === 'string') {
                aValue = aValue.toLowerCase();
                bValue = bValue.toLowerCase();
            }

            if (aValue < bValue) return this.currentSortDirection === 'asc' ? -1 : 1;
            if (aValue > bValue) return this.currentSortDirection === 'asc' ? 1 : -1;
            return 0;
        });
    }

    private connectSSE(): void {
        if (this.eventSource) {
            this.eventSource.close();
        }

        this.eventSource = new EventSource('/api/procs');

        this.eventSource.onopen = () => {
            console.log('Process SSE connection opened');
        };

        this.eventSource.onmessage = (event) => {
            if (event.data && event.data.trim()) {
                try {
                    const data: ProcsData = JSON.parse(event.data);
                    this.updateDashboard(data);
                } catch (error) {
                    console.error('Failed to parse process SSE data:', error);
                }
            }
        };

        this.eventSource.onerror = () => {
            console.error('Process SSE connection error, reconnecting...');
            this.eventSource?.close();
            this.eventSource = null;
            setTimeout(() => this.connectSSE(), 3000);
        };
    }

    public disconnectSSE(): void {
        if (this.eventSource) {
            this.eventSource.close();
            this.eventSource = null;
            console.log('Process SSE connection closed');
        }
    }

    private updateDashboard(data: ProcsData): void {
        this.updateSummary(data);
        this.updateMemory(data.memory);
        this.updateTimestamp(data.timestamp);
        console.log(data.processes[0].user);
        console.log("hERE");
        this.processes = data.processes;
        this.renderTable();
    }

    private updateSummary(data: ProcsData): void {
        const totalCount = document.getElementById('totalCount');
        const runningCount = document.getElementById('runningCount');
        const sleepingCount = document.getElementById('sleepingCount');
        const stoppedCount = document.getElementById('stoppedCount');
        const zombieCount = document.getElementById('zombieCount');
        const threadCount = document.getElementById('threadCount');

        if (totalCount) totalCount.textContent = data.total_count.toString();
        if (runningCount) runningCount.textContent = data.running_total.toString();
        if (sleepingCount) sleepingCount.textContent = data.sleeping_total.toString();
        if (stoppedCount) stoppedCount.textContent = data.stopped_total.toString();
        if (zombieCount) zombieCount.textContent = data.zombie_total.toString();
        if (threadCount) threadCount.textContent = data.thread_count.toString();
    }

    private updateMemory(memory: MemoryInfo): void {
        const memoryUsed = document.getElementById('memoryUsed');
        const memoryTotal = document.getElementById('memoryTotal');
        const memoryPercent = document.getElementById('memoryPercent');
        const swapUsed = document.getElementById('swapUsed');

        if (memoryUsed) memoryUsed.textContent = (memory.used_mb / 1024).toFixed(1);
        if (memoryTotal) memoryTotal.textContent = (memory.total_mb / 1024).toFixed(1);
        if (memoryPercent) memoryPercent.textContent = memory.used_percent.toFixed(1);
        if (swapUsed) swapUsed.textContent = memory.swap_used_percent.toFixed(1);
    }

    private updateTimestamp(timestamp: number): void {
        const timestampElement = document.getElementById('timestamp');
        if (timestampElement) {
            const date = new Date(timestamp * 1000);
            timestampElement.textContent = date.toLocaleString();
        }
    }

    private formatUptime(seconds: number): string {
        if (seconds === 0) return '-';
        
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;

        if (days > 0) {
            return `${days}d ${hours}h`;
        }
        if (hours > 0) {
            return `${hours}h ${minutes}m`;
        }
        if (minutes > 0) {
            return `${minutes}m ${secs}s`;
        }
        return `${secs}s`;
    }

    private getCpuClass(cpuPercent: number): string {
        if (cpuPercent > 10) return 'cpu-high';
        if (cpuPercent > 1) return 'cpu-medium';
        return 'cpu-low';
    }

    private renderTable(): void {
        const tbody = document.getElementById('processTableBody');
        if (!tbody) return;

        let filteredProcesses = this.processes;
        
        if (this.searchQuery) {
            filteredProcesses = this.processes.filter(proc => 
                proc.pid.toString().includes(this.searchQuery) || 
                proc.name.toLowerCase().includes(this.searchQuery)
            );
        }

        const sortedProcesses = this.sortProcesses(filteredProcesses);

        if (sortedProcesses.length === 0) {
            tbody.innerHTML = '<tr class="no-results"><td colspan="9">No processes found</td></tr>';
            return;
        }

        tbody.innerHTML = sortedProcesses.map(proc => `
            <tr data-pid="${proc.pid}" data-name="${proc.name.toLowerCase()}">
                <td class="col-pid">${proc.pid}</td>
                <td class="col-ppid">${proc.ppid}</td>
                <td class="col-name">${this.escapeHtml(proc.name)}</td>
                <td class="col-user">${this.escapeHtml(proc.user)}</td>
                <td class="col-state">
                    <span class="state-badge state-${proc.state}">${proc.state}</span>
                </td>
                <td class="col-cpu">
                    <span class="${this.getCpuClass(proc.cpu_percent)}">${proc.cpu_percent.toFixed(1)}%</span>
                </td>
                <td class="col-memory">${proc.memory.toFixed(1)}</td>
                <td class="col-vmem">${proc.vmem.toFixed(1)}</td>
                <td class="col-threads">${proc.threads}</td>
                <td class="col-uptime">${this.formatUptime(proc.uptime_seconds)}</td>
            </tr>
        `).join('');
    }

    private escapeHtml(text: string): string {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}

let procsDashboard: ProcsDashboard;

document.addEventListener('DOMContentLoaded', () => {
    procsDashboard = new ProcsDashboard();
    procsDashboard.init();
});

window.addEventListener('beforeunload', () => {
    if (procsDashboard) {
        procsDashboard.disconnectSSE();
    }
});