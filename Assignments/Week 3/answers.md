### Listing 3.1

In Listing 3.1, every thread reached the `printf` almost immediately after the parallel region begins, and OpenMP does
not impose any ordering on when different threads execute that statement. The order being observed is therefore an emergent
side effect of timing, which is basically tiny differences in when the OS schedules each thread, when each thread
actually starts running, and when it manages to aquire the internal lock used to serialize access to `stdout`. Because 
those effects vary from run to run, the thread numbers appear in a seemingly random order.

### Listing 3.2

In Listing 3.2, each thread prints only after it finishes computing `fib(n + t)`. With the naive recursive Fibonacci 
implementation, the amount of work grows very rapidly as the argument increases, so threads with smaller `t` consistantly 
have much less work than threads with larger `t`. When it is run with one thread per logical core, threads tend to run 
without much time-slicing, so completion time is mostly determined by how much computation each thread has, not by scheduling 
noise/overhead. As a result, the threads finish and therefore print in a predictable and repeatable order (from smallest `t` to the largest), 
which is why the output tends to be the same across runs.

### Listing 3.10

The snippet can use the cores efficiently, but only under the right conditions, and the decisive factor is the problem size.
For large size, the work per generation is size×size independent cell updates, and with collapse(2) the iteration space 
becomes a big pool of size² iterations that can be distributed well across threads, so the cores are kept busy and the
loop has good parallelism. Each thread reads from the current plane and writes each output cell to a distinct location
in aux_plane, so there are no true data dependencies between iterations, and the parallelization is conceptually clean. 
However, efficiency drops noticeably when size is small or moderate because the parallel for is entered anew in every
generation inside the while loop, which means parallel-region overhead is paid and an implicit barrier is reached at the 
end of every generation if the per-generation computation is small, that overhead dominates and many cores spend a 
significant fraction of time on runtime management and synchronization rather than useful work. Even for large size, 
speedup is often limited because the update is typically memory-bandwidth bound, each cell update performs little arithmetic 
but touches multiple memory locations, neighbor reads plus one write, so adding more cores eventually stops helping once the
memory subsystem is saturated. Variable size matters also for cache behavior, with char-sized cells, many cells share a cache
line, and if scheduling splits rows or blocks such that different threads write different bytes that happen to lie on the same
cache line near chunk boundaries, false sharing can occur and reduce efficiency, this is usually less severe with coarse
static chunking but can worsen with fine-grained scheduling. Overall, for sufficiently large boards the code can scale
reasonably until it hits the memory bandwidth ceiling, but for small boards and or few cells per thread it does not use
the cores efficiently due to per-generation overhead and synchronization costs, with potential additional degradation 
from cache-line effects driven by the small element size.

### PI Monte Carlo Optimization using LCG 

The runtime of the old `pi_monte_carlo` is around ~0.3 seconds. The second variant `pi_monte_carlo_lcg` offers a 
significant optimization step up leading to a runtime of approximately ~0.05 seconds. This improvement is primarily due 
to the replacement of the standard library's random number generator. 

Both versions parallelize the same loop, so the main runtime difference comes from what happens inside
each iteration. The fast version uses a tiny custom LCG random generator that is likely inlined and consists of
a few inexpensive integer operations, so the loop body stays very light. The slow version uses `std::default_random_engine`
together with `std::uniform_real_distribution<double>`, which typically adds significantly more arithmetic and control 
flow to convert engine output into doubles, making each iteration much more expensive. The counter-merge also
differs: `reduction(+ : counter)` keeps per-thread counters and combines them efficiently at the end, while the second 
version adds each thread’s local result with an atomic, which adds some synchronization overhead, though it’s minor
since it happens only once per thread. Overall, most of the speedup is from faster RNG; reduction vs. atomic is
a smaller secondary effect.