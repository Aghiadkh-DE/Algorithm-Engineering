One thing I found particularly interesting is how the paper reframes "correct multithreading" as an ordering problem
created jointly by the compiler and the CPU, not just by "threads running at the same time." The introductory v /
v_ready example looks obviously safe until you remember that optimizing compilers can legally reorder loads and stores (
as long as single-thread behavior is preserved), and modern CPUs can also reorder and buffer memory operations, so other
cores may observe writes in a different order than the source code implies. What’s elegant is the paper’s "atomics as
rendezvous points" intuition: making the flag atomic is not about making the flag special, it is about forcing the
toolchain to respect a cross-thread ordering boundary around it. The follow-on discussion about memory orderings then
makes the performance and complexity trade-off feel concrete: sequential consistency is the default because it is
easiest to reason about, but on weakly ordered hardware (ARM is the example), it can mean extra barriers that suppress
useful optimizations, hence the motivation for acquire/release and relaxed operations when you are using known-safe
patterns.

A second thing that stands out is the way cache coherence turns "innocent" sharing into a performance hazard, even when
you are doing everything "correctly" from a race-freedom standpoint. The false sharing section highlights that the cache
line is the unit of coherence traffic: if two cores touch different variables that happen to sit on the same line, the
line can ping-pong between caches and dominate runtime. The readers–writer lock example is a great illustration because
it punctures a common assumption: "more concurrency" (many readers) should help, but if all readers hammer a shared
counter or flag in one cache line, the coherence traffic can cost more than the critical sections themselves. Worth noting
is that the paper doesn't offer a free fix, e.g.: padding atomics to a full cache line can work, but it is
explicitly a space and time trade-off, so you are forced to think like a systems engineer, not just a language user.