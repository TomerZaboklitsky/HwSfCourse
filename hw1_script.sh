#!/bin/bash

set -e

echo "============================================================"
echo "  HW1 - Benchmark: Linear Scan vs Hash Index Lookup"
echo "============================================================"

# Step 1: Compile
echo ""
echo "# Step 1 - Compilation"
echo "------------------------------------------------------------"

echo "[build] Compiling not_optimized_version..."
g++ -std=c++17 -O0 -g -fno-omit-frame-pointer -o not_optimized_version not_optimized_version.cpp -Wall

echo "[build] Compiling optimized_version..."
g++ -std=c++17 -O0 -g -fno-omit-frame-pointer -o optimized_version optimized_version.cpp -Wall

echo "[build] Done."

# Step 2: Plain runs
echo ""
echo "# Step 2 - Plain Runs"
echo "------------------------------------------------------------"

echo "[run] not_optimized_version:"
./not_optimized_version
echo ""
echo "[run] optimized_version:"
./optimized_version
echo ""

# Step 3: perf stat Pass 1 - cycles + instructions (IPC)
echo ""
echo "# Step 3 - perf stat Pass 1: cycles + instructions (IPC)"
echo "------------------------------------------------------------"

echo "[stat] not_optimized_version:"
perf stat -e cycles,instructions ./not_optimized_version
echo ""
echo "[stat] optimized_version:"
perf stat -e cycles,instructions ./optimized_version
echo ""

# Step 4: perf stat Pass 2 - cache + branches + task-clock
echo ""
echo "# Step 4 - perf stat Pass 2: cache, branches, task-clock"
echo "------------------------------------------------------------"

echo "[stat] not_optimized_version:"
perf stat -e cache-references,cache-misses,branches,branch-misses,task-clock,context-switches ./not_optimized_version
echo ""
echo "[stat] optimized_version:"
perf stat -e cache-references,cache-misses,branches,branch-misses,task-clock,context-switches ./optimized_version
echo ""

# Step 5: Flame graphs
echo ""
echo "# Step 5 - Flame Graphs"
echo "------------------------------------------------------------"

if [ ! -d "FlameGraph" ]; then
    echo "[flamegraph] Cloning FlameGraph toolkit..."
    git clone https://github.com/brendangregg/FlameGraph.git
else
    echo "[flamegraph] FlameGraph already present."
fi

echo ""
echo "[record] not_optimized_version -> perf_not_opt.data"
perf record -F 999 -e cpu-clock -g --call-graph fp -o perf_not_opt.data ./not_optimized_version

echo ""
echo "[record] optimized_version -> perf_opt.data"
perf record -F 999 -e cpu-clock -g --call-graph fp -o perf_opt.data ./optimized_version

echo ""
echo "[verify] sample counts:"
echo -n "  not_optimized_version: "
perf script -i perf_not_opt.data | wc -l
echo -n "  optimized_version:     "
perf script -i perf_opt.data | wc -l

echo ""
echo "[svg] flame_not_optimized.svg"
perf script -i perf_not_opt.data | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl --title="Linear Scan - Not Optimized" > flame_not_optimized.svg

echo ""
echo "[svg] flame_optimized.svg"
perf script -i perf_opt.data | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl --title="Hash Index - Optimized" > flame_optimized.svg

echo ""
echo "============================================================"
echo "  Done. Generated files:"
echo "    - flame_not_optimized.svg"
echo "    - flame_optimized.svg"
echo "============================================================"
