# Benchmark Report

## Benchmarking Environment

- **Operating System:** Ubuntu 24.04.3 LTS
- **CPU:** Intel(R) Core(TM) i7-9700K @ 3.60GHz, 8 cores / 8 threads (no SMT).
- **RAM:** 16,333,044 kB total (~15.6 GiB) 3200 MHz DDR4.
- **Compiler:** `g++` 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04).

## Build Configuration and Compiler Flags

- **Build type:** Release (`cmake-build-release`).
- **C++ standard (Week 4):** C++20.
- **Effective flags (Release, Week 4):** `-O3 -march=native -mtune=native -DNDEBUG -Wall -Wextra -pedantic -fopenmp`.

*Notes:* Flags are derived from `Assignments/Week 4/CMakeLists.txt` and the release compile command recorded in
`cmake-build-release/compile_commands.json`.

## Patterns and Trends in the Graphs

### Speedup vs Threads (n = 10,000,000)

- **`__gnu_parallel::sort` scales up, then saturates.** It crosses a 1.0× speedup at **~5 threads** and peaks around **~
  1.32× at 14 threads**, then flattens or declines beyond ~16–25 threads. This indicates diminishing returns likely from
  memory bandwidth limits, synchronization overheads, and scheduling costs.
- **`min_max_quicksort` is mostly slower than `std::sort`.** It stays below 1.0× for almost all thread counts, only
  slightly exceeding 1.0× around **10 threads (~1.02×)**. This suggests higher constant factors or limited/no effective
  parallelism.

### Speedup vs Array Size (threads: 1 for std::sort, 8 for parallel runs)

- **`min_max_quicksort` remains consistently below 1.0×** (roughly **0.72–0.84×**) across all sizes. The algorithm
  likely has higher overhead and does not amortize well with size under the tested configuration.
- **`__gnu_parallel::sort` hovers around 1.0× with mild wins at larger sizes.** It reaches **~1.19× at 40M** and **~
  1.15× at 75M**, while small sizes (10M–30M) stay at or below parity. This aligns with parallel overheads being
  amortized only when input sizes grow.

## Potential Reasons for These Findings

- **Parallel overhead vs. work size:** Thread creation, task partitioning, and merges add fixed costs that only pay off
  at larger `n`.
- **Memory bandwidth saturation:** At higher thread counts, speedups flatten as the algorithm becomes bandwidth-bound
  rather than compute-bound.
- **Algorithmic constants:** `min_max_quicksort` appears to have larger constant factors or less efficient partitioning,
  outweighing any theoretical gains.

## Numeric Speedup Tables

### Speedup vs Threads (n = 10,000,000)

| Threads | std::sort (s) | min_max (s) | min_max speedup | __gnu_parallel::sort (s) | __gnu_parallel speedup |
|--------:|--------------:|------------:|----------------:|-------------------------:|-----------------------:|
|       1 |         0.191 |       0.805 |           0.237 |                    0.623 |                  0.307 |
|       2 |         0.143 |       0.399 |           0.358 |                    0.324 |                  0.441 |
|       3 |         0.117 |       0.272 |           0.430 |                    0.221 |                  0.529 |
|       4 |         0.124 |       0.217 |           0.571 |                    0.187 |                  0.663 |
|       5 |         0.155 |       0.193 |           0.803 |                    0.147 |                  1.054 |
|       6 |         0.155 |       0.210 |           0.738 |                    0.151 |                  1.026 |
|       7 |         0.147 |       0.186 |           0.790 |                    0.138 |                  1.065 |
|       8 |         0.175 |       0.196 |           0.893 |                    0.160 |                  1.094 |
|       9 |         0.215 |       0.258 |           0.833 |                    0.194 |                  1.108 |
|      10 |         0.173 |       0.170 |           1.018 |                    0.134 |                  1.291 |
|      11 |         0.113 |       0.174 |           0.649 |                    0.127 |                  0.890 |
|      12 |         0.121 |       0.180 |           0.672 |                    0.131 |                  0.924 |
|      13 |         0.113 |       0.167 |           0.677 |                    0.121 |                  0.934 |
|      14 |         0.145 |       0.170 |           0.853 |                    0.110 |                  1.318 |
|      15 |         0.138 |       0.166 |           0.831 |                    0.115 |                  1.200 |
|      16 |         0.129 |       0.160 |           0.806 |                    0.143 |                  0.902 |
|      17 |         0.121 |       0.160 |           0.756 |                    0.127 |                  0.953 |
|      18 |         0.119 |       0.168 |           0.708 |                    0.123 |                  0.967 |
|      19 |         0.121 |       0.172 |           0.703 |                    0.123 |                  0.984 |
|      20 |         0.113 |       0.169 |           0.669 |                    0.125 |                  0.904 |
|      21 |         0.122 |       0.175 |           0.697 |                    0.115 |                  1.061 |
|      22 |         0.147 |       0.174 |           0.845 |                    0.112 |                  1.312 |
|      23 |         0.143 |       0.173 |           0.827 |                    0.119 |                  1.202 |
|      24 |         0.141 |       0.159 |           0.887 |                    0.137 |                  1.029 |
|      25 |         0.119 |       0.158 |           0.753 |                    0.135 |                  0.881 |

### Speedup vs Array Size (threads: 1 for `std::sort`, 8 for parallel runs)

|  Size (n) | std::sort (s) | min_max (s) | min_max speedup | __gnu_parallel::sort (s) | __gnu_parallel speedup |
|----------:|--------------:|------------:|----------------:|-------------------------:|-----------------------:|
|  10000000 |         0.121 |       0.164 |           0.738 |                    0.126 |                  0.960 |
|  20000000 |         0.254 |       0.351 |           0.724 |                    0.251 |                  1.012 |
|  30000000 |         0.444 |       0.566 |           0.784 |                    0.448 |                  0.991 |
|  40000000 |         0.600 |       0.768 |           0.781 |                    0.504 |                  1.190 |
|  50000000 |         0.675 |       0.871 |           0.775 |                    0.729 |                  0.926 |
|  75000000 |         1.208 |       1.431 |           0.844 |                    1.053 |                  1.147 |
| 100000000 |         1.430 |       1.900 |           0.753 |                    1.419 |                  1.008 |
