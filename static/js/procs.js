"use strict";
class ProcsDashboard {
    eventSource = null;
    searchQuery = '';
    currentSortColumn = '';
    currentSortDirection = 'asc';
    processes = [];
    init() {
        this.setupSearch();
        this.setupSorting();
        this.setupNavigation();
        this.connectSSE();
    }
    setupSearch() {
        const searchInput = document.getElementById('searchInput');
        if (searchInput) {
            searchInput.addEventListener('input', (e) => {
                this.searchQuery = e.target.value.toLowerCase().trim();
                this.renderTable();
            });
        }
    }
    setupSorting() {
        const headers = document.querySelectorAll('.sortable');
        headers.forEach(header => {
            header.addEventListener('click', () => {
                const column = header.getAttribute('data-column');
                if (!column)
                    return;
                if (this.currentSortColumn === column) {
                    this.currentSortDirection = this.currentSortDirection === 'asc' ? 'desc' : 'asc';
                }
                else {
                    this.currentSortColumn = column;
                    this.currentSortDirection = 'asc';
                }
                this.updateSortIcons();
                this.renderTable();
            });
        });
    }
    setupNavigation() {
        const navLinks = document.querySelectorAll('.nav-link');
        navLinks.forEach(link => {
            link.addEventListener('click', (e) => {
                e.preventDefault();
                const href = link.getAttribute('href');
                this.disconnectSSE();
                if (href) {
                    window.location.href = href;
                }
            });
        });
    }
    updateSortIcons() {
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
    sortProcesses(processes) {
        if (!this.currentSortColumn)
            return processes;
        return [...processes].sort((a, b) => {
            let aValue = a[this.currentSortColumn];
            let bValue = b[this.currentSortColumn];
            if (typeof aValue === 'string') {
                aValue = aValue.toLowerCase();
                bValue = bValue.toLowerCase();
            }
            if (aValue < bValue)
                return this.currentSortDirection === 'asc' ? -1 : 1;
            if (aValue > bValue)
                return this.currentSortDirection === 'asc' ? 1 : -1;
            return 0;
        });
    }
    connectSSE() {
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
                    const data = JSON.parse(event.data);
                    this.updateDashboard(data);
                }
                catch (error) {
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
    disconnectSSE() {
        if (this.eventSource) {
            this.eventSource.close();
            this.eventSource = null;
            console.log('Process SSE connection closed');
        }
    }
    updateDashboard(data) {
        this.updateSummary(data);
        this.updateMemory(data.memory);
        this.updateTimestamp(data.timestamp);
        console.log(data.processes[0].user);
        console.log("hERE");
        this.processes = data.processes;
        this.renderTable();
    }
    updateSummary(data) {
        const totalCount = document.getElementById('totalCount');
        const runningCount = document.getElementById('runningCount');
        const sleepingCount = document.getElementById('sleepingCount');
        const stoppedCount = document.getElementById('stoppedCount');
        const zombieCount = document.getElementById('zombieCount');
        const threadCount = document.getElementById('threadCount');
        if (totalCount)
            totalCount.textContent = data.total_count.toString();
        if (runningCount)
            runningCount.textContent = data.running_total.toString();
        if (sleepingCount)
            sleepingCount.textContent = data.sleeping_total.toString();
        if (stoppedCount)
            stoppedCount.textContent = data.stopped_total.toString();
        if (zombieCount)
            zombieCount.textContent = data.zombie_total.toString();
        if (threadCount)
            threadCount.textContent = data.thread_count.toString();
    }
    updateMemory(memory) {
        const memoryUsed = document.getElementById('memoryUsed');
        const memoryTotal = document.getElementById('memoryTotal');
        const memoryPercent = document.getElementById('memoryPercent');
        const swapUsed = document.getElementById('swapUsed');
        if (memoryUsed)
            memoryUsed.textContent = (memory.used_mb / 1024).toFixed(1);
        if (memoryTotal)
            memoryTotal.textContent = (memory.total_mb / 1024).toFixed(1);
        if (memoryPercent)
            memoryPercent.textContent = memory.used_percent.toFixed(1);
        if (swapUsed)
            swapUsed.textContent = memory.swap_used_percent.toFixed(1);
    }
    updateTimestamp(timestamp) {
        const timestampElement = document.getElementById('timestamp');
        if (timestampElement) {
            const date = new Date(timestamp * 1000);
            timestampElement.textContent = date.toLocaleString();
        }
    }
    formatUptime(seconds) {
        if (seconds === 0)
            return '-';
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
    getCpuClass(cpuPercent) {
        if (cpuPercent > 10)
            return 'cpu-high';
        if (cpuPercent > 1)
            return 'cpu-medium';
        return 'cpu-low';
    }
    renderTable() {
        const tbody = document.getElementById('processTableBody');
        if (!tbody)
            return;
        let filteredProcesses = this.processes;
        if (this.searchQuery) {
            filteredProcesses = this.processes.filter(proc => proc.pid.toString().includes(this.searchQuery) ||
                proc.name.toLowerCase().includes(this.searchQuery));
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
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
}
let procsDashboard;
document.addEventListener('DOMContentLoaded', () => {
    procsDashboard = new ProcsDashboard();
    procsDashboard.init();
});
window.addEventListener('beforeunload', () => {
    if (procsDashboard) {
        procsDashboard.disconnectSSE();
    }
});
