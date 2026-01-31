## Latency and Throughput Metrics for Intrinsic Functions

When evaluating the performance characteristics of intrinsic functions, two fundamental metrics provide complementary
perspectives on execution behavior: latency and throughput. Understanding these metrics is essential for writing high
performance code that effectively utilizes modern processor capabilities.

### Latency

Latency refers to the total number of clock cycles required from the moment an instruction begins execution until its
result becomes available for use by dependent instructions. This metric captures the depth of the pipeline stages that
the instruction must traverse and reflects the time cost imposed on any computation that depends on the instruction's
output.

For intrinsic functions, latency determines the minimum time between issuing an instruction and consuming its result in
a subsequent operation. Instructions with high latency create longer dependency chains, which can limit the effective
instruction level parallelism when operations form sequential dependencies. For example, a division intrinsic typically
exhibits substantially higher latency than an addition intrinsic, meaning that code with tight dependencies on division
results will experience greater stalls.

### Throughput

Throughput measures the rate at which a processor can execute instances of a particular instruction, typically expressed
as cycles per instruction or, equivalently, instructions per cycle. This metric reflects how many execution units can
handle the instruction and how frequently those units can accept new operations.

A throughput of 0.5 cycles per instruction indicates that the processor can, on average, complete two such instructions
per cycle when sufficient independent work is available. Conversely, a throughput of 4 cycles per instruction means that
only one instruction of that type can complete every four cycles, regardless of how many independent instances are ready
for execution.

### Relationship Between Latency and Throughput

Latency and throughput are independent metrics that together characterize instruction performance. An instruction may
have high latency but also high throughput if the processor pipelines the operation across multiple execution units. In
such cases, many instructions can be in flight simultaneously, each at different pipeline stages, yielding high
aggregate throughput despite the delay for any single result.

Conversely, an instruction with low latency may still have limited throughput if only one execution unit can handle it.
The practical performance of a code sequence depends on whether the computation is latency bound, where critical path
dependencies dominate, or throughput bound, where the rate of independent operations saturates available execution
resources.
