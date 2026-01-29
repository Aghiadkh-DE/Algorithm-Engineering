Name: Aghiad Khertabeel,
Matrikel: 185359

## 1) The memory hierarchy and caches (performance is fundamentally about locality)

What stands out is how this section frames fast execution as a hierarchy management problem, not just a CPU speed problem. 
The text’s core model is that each storage layer buffers the next one down so the system can usually satisfy requests from a smaller, faster tier instead of paying the latency of DRAM/disk.

What makes this genuinely interesting is the implied contract between hardware design and software behavior:

- Hardware designers build multiple cache levels because the main memory is relatively slow and the processor–memory gap keeps growing.
- Software “earns” speed by arranging computation to reuse data and code in tight regions (spatial/temporal locality), which keeps the working set hot in cache.

The section also makes a bold, practical claim about how much leverage this gives programmers: cache-aware code can improve the performance of their programs by an order of magnitude. That’s a huge statement and it matches what you see in real workloads: 
two implementations with identical algorithms can differ massively if one streams through contiguous arrays and the other pointer-chases through scattered memory.

From an algorithm engineering perspective, this is where theory meets silicon. It changes how you think about efficient data structures and loops:

- Array-of-structs vs struct-of-arrays isn’t just style, it’s bandwidth and cache-line utilization.
- A loop nest isn’t just arithmetic count, it’s an access pattern, i.e., whether you walk memory sequentially or with a stride that thrashes caches.
- Even small refactors (reordering loops, blocking/tiling, reducing indirections) can outperform clever micro-optimizations because they reduce cache misses rather than shaving a few ALU cycles.

So the interesting part isn’t merely caches exist, but that the chapter quietly teaches a systems-oriented performance model: most speed comes from making memory behave like it’s fast by behaving locally.

---

## 2) OS abstractions as engineered “illusions” (processes and virtual memory)

The other compelling idea is the OS as an *abstraction engine*: it intentionally constructs simplified realities that applications can rely on. In Chapter 1.7, this shows up most sharply in virtual memory and processes: the OS gives each process the illusion that it has exclusive use even though the machine is shared.

Why that’s interesting is that these illusions aren’t just conceptual, they’re operationally enforced via mechanisms that shape everything from performance to security:

- **Processes**: your program behaves like it owns the CPU, but the OS interleaves execution using context switching (save/restore PC/registers/memory state). This explains why scheduling, interrupts, and system calls can affect latency and throughput even when your code hasn’t changed.
- **Virtual memory**: your program sees a clean virtual address space layout (code/data/heap/shared libs/stack/kernel region), while the OS and hardware cooperate to translate addresses and map pages to physical memory (and ultimately to disk when needed). This is both a safety boundary (isolation) and a performance story (page faults, TLB behavior, caching effects).
- **Files**: I/O becomes uniform (read/write byte streams), which is why the same program can work across wildly different devices and deployments.

The illusion viewpoint is powerful because it’s predictive: it helps you reason about bugs and behavior that otherwise look mysterious, e.g., why memory allocation patterns matter (heap growth), why copying large buffers can stall (paging/cache effects), why "works on my machine" can hinge on OS-level differences (VM layout, scheduling, file system semantics).

Taken together, to sum it up. Modern systems are usable because they hide complexity behind abstractions, but they’re debuggable and optimizable only if you understand what those abstractions are made of.
