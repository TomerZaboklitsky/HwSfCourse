#!/bin/bash

set -e

# ── Step 1: Download FlameGraph tools if not present ─────────────────────────
if [ ! -f "flamegraph.pl" ]; then
    echo "[*] Downloading FlameGraph tools..."
    git clone --depth=1 https://github.com/brendangregg/FlameGraph.git
    cp FlameGraph/flamegraph.pl .
    cp FlameGraph/stackcollapse-perf.pl .
fi

# ── Step 2: Compile both versions ────────────────────────────────────────────
echo "[*] Compiling not_optimized_version..."
g++ -std=c++17 -O0 -g -fno-omit-frame-pointer -o not_optimized_version not_optimized_version.cpp -Wall

echo "[*] Compiling optimized_version..."
g++ -std=c++17 -O0 -g -fno-omit-frame-pointer -o optimized_version optimized_version.cpp -Wall

# ── Step 3: perf stat (numbers) ────────────────────────────────────────────ptimized ──────────────────────
echo ""
echo "[*] Recording perf data for not_optimized_version..."
perf record -e cpu-clock -g -F 99 -o perf_not_opt.data ./not_optimized_version
perf script -i perf_not_opt.data | ./stackcollapse-perf.pl --inline | ./flamegraph.pl --title="Not Optimized" > flame_not_optimized.svg
echo "[*] Flame graph saved: flame_not_optimized.svg"

# ── Step 5: perf record + flame graph for optimized ──────────────────────────
echo ""
echo "[*] Recording perf data for optimized_version..."
perf record -e cpu-clock -g -F 99 -o perf_opt.data ./optimized_version
perf script -i perf_opt.data | ./stackcollapse-perf.pl --inline | ./flamegraph.pl --title="Optimized" > flame_optimized.svg
echo "[*] Flame graph saved: flame_optimized.svg"

echo ""
echo "========== DONE =========="
echo "Open flame_not_optimized.svg and flame_optimized.svg in a browser to view the flame graphs."
