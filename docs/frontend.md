# Модуль WebTop Web UI

## Обзор

Веб-интерфейс WebTop состоит из двух страниц: дашборд (`dashboard.html`) для отображения основных системных метрик и страница процессов (`procs.html`) для детального просмотра списка процессов. Интерфейс использует Server-Sent Events (SSE) для получения данных в реальном времени.

## Структура файлов

| Файл | Описание |
|------|----------|
| `dashboard.html` | Главная страница с графиками метрик |
| `procs.html` | Страница со списком процессов |
| `dashboard.css` | Стили для дашборда |
| `procs.css` | Стили для страницы процессов |
| `dashboard.ts` | TypeScript код дашборда (компилируется в `dashboard.js`) |
| `procs.ts` | TypeScript код страницы процессов (компилируется в `procs.js`) |

## Страница дашборда (`dashboard.html`)

### Структура

- **Шапка**: заголовок, навигация (Dashboard / Processes), время работы системы, текущее время
- **Сетка карточек** (6 карточек):

| Карточка | Содержимое |
|----------|-------------|
| CPU Usage | Круговая диаграмма (User, System, IOWait, Idle) + статистика (Usage %, количество ядер) |
| Memory | Круговая диаграмма (Used, Available) + статистика (Total GB, Used %, Swap %) |
| Load Average | Столбчатая диаграмма (1, 5, 15 минут) + числовые значения |
| Network Traffic | Столбчатая диаграмма (RX, TX в Mbps) + общие RX/TX в MB |
| Disks | Список дисков с полосой использования |
| Processes | Общее количество процессов и потоков |

### Класс `Dashboard` (TypeScript)

**Свойства**:
- `cpuChart`, `memoryChart`, `loadChart`, `networkChart` - объекты Chart.js
- `eventSource` - SSE соединение
- `colors` - цветовая схема для графиков

**Методы**:

| Метод | Описание |
|-------|----------|
| `init()` | Инициализация: создание графиков, подключение SSE, настройка навигации |
| `initCharts()` | Создание 4 графиков Chart.js (doughnut/bar) |
| `connectSSE()` | Подключение к `/api/dashboard` |
| `disconnectSSE()` | Закрытие SSE соединения |
| `updateDashboard(data)` | Обновление всех виджетов |
| `updateUptime(uptime, timestamp)` | Обновление времени работы |
| `updateCpuChart(cpu)` | Обновление CPU графика и статистики |
| `updateMemoryChart(memory)` | Обновление графика памяти |
| `updateLoadChart(loadavg)` | Обновление графика нагрузки |
| `updateNetworkChart(network)` | Обновление сетевого графика |
| `updateDisks(disks)` | Отрисовка списка дисков |
| `updateProcesses(processes)` | Обновление статистики процессов |

### SSE данные для дашборда

Интерфейс `DashboardData`:

```
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
```

### Зависимости

- `Chart.js`. Подгружается динамически из CDN при загрузке dashboard.html браузером

## Страница процессов (`procs.html`)

### Структура

- **Шапка**: заголовок, навигация, сводка по процессам (Total, Running, Sleeping, Stopped, Zombie, Threads)
- **Строка памяти**: Memory Used/Total GB, Memory %, Swap Used %
- **Поиск и время**: поле поиска по имени/PID, временная метка
- **Таблица процессов**: 10 колонок с сортировкой

### Колонки таблицы

| Колонка | Поле данных | Описание |
|---------|-------------|----------|
| PID | `pid` | Идентификатор процесса |
| PPID | `ppid` | Идентификатор родителя |
| Name | `name` | Имя процесса |
| User | `user` | Владелец |
| State | `state` | Состояние (R/S/D/T/Z/I) |
| CPU% | `cpu_percent` | Загрузка CPU |
| RSS (MB) | `memory` | Резидентная память |
| VMEM (MB) | `vmem` | Виртуальная память |
| Threads | `threads` | Количество потоков |
| Uptime | `uptime_seconds` | Время жизни процесса |

### Класс `ProcsDashboard` (TypeScript)

**Свойства**:
- `eventSource` - SSE соединение
- `searchQuery` - строка поиска
- `currentSortColumn`, `currentSortDirection` - параметры сортировки
- `processes` - массив процессов

**Методы**:

| Метод | Описание |
|-------|----------|
| `init()` | Инициализация: поиск, сортировка, навигация, SSE |
| `setupSearch()` | Обработчик поиска по имени/PID |
| `setupSorting()` | Обработчики кликов по заголовкам таблицы |
| `setupNavigation()` | Переключение между страницами |
| `updateSortIcons()` | Обновление иконок сортировки (▲/▼) |
| `sortProcesses()` | Сортировка массива процессов |
| `connectSSE()` | Подключение к `/api/procs` |
| `disconnectSSE()` | Закрытие SSE соединения |
| `updateDashboard(data)` | Обновление всех данных |
| `updateSummary(data)` | Обновление сводки по процессам |
| `updateMemory(memory)` | Обновление статистики памяти |
| `updateTimestamp(timestamp)` | Обновление временной метки |
| `formatUptime(seconds)` | Форматирование времени |
| `getCpuClass(cpuPercent)` | CSS класс для цвета CPU: `cpu-high` (>10%), `cpu-medium` (>1%), `cpu-low` |
| `renderTable()` | Отрисовка таблицы с фильтрацией и сортировкой |
| `escapeHtml()` | Экранирование HTML для безопасности |

### SSE данные для страницы процессов

Интерфейс `ProcsData`:

```TypeScript
interface ProcsData {
    cpu: {
        idle_percent: number;
        iowait_percent: number;
        system_percent: number;
        usage_percent: number;
        user_percent: number;
    };
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
```

## CSS стилизация

### Цветовая схема (темная тема)

| Элемент | Цвет |
|---------|------|
| Фон страницы | `#11111b` |
| Фон карточек/таблицы | `#1e1e2e` |
| Фон элементов | `#181825` |
| Основной текст | `#cdd6f4` |
| Второстепенный текст | `#a6adc8` |
| Границы | `#313244`, `#45475a` |
| Акцент (активные ссылки) | `#3b82f6` (синий) |

### Цвета CPU нагрузки

| Класс | Условие | Цвет |
|-------|---------|------|
| `cpu-high` | >10% | `#ef4444` (красный) |
| `cpu-medium` | >1% | `#eab308` (жёлтый) |
| `cpu-low` | ≤1% | `#cdd6f4` (белый) |

### Бейджи состояний процессов

| Состояние | Значение | Цвет |
|-----------|----------|------|
| R | Running | зелёный |
| S | Sleeping | синий |
| D | Uninterruptible sleep | жёлтый |
| T | Stopped | фиолетовый |
| Z | Zombie | красный |
| I | Idle | голубой |

## Особенности реализации

### SSE соединение

- Автоматическое переподключение при ошибке (таймаут 3 секунды)
- Корректное закрытие перед переходом на другую страницу
- Обработка `beforeunload` для закрытия соединения

### Сортировка

- Поддерживается по любой колонке
- Чередование направления (asc → desc)
- Визуальная индикация текущей колонки и направления

### Поиск

- Поиск по имени процесса и PID
- Регистронезависимый
- Фильтрация в реальном времени при вводе

## Зависимости

- **Chart.js v4.4.7** - построение графиков (CDN)
- TypeScript должен быть транспилирован в JavaScript в своей директории в одноменные .js файлы (это уже сделано)
