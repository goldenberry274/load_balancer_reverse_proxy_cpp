# Architecture

## Overview

This project is a C++ reverse proxy and load balancer designed to distribute incoming HTTP traffic across multiple backend servers.

The current implementation focuses on networking, concurrency, fault tolerance, configuration, and observability. It is intended as a learning project for systems programming and infrastructure engineering.

The load balancer currently supports:

* TCP socket-based client connections
* HTTP request forwarding
* Round Robin load balancing
* Backend health checks
* Automatic skipping of unhealthy backends
* Configurable backend pools through YAML
* Fixed-size thread pool for concurrent clients
* Thread-safe shared backend state
* Structured logging
* Runtime metrics
* `/status` endpoint
* Prometheus-compatible `/metrics` endpoint
* Automated tests using doctest

HTTP-aware request parsing is the next major protocol-layer feature.

---

## High-Level Architecture

```text
                       Clients
                          |
                          v
                +-------------------+
                |      Server       |
                |     Port 8080     |
                +-------------------+
                          |
                          | accept()
                          v
                +-------------------+
                |    Thread Pool    |
                |                   |
                | Worker 1          |
                | Worker 2          |
                | Worker 3          |
                | Worker 4          |
                +-------------------+
                          |
                          | handleClient()
                          v
                +-------------------+
                | Round Robin       |
                | Load Balancer     |
                +-------------------+
                    |      |      |
                    v      v      v
                 Backend Backend Backend
                  :9001   :9002   :9003
```

A separate health-checking thread periodically monitors the backend servers:

```text
              +-------------------+
              |   HealthChecker   |
              +-------------------+
                        |
          +-------------+-------------+
          |             |             |
          v             v             v
       Backend       Backend       Backend
        :9001         :9002         :9003
```

Observability is handled through runtime metrics and Prometheus:

```text
C++ Load Balancer
       |
       +---- /status
       |
       +---- /metrics
                |
                v
           Prometheus
                |
                v
        Time-series history
```

---

## Main Components

### Server

`Server` owns the listening socket and accepts incoming client connections.

Its main responsibilities are:

* Creating and binding the listening socket
* Accepting clients
* Submitting client work to the thread pool
* Handling internal routes such as `/status` and `/metrics`
* Connecting to selected backends
* Forwarding HTTP traffic
* Returning backend responses to clients
* Coordinating shutdown

The main accept loop does not process client requests directly. Instead, each accepted client socket is submitted to the thread pool.

This prevents a slow backend response from blocking the server from accepting new clients.

---

## ThreadPool

The thread pool provides concurrency using a fixed number of persistent worker threads.

Accepted clients are placed into a shared task queue:

```text
accept(client)
      |
      v
enqueue task
      |
      v
+-------------------+
|    Task Queue     |
+-------------------+
      |
      v
+-------------------+
| Worker Threads    |
+-------------------+
      |
      v
handleClient()
```

A `std::condition_variable` allows idle workers to sleep while no tasks are available.

The task queue is protected by a mutex.

The thread pool also exposes its current queue size for observability.

---

## Backend Model

Each backend currently stores:

```cpp
struct Backend {
    std::string host;
    int port;
    bool healthy = true;
    std::size_t requestsServed = 0;
};
```

The backend list is stored in a shared `std::vector<Backend>`.

The same backend state is accessed by:

* `Server`
* `RoundRobinBalancer`
* `HealthChecker`
* Status and metrics endpoints

A shared mutex protects concurrent reads and writes.

---

## Round Robin Balancer

The Round Robin balancer selects healthy backends sequentially.

Example:

```text
Request 1 -> Backend 9001
Request 2 -> Backend 9002
Request 3 -> Backend 9003
Request 4 -> Backend 9001
```

Unhealthy backends are skipped.

For example:

```text
9001 healthy
9002 unhealthy
9003 healthy
```

produces:

```text
9001
9003
9001
9003
...
```

The balancer uses a mutex because multiple thread-pool workers may call it concurrently.

The same lock also protects health state updated by the health checker.

---

## HealthChecker

`HealthChecker` runs in a background thread.

At a configurable interval it attempts to connect to every backend.

A successful connection marks the backend healthy.

A failed connection marks it unhealthy.

The load balancer then automatically avoids that backend.

This provides basic fault tolerance:

```text
Backend 1 ---- healthy
Backend 2 ---- failed
Backend 3 ---- healthy

                |
                v

New traffic is sent only to
Backend 1 and Backend 3.
```

---

## Configuration

Runtime configuration is loaded from YAML.

Example:

```yaml
listen_port: 8080

health_check:
  interval: 3

backends:
  - host: 127.0.0.1
    port: 9001

  - host: 127.0.0.1
    port: 9002

  - host: 127.0.0.1
    port: 9003
```

This allows backend addresses and server settings to change without recompiling the application.

---

## Logging

Application events are sent through a centralized logger instead of directly using `std::cout` or `std::cerr`.

Example:

```text
[INFO] Server started on port 8080
[INFO] Forwarding request to backend 127.0.0.1:9001
[WARNING] Backend 127.0.0.1:9002 became unhealthy
[ERROR] Failed to connect to backend
```

The logger uses a mutex so output generated by multiple threads does not become interleaved.

---

## Metrics

The application tracks runtime metrics using thread-safe atomic counters.

Current metrics include:

* Total requests
* Completed requests
* Failed requests
* Active connections
* Average backend latency
* Uptime
* Thread-pool queue size
* Healthy backend count
* Requests served per backend

Example `/status` response:

```text
Load Balancer Status

Total requests: 52
Completed requests: 52
Failed requests: 0
Active connections: 0
Average backend latency: 1.08 ms
Uptime: 27 seconds
Thread pool queued tasks: 0
Healthy backends: 3/3
```

---

## Prometheus Integration

The load balancer exposes:

```text
GET /metrics
```

using Prometheus-compatible text metrics.

Example:

```text
load_balancer_total_requests 50
load_balancer_failed_requests 0
load_balancer_active_connections 4
load_balancer_thread_pool_queue_size 6
load_balancer_average_backend_latency_ms 1637.3
load_balancer_healthy_backends 3
```

Prometheus periodically scrapes this endpoint and stores historical values as time-series data.

This makes it possible to analyze behavior such as:

* Queue growth during traffic spikes
* Backend latency increases
* Backend failures
* Request volume over time
* Thread-pool saturation

---

## Internal HTTP Endpoints

The proxy currently handles two internal routes.

### `/status`

Human-readable runtime diagnostics.

### `/metrics`

Machine-readable metrics intended for Prometheus.

These requests are handled directly by the load balancer and are not forwarded to backend servers.

---

## Current HTTP Handling

At the moment, incoming HTTP requests are primarily forwarded as raw byte streams.

Internal routes are detected using basic string matching.

For example:

```cpp
request.rfind("GET /status ", 0)
```

The next protocol-layer improvement is a dedicated HTTP parser.

The planned parser will convert raw requests into a structure such as:

```cpp
struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};
```

This will enable cleaner:

* Internal routing
* Request validation
* Method handling
* Header inspection
* Rate limiting
* Path-based routing
* Security rules

The project will intentionally support a practical subset of HTTP rather than attempting to fully reimplement the complete HTTP specification.

---

## Testing

Automated tests are implemented using doctest and CTest.

Current tests cover areas such as:

* Round Robin ordering
* Skipping unhealthy backends
* Empty backend handling
* Request counters
* YAML configuration parsing
* Invalid configuration handling

Tests are run with:

```bash
ctest --test-dir build --output-on-failure
```

---

## Concurrency Model

The current concurrency model combines:

```text
Main Server Thread
    |
    +---- accept()

Thread Pool
    |
    +---- client request processing

Health Checker Thread
    |
    +---- backend health monitoring

Prometheus
    |
    +---- periodically requests /metrics
```

Shared backend state is protected by a mutex.

Simple runtime counters use atomic variables.

---

## Future Architecture

Planned improvements include:

* Structured HTTP parsing
* Rate limiting
* Weighted Round Robin
* Least Connections balancing
* Per-backend latency metrics
* Request validation
* Docker Compose deployment
* Grafana dashboards
* Automated benchmark suite
* Graceful configuration reload
* TLS termination
* Connection reuse
* `epoll`-based event handling

These features will be added incrementally while keeping the existing load-balancing and observability components modular.
