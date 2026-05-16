# Модуль `webtop::reader`

## Обзор

Модуль предоставляет набор классов для чтения и парсинга системной информации из файлов procfs (Linux /proc). Включает утилиты для работы с файлами, парсеры CPU, памяти, дисков, сети, процессов, загрузки системы и файлов окружения (.env).

## Структура

### Вспомогательные классы

#### `FileReader`

Базовый класс для чтения файлов и строковых операций.

| Метод | Описание |
|-------|----------|
| `ReadFile(path)` | Читает весь файл в строку |
| `ReadLines(path)` | Читает файл построчно, возвращает вектор строк |
| `Split(str, delimiter)` | Разделяет строку по разделителю (по умолчанию пробел) |

### Парсеры

#### `CpuParser`

Парсит информацию о CPU из `/proc/stat`.

**Структура `CpuData`**:

| Поле | Описание |
|------|----------|
| `user`, `nice`, `system`, `idle`, `iowait`, `irq`, `softirq`, `steal` | Время в тиках |
| `Total()` | Суммарное время |
| `IdleTotal()` | Время простоя (idle + iowait) |

**Методы**:

- `ReadAll()` - возвращает данные по всем CPU (первая строка - суммарный CPU)
- `ReadCores()` - возвращает данные только по отдельным ядрам

#### `MemoryParser`

Парсит информацию о памяти из `/proc/meminfo`.

**Структура `MemoryData`**:

| Поле | Описание |
|------|----------|
| `mem_total_kb`, `mem_free_kb`, `mem_available_kb` | Оперативная память |
| `mem_cached_kb`, `mem_buffers_kb` | Кэш и буферы |
| `swap_total_kb`, `swap_free_kb`, `swap_cached_kb` | Swap |

**Вспомогательные методы**:

- `MemUsedKb()`, `MemUsedMb()`, `MemUsedPercent()`
- `MemTotalMb()`, `MemFreeMb()`, `MemAvailableMb()`
- `SwapUsedKb()`, `SwapUsedMb()`, `SwapUsedPercent()`, `SwapTotalMb()`

**Метод**: `Read()` - возвращает заполненную структуру `MemoryData`.

#### `DiskParser`

Парсит информацию о дисках из `/proc/mounts` и `statvfs`.

**Структура `DiskStats`**:

| Поле | Описание |
|------|----------|
| `device`, `mount_point`, `fs_type` | Идентификация диска |
| `total_bytes`, `used_bytes`, `free_bytes`, `available_bytes` | Статистика в байтах |
| `used_percent` | Процент использования |
| `TotalMb()`, `UsedMb()`, `FreeMb()`, `AvailableMb()` | Методы для получения в MB |

**Метод**: `ReadAll()` - возвращает вектор `DiskStats` для всех физических дисков (исключает виртуальные ФС, loop-устройства, RAM-диски).

#### `NetworkParser`

Парсит сетевую статистику из `/proc/net/dev` и `/proc/net/route`.

**Структура `NetworkInterfaceStats`**:

| Поля RX | Поля TX |
|---------|---------|
| `rx_bytes`, `rx_packets`, `rx_errors`, `rx_dropped` | `tx_bytes`, `tx_packets`, `tx_errors`, `tx_dropped` |
| `rx_fifo`, `rx_frame`, `rx_compressed`, `rx_multicast` | `tx_fifo`, `tx_collisions`, `tx_carrier`, `tx_compressed` |

**Методы**:

- `ReadAll()` - возвращает статистику по всем сетевым интерфейсам
- `IsInterfaceDestDefault(iname)` - проверяет, является ли интерфейс маршрутом по умолчанию

#### `ProcessParser`

Парсит информацию о процессах из `/proc/[pid]/stat` и `/proc/[pid]/status`.

**Структура `ProcessInfo`**:

| Поле | Описание |
|------|----------|
| `pid`, `ppid` | Идентификаторы процесса и родителя |
| `name`, `state` | Имя процесса и состояние (R/S/D/Z/T) |
| `uid`, `user` | Идентификатор и имя пользователя |
| `utime`, `stime` | Время в user и kernel space (тики) |
| `cutime`, `cstime` | Время завершённых дочерних процессов |
| `vmem_bytes`, `rss_pages` | Виртуальная память и RSS (страницы) |
| `priority`, `nice` | Приоритет и niceness процесса |
| `num_threads` | Количество потоков |
| `starttime_ticks` | Время запуска процесса относительно момента старта системы в тиках |

**Методы**:

- `ReadAll()` - возвращает информацию по всем процессам
- `GetProcessCount()` - возвращает общее количество процессов
- `GetThreadCount()` - возвращает общее количество потоков

#### `LoadAvgParser`

Парсит нагрузку системы из `/proc/loadavg`.

**Структура `LoadAvgStats`**:

| Поле | Описание |
|------|----------|
| `load_1min`, `load_5min`, `load_15min` | Средняя нагрузка за 1/5/15 минут |
| `running_processes`, `total_processes` | Выполняемые / всего процессов |
| `last_pid` | Последний назначенный PID |

**Метод**: `Read()` - возвращает заполненную структуру `LoadAvgStats`.

#### `UptimeParser`

Парсит время работы системы из `/proc/uptime`.

**Структура `UptimeStats`**:

| Поле | Описание |
|------|----------|
| `uptime_seconds` | Время работы системы (секунды) |
| `idle_seconds` | Суммарное время простоя (секунды) |

**Метод**: `Read()` - возвращает структуру `UptimeStats`.

#### `DotEnvParser`

Парсит файлы `.env`.

**Метод**: `ReadAll(path)` - возвращает `std::map<std::string, std::string>` с парами ключ=значение. Игнорирует пустые строки, комментарии (начинающиеся с #) и строки без ровно одного знака '='.

## Константы

Вынесены, чтобы избежать магических чисел в коде.

| Константа | Значение | Описание |
|-----------|----------|----------|
| `kBytesInKb` | 1024 | Байтов в килобайте |
| `kBytesInMb` | 1024 * 1024 | Байтов в мегабайте |
