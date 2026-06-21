# ── WASM build ────────────────────────────────────────────────────────────────
FROM tamfrost/emsdk:4.0.6-utils-1 AS wasm-builder

WORKDIR /app

# Make nlohmann/json available in the emscripten include path
RUN mkdir -p /usr/local/include/nlohmann && \
    cp -r /usr/include/nlohmann/. /usr/local/include/nlohmann/

# Build GeographicLib for WASM so find_library(Geographic) works under emcmake
WORKDIR /tmp/geographiclib
RUN git clone --depth=1 --branch r2.3 \
    https://github.com/geographiclib/geographiclib.git . && \
    mkdir build && cd build && \
    emcmake cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF .. && \
    emmake make -j$(nproc) && \
    emmake make install

WORKDIR /app
COPY wrappers/wasm/ wrappers/wasm/
COPY src/ src/
COPY include/ include/

WORKDIR /app/wrappers/wasm
RUN rm -rf build && mkdir -p build && \
    cd build && \
    emcmake cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TYPE=release \
      -DCOMMIT_HASH=docker \
      -DGEOFIX_VERSION=1.0.0 \
      .. && \
    emmake make -j$(nproc)
# Output: /app/wrappers/wasm/build/fix.js  (SINGLE_FILE=1, no separate .wasm)

# ── Native build ──────────────────────────────────────────────────────────────
FROM alpine:3.18 AS builder

RUN apk add --no-cache \
    cmake make g++ gcc git \
    linux-headers \
    eigen-dev \
    libstdc++-dev \
    nlohmann-json

# GeographicLib (static)
WORKDIR /tmp/geographiclib
RUN git clone --depth=1 --branch r2.3 \
    https://github.com/geographiclib/geographiclib.git . && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF .. && \
    make -j$(nproc) && make install

# Standalone Asio (header-only, required by Crow without Boost)
WORKDIR /tmp/asio
RUN git clone --depth=1 --branch asio-1-28-0 \
    https://github.com/chriskohlhoff/asio.git . && \
    cp asio/include/asio.hpp /usr/local/include/ && \
    cp -r asio/include/asio /usr/local/include/

# Crow (header-only)
WORKDIR /tmp/crow
RUN git clone --depth=1 --branch v1.2.0 \
    https://github.com/CrowCpp/Crow.git . && \
    cp include/crow.h /usr/local/include/ && \
    cp -r include/crow /usr/local/include/

WORKDIR /app
COPY CMakeLists.txt .
COPY src/ src/
COPY include/ include/
COPY public/ public/

RUN mkdir build && cd build && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXE_LINKER_FLAGS="-static" \
      -DCMAKE_LIBRARY_PATH=/usr/local/lib \
      -DDFFIX_VERSION=1.0.0 \
      -DCOMMIT_HASH=docker \
      .. && \
    make -j$(nproc)

# ── Runtime (scratch) ─────────────────────────────────────────────────────────
FROM scratch

# Static binary
COPY --from=builder /app/build/df-fix-api /df-fix-api

# Public assets (index.html etc.)
COPY --from=builder /app/public /public

# WASM single-file bundle served at GET /api/v1/fix.js
COPY --from=wasm-builder /app/wrappers/wasm/build/fix.js /public/fix.js

EXPOSE 8080
ENTRYPOINT ["/df-fix-api"]
