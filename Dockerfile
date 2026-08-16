# ============================================================
# Build stage
# ============================================================

FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy build configuration first.
# This gives Docker a better chance of reusing cached layers.
COPY CMakeLists.txt ./

COPY include/ ./include/
COPY src/ ./src/
COPY external/ ./external/
COPY tests/ ./tests/

RUN cmake \
    -S . \
    -B build \
    -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build --parallel "$(nproc)"


# ============================================================
# Runtime stage
# ============================================================

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libyaml-cpp0.8 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder \
    /app/build/load_balancer \
    /app/load_balancer

COPY config.yaml /app/config.yaml

EXPOSE 8080

CMD ["./load_balancer", "config.yaml"]