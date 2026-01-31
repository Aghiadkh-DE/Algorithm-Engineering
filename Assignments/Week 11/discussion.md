### 1. Concurrency as an explicit requirement to realize SSD internal parallelism

A central observation is that SSD throughput is not solely a device property but a joint function of host-side request
concurrency and the SSD’s internal parallel structure (channel-, package-, chip-, and plane-level parallelism). The
paper’s motivation example shows that simply replacing HDD with SSD yields a large gain, yet additional
application-layer changes can unlock another multiplicative improvement: throughput increases from roughly 20K qps
under naive SSD adoption to about 100K qps after making the design SSD-friendly, with the stated "secret source"
being multiple concurrent I/O threads. This is an architectural point rather than a tuning anecdote: without enough
in-flight requests, the controller cannot schedule work across its parallel units, leaving capacity idle.

The implication for algorithm engineering is that "I/O complexity" must be augmented with a concurrency model. For
random, small requests, maximizing throughput typically requires a sufficient queue depth and independence of
operations so that the controller can distribute them across channels and packages. This reframes performance work from
reduce I/O count to structure I/O so that the device can pipeline it, which is a different optimization objective.

### 2. The non-monotonic interaction between thread count and I/O size

A second notable point is the explicit distinction between small-I/O and large-I/O regimes, and the empirical
claim that the optimal thread count can reverse between them. For small I/O, the paper argues that a single thread
cannot saturate internal parallelism and demonstrates increasing aggregate throughput as thread count grows (e.g., 10KB
writes scaling up to approximately 500MB/s at 8 threads). In contrast, for large I/O, the paper reports that a few
threads can already exploit internal parallelism, while additional threads can degrade throughput, in the provided
example with 10MB writes, throughput rises from about 414MB/s (1 thread) to 816MB/s (2 threads) but then
drops to roughly 500MB/s at 8 threads.

Mechanistically, this is interesting because it attributes the decline not to the flash medium itself but to 
contention and interference effects. A competition on shared internal resources (notably a lock-protected mapping
table) and interference between background activities such as OS readahead and write-back. This is a concrete example of
a common performance-engineering pattern like adding parallelism increases pressure on shared critical sections and
secondary subsystems, producing a throughput peak followed by regression. The practical consequence is that
SSD-parallelism optimization is not more threads, but enough concurrency to fill the device, without crossing the
contention threshold, which must be determined empirically per workload and device.
