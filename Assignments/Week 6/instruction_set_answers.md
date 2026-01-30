### Characteristics of SSE, AVX/AVX2, and AVX-512

SSE (Streaming SIMD Extensions) introduced 128-bit vector registers (XMM) and common integer and floating‑point SIMD
operations, typically processing 4× `float` or 2× `double` values per vector. It has very broad hardware support across
x86 CPUs. AVX extends this to 256-bit registers (YMM) and adds three‑operand encoding, while AVX2 broadens support to
full integer SIMD and adds operations such as gathers. At 256 bits, AVX/AVX2 can process 8× `float` or 4× `double`
values per vector, offering higher throughput but sometimes with increased power/thermal costs. AVX‑512 expands to
512-bit registers (ZMM) and introduces mask registers (k‑registers) for predicated operations, which helps handle tail
elements without branches. It can process 16× `float` or 8× `double` per vector and provides a richer instruction set (
e.g., compress/expand and conflict detection), though some CPUs may downclock under heavy AVX‑512 workloads due to power
and thermal limits.

### How Memory Aliasing Affects Performance

Memory aliasing happens when different pointers might refer to the same memory region. If the compiler cannot prove that
pointers do not overlap, it must assume aliasing is possible, which often blocks vectorization and reordering of
loads/stores. This can force extra reloads, reduce register reuse, and limit instruction‑level parallelism. Using
clearer dataflow or qualifiers such as `restrict` (in C) can give the compiler more freedom to optimize.

### Advantages of Unit‑Stride (Stride‑1) Access

Unit‑stride (contiguous) access is faster because consecutive elements share cache lines, improving locality and
reducing cache misses. Hardware prefetchers are also most effective with predictable, sequential access, which helps
sustain higher memory bandwidth and reduces TLB pressure. With larger strides (e.g., stride‑8), many fetched cache lines
contain mostly unused data, leading to wasted bandwidth and higher miss rates.

### When to Prefer Structure of Arrays (SoA)

The structure of Arrays is preferable when you frequently operate on one or a few fields across many records. It aligns each
field contiguously in memory, which makes SIMD/vectorization easier, reduces wasted cache traffic, and improves
bandwidth efficiency. This layout is also beneficial for GPUs and other accelerators where coalesced access is
important, and for batch‑style processing that streams through single fields at a time.
