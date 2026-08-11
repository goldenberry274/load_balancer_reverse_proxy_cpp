# ----------------------------
# Build stage
# ----------------------------
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/
COPY external/ external/

# Include tests only if the root CMakeLists expects them
COPY tests/ tests/

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build -j$(nproc)


# ----------------------------
# Runtime stage
# ----------------------------
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libyaml-cpp0.8 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/load_balancer /app/load_balancer

COPY config.yaml /app/config.yaml

EXPOSE 8080

CMD ["./load_balancer", "config.yaml"]
