# Benchmarks

## Purpose

This document records performance and concurrency experiments for the C++ reverse proxy/load balancer.

The goal is not only to measure raw requests per second, but also to understand how architectural changes affect:

* Throughput
* Latency
* Concurrent request handling
* Thread-pool saturation
* Backend distribution
* Failure behavior

Benchmark results should be reproducible and should record the machine, configuration, backend behavior, and command used.

---

## Current Test Environment

Current development testing uses:

```text
Load balancer:
127.0.0.1:8080

Backend 1:
127.0.0.1:9001

Backend 2:
127.0.0.1:9002

Backend 3:
127.0.0.1:9003
```

The backends are lightweight Python HTTP servers used for development and testing.

The load balancer currently uses:

```text
Algorithm: Round Robin
Worker threads: 4
Health checks: enabled
Metrics: enabled
Prometheus: enabled
```

Because all services currently run locally, latency measurements should not be treated as representative of real network deployments.

---

## Round Robin Distribution Test

A 50-request concurrent test was performed with:

```bash
seq 1 50 | xargs -I{} -P10 \
curl -s -o /dev/null http://127.0.0.1:8080/
```

Observed backend distribution:

```text
Backend 9001: 17 requests
Backend 9002: 17 requests
Backend 9003: 16 requests
```

Total:

```text
17 + 17 + 16 = 50 requests
```

This demonstrates that Round Robin distributes concurrent requests approximately evenly across healthy backends.

---

## Backend Failure Test

One backend was stopped while health checking was enabled.

After the health checker marked the backend unhealthy, subsequent requests were distributed only among the remaining healthy servers.

Expected behavior with two healthy backends:

```text
Backend 1: approximately 50%
Backend 2: unavailable
Backend 3: approximately 50%
```

This confirms that the Round Robin balancer respects health state and avoids known unhealthy backends.

---

## Thread Pool Saturation Test

To make queue behavior visible, the backend servers were configured with an artificial delay of approximately two seconds.

Example backend delay:

```python
time.sleep(2)
```

Concurrent traffic was then generated.

Observed load-balancer status during the test:

```text
Total requests: 44
Completed requests: 40
Failed requests: 0
Active connections: 4
Average backend latency: 2101.54 ms
Thread pool queued tasks: 6
Healthy backends: 3/3
```

Interpretation:

```text
Active connections: 4
```

means all four worker threads were busy.

```text
Thread pool queued tasks: 6
```

means six additional requests were waiting for an available worker.

The average measured backend latency of approximately 2.1 seconds is consistent with the artificial two-second backend delay plus processing overhead.

This demonstrates that the thread pool queue provides meaningful visibility into saturation.

---

## Prometheus Metrics Test

The `/metrics` endpoint was queried during concurrent load.

Observed metrics included:

```text
load_balancer_total_requests 26
load_balancer_completed_requests 22
load_balancer_failed_requests 0
load_balancer_active_connections 4
load_balancer_thread_pool_queue_size 6
load_balancer_average_backend_latency_ms 1637.314091
load_balancer_uptime_seconds 17
load_balancer_healthy_backends 3
load_balancer_total_backends 3
```

Backend distribution at the same time:

```text
load_balancer_backend_requests{host="127.0.0.1",port="9001"} 9
load_balancer_backend_requests{host="127.0.0.1",port="9002"} 8
load_balancer_backend_requests{host="127.0.0.1",port="9003"} 8
```

The metrics confirm that Prometheus can observe:

* Active worker utilization
* Queued requests
* Backend latency
* Backend health
* Per-backend request distribution

---

## Metrics Interpretation

### Active Connections

Represents requests currently being processed by worker threads.

For a four-worker pool:

```text
Active connections = 4
```

indicates the pool may be fully occupied.

---

### Thread Pool Queue Size

Represents client tasks waiting for a worker.

```text
Queue size = 0
```

means workers are keeping up with traffic.

A continuously increasing value indicates requests are arriving faster than they can be processed.

---

### Average Backend Latency

Measures the average elapsed time spent communicating with backend servers for successful proxied requests.

Current local backend latency under normal conditions has been approximately:

```text
~1 ms
```

During artificial two-second backend delays, observed latency increased to approximately:

```text
~2.1 seconds
```

This confirms that the metric responds to actual backend slowdown.

---

### Backend Request Counts

Per-backend request totals are used to verify load-balancing behavior.

For Round Robin, counts should remain approximately equal while all backends are healthy.

---

## Planned Formal Benchmarks

The current tests validate behavior and concurrency but are not yet formal throughput benchmarks.

Future benchmarks will use tools such as:

```text
wrk
hey
ApacheBench
```

The primary benchmark tool planned is `wrk`.

Example:

```bash
wrk -t4 -c100 -d30s http://127.0.0.1:8080/
```

Future results should record:

* Requests per second
* Average latency
* Maximum latency
* Failed requests
* Transfer rate
* CPU usage
* Memory usage

---

## Planned Comparison: Single Thread vs Thread Pool

A future controlled benchmark should compare:

### Single-threaded server

```text
accept()
handleClient()
accept()
handleClient()
```

against:

### Thread-pool server

```text
accept()
enqueue()

Worker 1
Worker 2
Worker 3
Worker 4
```

The same backend implementation, traffic level, and machine should be used for both measurements.

Results should be recorded in a table such as:

| Implementation       | Requests/sec | Avg latency | Failed requests |
| -------------------- | -----------: | ----------: | --------------: |
| Single-threaded      |          TBD |         TBD |             TBD |
| 4-worker thread pool |          TBD |         TBD |             TBD |
| 8-worker thread pool |          TBD |         TBD |             TBD |

No values should be added until they have been measured.

---

## Planned Thread Count Comparison

Thread-pool sizes should also be compared:

| Workers | Requests/sec | Avg latency | Queue peak |
| ------: | -----------: | ----------: | ---------: |
|       1 |          TBD |         TBD |        TBD |
|       2 |          TBD |         TBD |        TBD |
|       4 |          TBD |         TBD |        TBD |
|       8 |          TBD |         TBD |        TBD |
|      16 |          TBD |         TBD |        TBD |

This can help identify when additional worker threads stop improving throughput.

---

## Planned Backend Failure Benchmark

Future tests should measure behavior when:

* One backend fails
* Two backends fail
* A backend becomes slow but remains healthy
* A backend recovers

Metrics of interest include:

* Failure detection time
* Failed requests during detection
* Redistribution time
* Queue growth
* Latency increase

---

## Planned Load-Balancing Algorithm Comparison

Once additional strategies are implemented, benchmark:

```text
Round Robin
Weighted Round Robin
Least Connections
```

under:

* Equal backend performance
* One slow backend
* Unequal backend capacities
* Backend failure

The objective is not only to find the highest throughput, but to understand when each algorithm performs better or worse.

---

## Benchmarking Principles

To keep benchmark results useful:

1. Run multiple trials.
2. Record the exact command.
3. Keep backend behavior consistent between comparisons.
4. Record thread-pool size.
5. Record machine specifications.
6. Separate local-development results from real-network results.
7. Do not report unmeasured numbers.
8. Keep raw benchmark output when possible.

The goal is to make performance claims reproducible rather than anecdotal.
