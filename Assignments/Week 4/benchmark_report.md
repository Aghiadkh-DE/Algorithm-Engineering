# Benchmark Report

## Benchmarking Environment

- **Operating System:** Ubuntu 24.04.3 LTS
- **CPU:** Intel(R) Core(TM) i7-9700K @ 3.60GHz, 8 cores / 8 threads (no SMT).
- **RAM:** 16,333,044 kB total (~15.6 GiB) 3200 MHz DDR4.
- **Compiler:** `g++` 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04).

## Build Configuration and Compiler Flags

- **Build type:** Release (`cmake-build-release`).
- **C++ standard:** C++23.
- **Effective flags (Release, Week 4):** `-O3 -march=native -mtune=native -DNDEBUG -Wall -Wextra -pedantic -fopenmp`.

*Notes:* Flags are derived from `Assignments/Week 4/CMakeLists.txt` and the release compile command recorded in
`cmake-build-release/compile_commands.json`.

## Patterns and Trends in the Graphs

### 1. Thread Scaling Benchmark (Speedup vs. Threads)

The thread scaling benchmark tested all three sorting algorithms with 1 to 25 threads on an array of 10 million elements.

**Single-threaded baseline behavior:** The `std::sort` algorithm maintains a constant runtime of approximately 0.63-0.67 seconds regardless of thread count, which confirms it does not utilize parallelism. This provides our baseline for calculating speedup.

**Initial scaling phase (1-5 threads):** Both parallel algorithms demonstrate strong scaling in this range. The `min_max_quicksort` achieves roughly 4.5x speedup by 5 threads, dropping from 0.87s to 0.19s. Similarly, `gnu_parallel` reaches about 4.3x speedup, improving from 0.69s to 0.15s. This near-linear scaling indicates that the algorithms effectively distribute work across available cores with minimal overhead.

**Optimal performance region (5-10 threads):** Both algorithms reach peak performance around 7-8 threads, which aligns with the 8 physical cores of the i7-9700K CPU. The `min_max_quicksort` achieves its best time of 0.159s at 7 threads (approximately 5.5x speedup over single-threaded `min_max`). The `gnu_parallel` sort performs best around 8-10 threads at 0.118s (approximately 5.8x speedup over its single-threaded version, and 5.5x faster than `std::sort`).

**Beyond physical core count (10-25 threads):** Performance plateaus and becomes inconsistent once thread count exceeds the available physical cores. The `min_max_quicksort` fluctuates between 0.16s and 0.22s with no clear improvement trend. Interestingly, `gnu_parallel` shows occasional improvements up to 18-21 threads, achieving times as low as 0.106s, before degrading at higher thread counts. This suggests `gnu_parallel`'s work-stealing scheduler can sometimes benefit from oversubscription.

**Notable anomalies:** Thread count 25 shows degraded performance for both parallel algorithms (0.189s for `min_max`, 0.187s for `gnu_parallel`), likely due to excessive context switching overhead. The performance dip at 23-25 threads is particularly pronounced for `gnu_parallel`, which loses much of its advantage over `min_max`.

**Reasons for these patterns:**
- Amdahl's Law limits speedup because sequential portions (memory allocation, final merges) cannot be parallelized.
- Memory bandwidth saturation occurs once all 8 cores are active, as the memory bus becomes the bottleneck rather than CPU compute.
- Context switching overhead increases when threads exceed cores, as the OS must frequently swap threads on and off cores.
- Cache thrashing worsens with more threads competing for the shared L3 cache.
- The `gnu_parallel` implementation uses work-stealing, which allows idle threads to take work from busy ones, explaining its slight advantage at moderate oversubscription levels.

---

### 2. Size Scaling Benchmark (Speedup vs. Array Size)

The size scaling benchmark compared the three algorithms across array sizes from 10 million to 100 million elements, using 8 threads for the parallel algorithms.

**Scaling behavior:** All three algorithms maintain O(n log n) time complexity as expected. The `std::sort` scales from 0.70s at 10M elements to 7.78s at 100M elements (approximately 11x increase for 10x more data, consistent with the logarithmic factor). Both parallel algorithms show similar scaling patterns.

**Parallel efficiency at different sizes:** The speedup of parallel algorithms over `std::sort` remains relatively consistent across all sizes. At 10M elements, `min_max` achieves 4.1x speedup and `gnu_parallel` achieves 4.7x speedup. At 100M elements, `min_max` achieves 3.9x speedup and `gnu_parallel` achieves 4.7x speedup. This consistency indicates good strong scaling properties.

**Comparison between parallel algorithms:** The `gnu_parallel` sort consistently outperforms `min_max_quicksort` by 15-35% across all sizes. The gap is smallest at 40-50M elements where both algorithms show similar performance (0.89s vs 0.74s at 50M), suggesting this range may hit a memory bandwidth ceiling that affects both equally.

**Performance anomaly at 40-50M range:** Both parallel algorithms show a slight efficiency dip in the 40-50M element range. The `min_max` algorithm takes 0.887s for 40M but only 0.891s for 50M, which is unexpectedly close. This could indicate L3 cache boundary effects or memory allocation patterns at these sizes.

**Reasons for these patterns:**
- Larger arrays amortize fixed overhead costs (thread creation, task scheduling) over more useful work, maintaining efficiency.
- Memory bandwidth becomes the dominant factor at larger sizes, which explains why the speedup ratio stabilizes rather than improving.
- The `gnu_parallel` advantage comes from its optimized pivot selection using sampling and its cache-aware block partitioning.
- Both algorithms benefit from hardware prefetching on the sequential memory access patterns inherent to sorting.

---

### 3. Algorithm-Specific Observations

**`min_max_quicksort` characteristics:**
The algorithm uses a clever overflow-safe average calculation for pivot selection and tracks minimum/maximum values during partitioning to compute tighter pivot bounds for recursive calls. It falls back to insertion sort for small subarrays under 32 elements and uses OpenMP tasks with a `final` clause at 10,000 elements to limit task creation overhead. The tracking of min/max during partition adds slight overhead but improves pivot quality, reducing the chance of worst-case partitioning.

**Why `gnu_parallel` outperforms `min_max`:**
The GNU parallel sort benefits from years of optimization for common architectures. It uses sampling-based pivot selection that examines multiple elements to find better partition points. Its work-stealing scheduler dynamically rebalances load when partitions are uneven. The implementation also uses cache-aware block sizes and may leverage SIMD vectorization for element comparisons and swaps. These optimizations compound to give it a consistent 15-35% advantage.

---

### 4. Summary of Findings

The benchmarks reveal that parallel sorting provides substantial speedup on multi-core systems, but with important practical limits. Optimal thread count matches physical core count, with diminishing or negative returns beyond that point. Both parallel algorithms achieve 4-5x speedup over single-threaded sorting on 8 cores, representing 50-60% parallel efficiency. The `gnu_parallel` implementation consistently outperforms the custom `min_max_quicksort` due to its mature optimizations, but both are viable for parallel sorting workloads. Memory bandwidth rather than compute power becomes the limiting factor for sorting large arrays, which explains why speedup plateaus rather than continuing to improve with more threads.

