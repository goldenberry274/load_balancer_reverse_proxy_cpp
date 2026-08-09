# C++ Reverse Proxy & Load Balancer

A multithreaded reverse proxy and load balancer written in modern C++.

The project is designed as a systems-programming and infrastructure project, with an emphasis on networking, concurrency, fault tolerance, observability, and extensible load-balancing strategies.

It currently supports Round Robin traffic distribution, backend health monitoring, YAML configuration, a fixed-size thread pool, structured logging, runtime metrics, and Prometheus integration.

---

## Features

* TCP socket-based reverse proxy
* Round Robin load balancing
* Automatic skipping of unhealthy backends
* Background backend health checks
* YAML-based configuration
* Fixed-size thread pool for concurrent clients
* Thread-safe shared backend state
* Structured logging
* Runtime metrics
* Human-readable `/status` endpoint
* Prometheus-compatible `/metrics` endpoint
* Per-backend request counters
* Backend latency measurement
* Thread-pool queue monitoring
* Automated tests using doctest and CTest

---

## Architecture

```text
                    Clients
                       |
                       v
              +----------------+
              |     Server     |
              |    Port 8080   |
              +----------------+
                       |
                       | accept()
                       v
              +----------------+
              |  Thread Pool   |
              |                |
              | Worker 1       |
              | Worker 2       |
              | Worker 3       |
              | Worker 4       |
              +----------------+
                       |
                       | handleClient()
                       v
              +----------------+
              |  Round Robin   |
              | Load Balancer  |
              +----------------+
                 |      |      |
                 v      v      v
              Backend Backend Backend
               :9001   :9002   :9003
```

A separate health-checking thread periodically checks the availability of each backend.

```text
HealthChecker
     |
     +-------- Backend :9001
     +-------- Backend :9002
     +-------- Backend :9003
```

The load balancer also exposes runtime metrics:

```text
C++ Load Balancer
       |
       +---- /status
       |
       +---- /metrics
                |
                v
           Prometheus
```

For a more detailed explanation, see:

```text
docs/architecture.md
```

---

## Project Structure

```text
.
├── README.md
├── CMakeLists.txt
├── config.yaml
│
├── include/
│   ├── balancer/
│   │   ├── Backend.hpp
│   │   └── RoundRobinBalancer.hpp
│   │
│   ├── config/
│   │   └── Config.hpp
│   │
│   ├── core/
│   │   ├── Server.hpp
│   │   └── ThreadPool.hpp
│   │
│   ├── health/
│   │   └── HealthChecker.hpp
│   │
│   └── observability/
│       ├── Logger.hpp
│       └── Metrics.hpp
│
├── src/
│   ├── main.cpp
│   │
│   ├── balancer/
│   │   └── RoundRobinBalancer.cpp
│   │
│   ├── config/
│   │   └── Config.cpp
│   │
│   ├── core/
│   │   ├── Server.cpp
│   │   └── ThreadPool.cpp
│   │
│   ├── health/
│   │   └── HealthChecker.cpp
│   │
│   └── observability/
│       ├── Logger.cpp
│       └── Metrics.cpp
│
├── tests/
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   ├── test_round_robin.cpp
│   └── config_test.cpp
│
├── examples/
│   └── backend_server.py
│
├── monitoring/
│   └── prometheus.yml
│
└── docs/
    ├── architecture.md
    └── benchmarks.md
```

---

## Requirements

The project is intended to run on Linux.

You will need:

* C++17-compatible compiler
* CMake 3.16+
* yaml-cpp
* pthread support
* Python 3 for the example backend servers
* Docker if using the Prometheus setup

On Ubuntu/WSL:

```bash
sudo apt update

sudo apt install -y \
    build-essential \
    cmake \
    libyaml-cpp-dev \
    python3 \
    curl
```

---

## Building

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

If using the included helper script:

```bash
bash commands_cmake.sh -cmake_rebuild
```

Run the load balancer with:

```bash
./build/load_balancer config.yaml
```

---

## Configuration

The load balancer is configured through YAML.

Example `config.yaml`:

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

Backend addresses can therefore be changed without recompiling the application.

---

## Running Example Backends

Start three test backend servers on ports `9001`, `9002`, and `9003`.

For example:

```bash
python3 examples/backend_server.py
```

If separate backend files are used, start each one in its own terminal.

The load balancer listens on:

```text
127.0.0.1:8080
```

A normal request can be made with:

```bash
curl http://127.0.0.1:8080/
```

Repeated requests should be distributed across the healthy backends using Round Robin.

Example:

```text
Hello from backend 1
Hello from backend 2
Hello from backend 3
Hello from backend 1
```

---

## Health Checking

The `HealthChecker` periodically attempts to connect to each backend.

If a backend becomes unavailable, it is marked unhealthy and removed from normal traffic rotation.

Example:

```text
9001 -> healthy
9002 -> unhealthy
9003 -> healthy
```

Requests will then alternate only between `9001` and `9003`.

Once `9002` becomes reachable again, the health checker marks it healthy and it can rejoin the rotation.

---

## Concurrent Request Handling

Incoming client connections are processed through a fixed-size thread pool.

The main server thread is responsible for accepting connections and submitting client work to the queue.

Worker threads process requests concurrently.

```text
Main Thread

accept(A) -> enqueue
accept(B) -> enqueue
accept(C) -> enqueue
accept(D) -> enqueue

Thread Pool

Worker 1 -> A
Worker 2 -> B
Worker 3 -> C
Worker 4 -> D
```

This prevents a slow backend response from blocking the server from accepting additional clients.

---

## Status Endpoint

The load balancer exposes:

```text
GET /status
```

Example:

```bash
curl http://127.0.0.1:8080/status
```

Example output:

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

The status endpoint is intended for human-readable diagnostics.

---

## Metrics Endpoint

The load balancer exposes a Prometheus-compatible endpoint:

```text
GET /metrics
```

Example:

```bash
curl http://127.0.0.1:8080/metrics
```

Example output:

```text
load_balancer_total_requests 50
load_balancer_completed_requests 50
load_balancer_failed_requests 0
load_balancer_active_connections 0
load_balancer_thread_pool_queue_size 0
load_balancer_average_backend_latency_ms 1.08
load_balancer_uptime_seconds 83
load_balancer_healthy_backends 3
load_balancer_total_backends 3

load_balancer_backend_requests{host="127.0.0.1",port="9001"} 17
load_balancer_backend_requests{host="127.0.0.1",port="9002"} 17
load_balancer_backend_requests{host="127.0.0.1",port="9003"} 16
```

Prometheus can scrape this endpoint periodically and store the values as time-series data.

---

## Prometheus

The project includes:

```text
monitoring/prometheus.yml
```

Prometheus can be started in Docker:

```bash
docker run \
    --name load-balancer-prometheus \
    --add-host=host.docker.internal:host-gateway \
    -p 9090:9090 \
    -v "$(pwd)/monitoring/prometheus.yml:/etc/prometheus/prometheus.yml:ro" \
    prom/prometheus
```

Then open:

```text
http://localhost:9090
```

Useful metrics to query include:

```text
load_balancer_total_requests
```

```text
load_balancer_active_connections
```

```text
load_balancer_thread_pool_queue_size
```

```text
load_balancer_average_backend_latency_ms
```

---

## Concurrent Load Testing

A simple concurrent request test can be run using:

```bash
seq 1 50 | xargs -I{} -P10 \
curl -s -o /dev/null http://127.0.0.1:8080/
```

A successful Round Robin distribution across three healthy backends should look approximately like:

```text
Backend 9001: 17
Backend 9002: 17
Backend 9003: 16
```

More detailed performance testing is documented in:

```text
docs/benchmarks.md
```

---

## Tests

The project uses doctest together with CTest.

Build the project:

```bash
cmake -S . -B build
cmake --build build
```

Then run:

```bash
ctest --test-dir build --output-on-failure
```

The current automated tests cover:

* Round Robin cycling
* Skipping unhealthy backends
* No healthy backend handling
* Request counters
* Invalid backend lists
* YAML configuration parsing
* Invalid configuration values

Tests can also be run directly:

```bash
./build/tests/load_balancer_tests
```

---

## Logging

The project uses a centralized thread-safe logger.

Example output:

```text
[2026-08-07 10:40:22] [INFO] Server started on port 8080
[2026-08-07 10:40:22] [INFO] Forwarding request to backend 127.0.0.1:9001
[2026-08-07 10:40:23] [WARNING] Backend 127.0.0.1:9002 became unhealthy
[2026-08-07 10:40:25] [ERROR] Failed to connect to backend
```

Centralizing logging keeps output consistent and prevents messages from multiple threads from being mixed together.

---

## Current Limitations

The project currently forwards most HTTP traffic as raw bytes and performs only basic internal-route detection.

It does not yet fully parse HTTP messages.

Other current limitations include:

* No TLS/HTTPS termination
* No connection pooling
* No persistent backend connections
* No advanced load-balancing algorithms
* No rate limiting
* No dynamic configuration reload
* No full HTTP/1.1 implementation
* No `epoll` or asynchronous I/O

These limitations are intentional and will be addressed incrementally.

---

## Roadmap

Planned features include:

* [ ] HTTP request parser
* [ ] Request validation
* [ ] Rate limiting
* [ ] Weighted Round Robin
* [ ] Least Connections balancing
* [ ] Path-based routing
* [ ] Per-backend latency metrics
* [ ] Docker Compose deployment
* [ ] Grafana dashboard
* [ ] Formal `wrk` benchmarks
* [ ] GitHub Actions CI
* [ ] Graceful configuration reload
* [ ] TLS termination
* [ ] Backend connection pooling
* [ ] `epoll`-based event loop

---

## Learning Goals

This project is intended to develop practical experience with:

* Linux socket programming
* TCP/IP networking
* HTTP
* Concurrent C++
* Threads and thread pools
* Mutexes and condition variables
* Atomics
* RAII
* Smart pointers
* CMake
* YAML configuration
* Failure handling
* Load-balancing algorithms
* Observability
* Prometheus
* Docker
* Unit testing
* Performance benchmarking
* Systems architecture

---

## Documentation

Additional documentation is available in:

```text
docs/architecture.md
docs/benchmarks.md
```

`architecture.md` describes the major components and concurrency model.
`benchmarks.md` records load-testing experiments and future benchmarking plans.

en license
