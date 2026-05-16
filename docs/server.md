# Модуль `webtop::server`

## Обзор

Модуль предоставляет HTTP-сервер для веб-интерфейса WebTop. Сервер отдаёт
статические файлы (HTML, CSS, JS) и предоставляет SSE (Server Send Events)
эндпоинты для потоковой передачи метрик системы в реальном времени.

## Компоненты

### Класс `HttpServer`

Главный класс HTTP-сервера. Управляет сокетом, обрабатывает входящие соединения,
маршрутизирует запросы.

#### Конструктор

```cpp
HttpServer(aggregator::MetricsAggregator& aggregator, int port,
           std::string static_dir, int common_update_rate, int procs_update_rate)
```

**Параметры**:

- `aggregator` - ссылка на агрегатор метрик
- `port` - порт для прослушивания
- `static_dir` - корневая директория со статическими файлами
- `common_update_rate` - интервал обновления общих метрик (с)
- `procs_update_rate` - интервал обновления процессов (с)

#### Методы

| Метод     | Описание                                       |
| --------- | ---------------------------------------------- |
| `Start()` | Запускает сервер. Возвращает `true` при успехе |
| `Stop()`  | Останавливает сервер и закрывает соединения    |

#### Внутренние маршруты

<!-- markdownlint-disable MD013 -->

| Путь              | Обработчик         | Описание                                         |
| ----------------- | ------------------ | ------------------------------------------------ |
| `/api/dashboard`  | `MetricsHandler`   | SSE-поток метрик дашборда общих ресурсов системы |
| `/api/procs`      | `ProcessesHandler` | SSE-поток данных о процессах                     |
| `/`               | `StaticHandler`    | Перенаправление на `/procs.html`                 |
| `/dashboard.html` | `StaticHandler`    | HTML страница дашборда                           |
| `/procs.html`     | `StaticHandler`    | HTML страница процессов                          |
| `/css/*.css`      | `StaticHandler`    | CSS файлы                                        |
| `/js/*.js`        | `StaticHandler`    | JavaScript файлы                                 |

<!-- markdownlint-enable MD013 -->

### Класс `Router`

Маршрутизатор HTTP-запросов.

**Методы**:

| Метод                         | Описание                                  |
| ----------------------------- | ----------------------------------------- |
| `AddRoute(path, handler)`     | Добавляет маршрут с функцией-обработчиком |
| `Route(client_fd, path)`      | Выполняет маршрутизацию запроса           |
| `SetNotFoundHandler(handler)` | Устанавливает обработчик для 404          |

### Абстрактный класс `RequestHandler`

Базовый класс для всех обработчиков запросов.

```cpp
virtual void Handle(int client_fd, const std::string& path) = 0;
```

### Абстрактный класс `SseHandler`

Наследуется от `RequestHandler`. Реализует цикл отправки событий через
Server-Sent Events. Является базовым классом для всех SSE-обработчитков.

**Конструктор**: `SseHandler(std::atomic<bool>& running, int update_interval)`

**Методы**:

- `Handle(client_fd, path)` - устанавливает SSE-соединение и запускает цикл
  отправки данных
- `GetData()` - виртуальный метод, возвращает JSON-строку с данными. Должен быть
  переопределен в классах-наследниках

**Особенности**:

- Устанавливает заголовки `Content-Type: text/event-stream`,
  `Cache-Control: no-cache`
- Отправляет данные в формате `data: <json>\r\n\r\n`
- При обрыве соединения (EPIPE, ECONNRESET) корректно завершает цикл отправки
  SSE ответов

### Класс `MetricsHandler`

Наследуется от `SseHandler`. Обрабатывает `/api/dashboard`.

**Метод `GetData()`** возвращает JSON со следующими полями:

<!-- markdownlint-disable MD013 -->

| Категория                       | Поля                                                                                                        |
| ------------------------------- | ----------------------------------------------------------------------------------------------------------- |
| `timestamp`, `uptime_formatted` | Временные метки                                                                                             |
| `cpu`                           | `usage_percent`, `user_percent`, `system_percent`, `idle_percent`, `iowait_percent`, `cores_percent`        |
| `memory`                        | `total_mb`, `used_mb`, `available_mb`, `used_percent`, `swap_total_mb`, `swap_used_mb`, `swap_used_percent` |
| `loadavg`                       | `load_1min`, `load_5min`, `load_15min`                                                                      |
| `network`                       | `rx_mbps`, `tx_mbps`, `rx_bytes_total`, `tx_bytes_total`                                                    |
| `disks`                         | Массив объектов с `device`, `mount_point`, `total_mb`, `used_mb`, `free_mb`, `used_percent`                 |
| `processes`                     | `total_count`, `thread_count`                                                                               |

<!-- markdownlint-enable MD013 -->

### Класс `ProcessesHandler`

Наследуется от `SseHandler`. Обрабатывает `/api/procs`.

**Метод `GetData()`** возвращает JSON со следующими полями:

<!-- markdownlint-disable MD013 -->

| Поле                                                               | Описание                                                                                                                                 |
| ------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------- |
| `timestamp`                                                        | Время снимка                                                                                                                             |
| `total_count`, `thread_count`                                      | Всего процессов и потоков                                                                                                                |
| `running_total`, `sleeping_total`, `stopped_total`, `zombie_total` | Количество процессов по состояниям                                                                                                       |
| `cpu`                                                              | Данные CPU (аналогично `MetricsHandler`)                                                                                                 |
| `memory`                                                           | Данные памяти (аналогично `MetricsHandler`)                                                                                              |
| `processes`                                                        | Массив объектов с полями: `pid`, `ppid`, `name`, `user`, `state`, `memory` (MB), `vmem` (MB), `cpu_percent`, `threads`, `uptime_seconds` |

<!-- markdownlint-enable MD013 -->

### Класс `StaticHandler`

Наследуется от `RequestHandler`. Отдаёт статические файлы.

**Конструктор**: `explicit StaticHandler(std::string static_root)`

**Методы**:

| Метод                     | Описание            |
| ------------------------- | ------------------- |
| `Handle(client_fd, path)` | Отдаёт файл или 404 |

**Поддерживаемые MIME-типы**:

| Расширение | MIME-тип                 |
| ---------- | ------------------------ |
| `.css`     | `text/css`               |
| `.js`      | `application/javascript` |
| `.html`    | `text/html`              |
| `.json`    | `application/json`       |
| `.ico`     | `image/x-icon`           |
| прочие     | `text/plain`             |

**Кэширование**:

- HTML: `Cache-Control: no-cache`
- Остальные файлы: `Cache-Control: max-age=3600`

---

## Потокобезопасность

- Сервер использует `ppoll()` для ожидания соединений
- Каждый клиент обрабатывается в отдельном потоке (`std::thread::detach()`)
- SSE-обработчики проверяют флаг `running_` в цикле отправки
- При остановке сервера через `Stop()` флаг `running_` сбрасывается, соединения
  закрываются

## Зависимости

- Модуль `webtop::aggregator` - для получения метрик
- Модуль `webtop::logger` - для логирования
- Модуль `webtop::util` - для `ScopeGuard`
- Библиотека `nlohmann/json` - для сериализации JSON
- POSIX: `socket`, `bind`, `listen`, `accept`, `ppoll`, `pthread_sigmask`
