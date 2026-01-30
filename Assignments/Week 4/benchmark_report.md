# Benchmark Report: min_max_quicksort vs std::sort vs __gnu_parallel::sort

## Benchmarking Environment
- **Operating System:** Ubuntu 24.04.3 LTS (WSL2), kernel 6.6.87.1-microsoft-standard-WSL2
- **CPU:** Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz (8 cores / 8 threads)
- **RAM:** 15 GiB
- **Compiler:** g++ 13.3.0 (Ubuntu)

## Observed Patterns and Trends

### Speedup vs Threads (n ≥ 10,000,000)
- **Both parallel algorithms remain below the std::sort baseline** (speedup < 1.0) across most thread counts.
- **__gnu_parallel::sort scales better than min_max_quicksort**, improving up to mid/high thread counts before flattening or dipping.
- **min_max_quicksort is non‑monotonic** at higher thread counts (gains up to a point, then regressions).

**Possible reasons**
- **Parallel overhead dominates** at this problem size on this machine: task creation, synchronization, and scheduling costs can outweigh parallel gains versus an optimized single‑threaded std::sort.
- **Work imbalance in min_max_quicksort:** only one recursive branch is spawned as a task per split, which can leave threads underutilized and amplify imbalance from uneven partitions.
- **Memory bandwidth limits** cap scaling at higher thread counts, causing diminishing returns or regressions.
- **WSL2 scheduling jitter / frequency scaling** can introduce variability at higher thread counts.

### Speedup vs Array Size (all threads)
- **Smaller sizes are slower than std::sort** due to parallel overheads and fixed setup costs.
- **As size increases, __gnu_parallel::sort approaches or slightly exceeds std::sort**, showing better amortization of overhead.
- **min_max_quicksort stays below std::sort** even at larger sizes in this setup.

**Possible reasons**
- **Amortization effect:** larger arrays provide enough work to offset parallel setup costs.
- **Algorithm implementation quality:** __gnu_parallel::sort is heavily optimized and balanced; min_max_quicksort has higher overhead and less consistent parallelism.
- **Cache and bandwidth effects:** large arrays become bandwidth‑bound, limiting overall speedup for both parallel methods.

## Files
- CSV results: `Assignments/Week 4/benchmark_results.csv`
- Graphs: `Assignments/Week 4/speedup_vs_threads.svg`, `Assignments/Week 4/speedup_vs_size.svg`
