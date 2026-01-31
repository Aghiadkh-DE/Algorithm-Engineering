# Memory Bandwidth, Compute Bounds, and Locality Principles

## 1. Bandwidth-Bound versus Compute-Bound Computations

### Overview

The performance of a computation is typically constrained by one of two fundamental resources: the rate at which data
can be transferred between memory and the processor, or the rate at which the processor can perform arithmetic and
logical operations. Identifying which resource constitutes the bottleneck is essential for guiding optimization efforts
effectively.

### Compute-Bound Computations

A computation is classified as compute-bound when its execution time is primarily determined by the number of arithmetic
or logical operations that the processor must perform. In such cases, the processor's execution units are fully
utilized, and additional memory bandwidth would not accelerate the computation. Compute-bound workloads typically
exhibit high arithmetic intensity, defined as the ratio of floating point operations to bytes transferred from memory.

Examples of compute-bound computations include dense matrix multiplication, where each data element is reused many
times, and iterative numerical methods that perform extensive calculations on data already resident in cache. For these
workloads, performance improvements are achieved by increasing the computational throughput through techniques such as
vectorization, loop unrolling, and exploitation of instruction level parallelism.

### Bandwidth-Bound Computations

A computation is classified as bandwidth-bound when its execution time is primarily determined by the rate at which data
can be moved between main memory and the processor. In such cases, the processor's execution units remain underutilized
because they must wait for data to arrive from the memory subsystem. Bandwidth-bound workloads typically exhibit low
arithmetic intensity, performing relatively few operations per byte of data transferred.

Examples of bandwidth-bound computations include simple vector operations such as element-wise addition or scaling,
stream processing applications, and sparse matrix computations where data reuse is minimal. For these workloads,
performance improvements require reducing memory traffic through better data layout, increasing cache utilization, or
employing hardware with higher memory bandwidth.

### Distinguishing the Two Regimes

The roofline model provides a useful framework for characterizing whether a computation is bandwidth-bound or
compute-bound. This model plots attainable performance as a function of arithmetic intensity, revealing a transition
point where the constraint shifts from memory bandwidth to peak compute throughput. Computations with arithmetic
intensity below this threshold are bandwidth-bound, while those above are compute-bound. Understanding this distinction
allows practitioners to focus optimization efforts on the actual bottleneck rather than pursuing improvements that yield
negligible benefit.

## 2. Temporal Locality and Spatial Locality

### The Memory Hierarchy and Access Latency

Modern computer systems employ a hierarchical memory structure comprising registers, multiple levels of cache, main
memory, and secondary storage. Each level offers a tradeoff between capacity and access latency, with smaller and faster
memories positioned closer to the processor. Exploiting this hierarchy effectively requires that programs exhibit
predictable memory access patterns that allow frequently used data to reside in faster memory levels.

### Temporal Locality

Temporal locality refers to the tendency of a program to access the same memory locations repeatedly within a short
period of time. When a program exhibits strong temporal locality, data that has been recently accessed is likely to be
accessed again soon. This property enables the cache to retain useful data across multiple accesses, thereby avoiding
repeated transfers from slower memory levels.

Temporal locality improves performance because the cache can satisfy subsequent requests for the same data without
incurring the latency penalty of accessing main memory. Programs that iterate over fixed data structures, reuse
intermediate results, or employ memoization techniques benefit substantially from temporal locality. The effectiveness
of this locality depends on the cache size and replacement policy, as the relevant data must remain in cache between
accesses.

### Spatial Locality

Spatial locality refers to the tendency of a program to access memory locations that are close to one another in address
space. When a program exhibits strong spatial locality, accessing one memory location implies that nearby locations will
likely be accessed in the near future. This property enables the cache to prefetch contiguous blocks of data, amortizing
the cost of memory access across multiple useful elements.

Spatial locality improves performance because caches transfer data in fixed-size blocks called cache lines, typically
ranging from 32 to 128 bytes. When a program accesses one element within a cache line, the entire line is brought into
cache, making adjacent elements immediately available without additional memory transactions. Programs that traverse
arrays sequentially, access structure fields in order, or operate on contiguous memory regions benefit substantially
from spatial locality.

### Combined Effect on Performance

Programs that exhibit both temporal and spatial locality achieve the highest cache hit rates and consequently the best
performance. Temporal locality ensures that working sets remain in cache across repeated accesses, while spatial
locality ensures that each cache line transfer provides multiple useful data elements. Poor locality, by contrast,
results in frequent cache misses, forcing the processor to stall while data is retrieved from slower memory levels.

Optimizations that improve locality include loop tiling to enhance temporal reuse, data structure reorganization to
improve spatial access patterns, and algorithm redesign to minimize random memory accesses. These techniques reduce
effective memory latency and bandwidth consumption, thereby improving both bandwidth-bound and compute-bound workloads
by ensuring that data arrives at the processor when needed.

## Conclusion

Bandwidth-bound and compute-bound computations represent distinct performance regimes determined by whether memory
transfer rates or processor execution rates constitute the limiting factor. Temporal locality and spatial locality are
fundamental principles that improve performance by increasing cache hit rates and reducing the effective latency of
memory accesses. Understanding these concepts enables practitioners to diagnose performance bottlenecks accurately and
apply appropriate optimization strategies.
