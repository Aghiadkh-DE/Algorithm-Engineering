### 1. Cache optimization as a numerical analysis problem, not only a performance problem

A recurring and easily underestimated theme is that cache-focused loop and blocking transformations are not purely
mechanical performance tweaks. Even when transformations preserve program dependencies and the mathematical algorithm
remains unchanged, finite-precision arithmetic can make numerically equivalent expressions produce different
floating-point results when compilers exploit the new structure to reorder operations. This turns certain locality
transformations into a question of numerical reproducibility and stability, especially for long reductions and
factorization routines where rounding error can accumulate.

The same tension appears again when the paper discusses blocked factorizations. Blocking is adopted to route work through
high-reuse Level 3 kernels, but once pivoting enters the picture, block algorithms can choose different pivots than
their unblocked counterparts because pivot search may be constrained to the active block. This can alter round-off
behavior even when both methods are mathematically correct. The broader implication is that cache-aware algorithm
engineering often has to balance locality improvements against numerical properties such as stability, robustness, and
reproducibility.

### 2. Conflict behavior can negate blocking, motivating data layout interventions

The paper highlights an important limitation of the standard narrative that blocking automatically improves cache
behavior. Blocked codes can suffer substantial conflict misses due to self-interference: regular access patterns within
a block can map multiple frequently reused words onto the same cache sets or lines, effectively shrinking the usable
working set inside the cache. In such cases, the blocked algorithm may not realize its intended reuse, even when the
nominal block size is compatible with cache capacity.

The proposed remedy is conceptually simple but technically revealing: copy a non-contiguous block into a contiguous
temporary buffer so that its elements distribute more evenly across cache locations and can be reused without systematic
eviction. This is a strong example of data movement being deliberately introduced to reduce a larger amount of
unintended data movement. The technique also exposes a practical tradeoff. Copying adds overhead and extra memory
traffic, so it is only beneficial when conflict-induced miss costs dominate. This framing motivates a more general
viewpoint where cache optimization is a coupled problem of access order, data layout, and mapping constraints, rather
than capacity alone.