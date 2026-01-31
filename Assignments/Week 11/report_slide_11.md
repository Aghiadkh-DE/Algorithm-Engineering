## Report on slide 11: SSD Parallelism

### Overview

Solid-state drives (SSDs) attain high throughput by exploiting parallelism inside the flash subsystem, because
individual NAND operations exhibit high intrinsic latency. The internal hierarchy (channels, packages, dies, planes)
provides multiple concurrency dimensions. The flash translation layer (FTL) schedules requests across these dimensions
using mechanisms commonly described as **channel striping**, **flash-chip pipelining (way interleaving)**, **die
interleaving**, and **plane sharing**. This report explains these mechanisms in architectural terms and analyzes their
performance consequences. It then derives application-facing implications and proposes a measurement workflow for
quantifying and exploiting SSD parallelism.

### 1. Background: Latency Hiding as the Core Objective

NAND flash operations (read, program, erase) have latencies that are large compared with host-side CPU cycles and
interface transaction times. Consequently, a single outstanding I/O request often leaves significant internal resources
idle. SSD controllers therefore aim to keep multiple flash units busy concurrently. Parallel execution increases
effective bandwidth and amortizes long-latency flash operations by overlapping them with independent work.

The key systems premise is that SSD throughput depends on maintaining sufficient concurrency in the device’s internal
pipeline. In practice, this means that host-visible performance often scales with the number of independent operations
that can be scheduled simultaneously and with how well those operations map onto distinct internal resources.

### 2. Flash Organization Relevant to Parallelism

A typical SSD organizes NAND flash into a hierarchy:

- **Channels**: independent controller interfaces that can operate concurrently.
- **Packages (chips) per channel**: multiple NAND packages share a channel; the controller can interleave access to
  them.
- **Dies per package**: independent silicon dies within a package; die-level independence enables interleaving within a
  package.
- **Planes per die**: subarrays within a die that can sometimes execute operations in parallel or as multi-plane
  commands.

This hierarchy matters because each level has distinct constraints and arbitration points. A channel provides the
strongest form of independence, while planes offer finer-grained parallelism but are subject to stricter command and
alignment constraints.

### 3. The Four Parallelism Mechanisms

#### 3.1 Channel Striping

**Channel striping** distributes requests across multiple channels. Because channels can operate independently, striping
increases aggregate bandwidth when the workload supplies enough outstanding operations.

Mechanistically, striping resembles a wide memory system: the controller selects a channel for each operation, aiming to
balance load and avoid collisions. For large sequential transfers, controllers commonly stripe contiguous logical data
across channels to raise bandwidth. For small random I/O, striping is effective only if the host issues multiple
independent requests so that the controller can populate different channels concurrently.

#### 3.2 Flash-Chip Pipelining (Way Interleaving)

**Flash-chip pipelining** (often described as **way interleaving**) exploits multiple NAND packages connected to the
same channel. Even though the channel is shared, the controller can pipeline operations by alternating commands across
packages. While one package is internally busy with a long-latency NAND operation, the channel can be used to dispatch
commands to another package.

This mechanism increases utilization of the shared bus and command path. Its effectiveness depends on the controller’s
ability to overlap command issuance and completion handling across ways, and on the workload providing sufficient
parallel requests to keep multiple packages active.

#### 3.3 Die Interleaving

**Die interleaving** assigns concurrent requests to different dies within the same package. Dies are more independent
than planes because they have distinct internal circuits; operations can progress in parallel, subject to package-level
and channel-level arbitration.

Die interleaving is particularly relevant for workloads that generate many small I/Os. The controller can scatter these
operations across dies to hide per-die latency. However, die-level parallelism can be limited by metadata updates,
error-correction processing, and shared controller resources.

#### 3.4 Plane Sharing

**Plane sharing** (plane-level parallelism) assigns requests to different planes within a die, sometimes via multi-plane
command modes. Plane-level concurrency can increase throughput and reduce latency variance when operations can be paired
or coordinated.

Plane sharing is constrained: multi-plane operations typically require specific address relationships (for example,
alignment to corresponding blocks or pages across planes) and compatible command types. As a result, plane-level
parallelism is often more sensitive to data placement and request structure than channel or die parallelism. Controllers
may also prefer plane-level techniques for internal activities such as garbage collection moves, which can compete with
foreground I/O.

### 4. FTL Scheduling: How Parallelism Meets Real Workloads

The FTL maps host logical block addresses to physical flash locations, enabling out-of-place updates, garbage
collection, and wear leveling. Parallelism mechanisms are not independent of the FTL; they are implemented through FTL
placement and scheduling decisions.

Several interactions determine whether the four parallelism mechanisms translate into host-visible performance:

1. **Request independence and queue depth**: the controller needs multiple in-flight operations to populate channels,
   ways, dies, and planes.
2. **Mapping and placement locality**: aggressive striping can raise bandwidth, but poor placement can increase write
   amplification and garbage collection pressure.
3. **Background work interference**: garbage collection and wear leveling consume the same internal resources (channels,
   dies, planes), potentially reducing effective parallelism for foreground traffic.
4. **Serialization points**: metadata updates (mapping tables), write journaling inside the controller, and
   error-correction pipelines can introduce bottlenecks that cap scaling even when flash resources remain available.

The net effect is that SSD parallelism is workload-dependent. Two workloads with identical average bandwidth demands can
differ dramatically in observed throughput and tail latency depending on concurrency, locality, and write intensity.

### 5. Application-Facing Implications

#### 5.1 Concurrency as a First-Class Performance Parameter

Because internal parallelism is exposed through the number of in-flight operations, application and system designs that
maintain higher I/O concurrency typically achieve higher throughput. Concurrency can be supplied via multiple threads,
asynchronous I/O, batched submission, or any architecture that keeps an active pipeline of independent requests.

For NVMe devices, the host interface itself is designed for parallel submission and completion processing via multiple
queues. This reduces contention in multi-core systems and allows higher queue depths with lower CPU overhead, compared
with legacy single-queue designs.

#### 5.2 Granularity and Alignment Effects

Although parallelism is primarily about concurrency, request structure still matters. Requests aligned to physical and
file-system blocks avoid split transactions and reduce internal work. Larger sequential transfers are likely to be
striped across channels and ways efficiently, while small random I/O depends more heavily on concurrency to activate
multiple flash units.

#### 5.3 Synchronization Barriers Reduce Effective Parallelism

Synchronous durability barriers (for example, frequent fsync-style persistence points) tend to collapse concurrency by
forcing dependent ordering on the I/O stream. This reduces the controller’s ability to schedule requests across
independent internal units and can amplify latency.

#### 5.4 Parallelism Does Not Eliminate Write Amplification

Parallelism improves throughput by overlapping operations, but it does not remove fundamental flash constraints.
Workloads dominated by fine-grained random updates can still trigger heavy garbage collection and internal data
movement. In such cases, higher concurrency may increase throughput temporarily while also increasing internal pressure,
potentially worsening tail latency under sustained load.

### 6. Measurement Workflow for SSD Parallelism

A performance engineering workflow for characterizing SSD parallelism aims to measure scaling behavior and identify
saturation points.

1. **Microbenchmark design**: vary queue depth and the number of concurrent workers while controlling I/O size, access
   pattern (random vs sequential), and read-write mix.
2. **Scaling curves**: plot throughput and latency percentiles as a function of concurrency. The slope and saturation
   point indicate how much internal parallelism is being activated.
3. **CPU overhead attribution**: measure CPU time in the I/O stack (system calls, interrupts, polling) to distinguish
   device saturation from host-side bottlenecks.
4. **Device-side indicators**: monitor device busy time, internal error correction activity (when available), and
   thermal throttling indicators, since these can cap apparent scaling.

The core diagnostic objective is to separate three regimes: underdriven (insufficient concurrency), scaling (parallel
units being filled), and saturated (a resource bottleneck dominates, often the flash subsystem, controller pipeline, or
host I/O path).

### 7. Limitations and Practical Boundaries

Internal parallelism is real but bounded.

- **Finite channels and ways** cap maximum concurrency benefits.
- **Shared resources** (controller compute, DRAM for mapping, ECC engines) can become the bottleneck before flash
  parallel units are fully utilized.
- **Plane-level constraints** limit how often plane sharing can be exploited for arbitrary workloads.
- **Background tasks** can consume parallel units unpredictably, especially at high utilization or under sustained
  writes.

These factors imply that optimal concurrency is device- and workload-specific and is best treated as an experimentally
determined parameter.

### Conclusion

Slide 11’s four mechanisms describe how SSDs convert a high-latency flash medium into a high-throughput device by
exploiting parallel resources across channels, packages, dies, and planes. Channel striping and way interleaving provide
coarse-grained concurrency, die interleaving extends this within packages, and plane sharing offers finer-grained
opportunities subject to strict constraints. Host-visible performance depends on whether application and system behavior
supplies enough independent in-flight operations for the controller to schedule effectively, while avoiding workload
patterns that trigger excessive internal data movement. A measurement-driven approach, varying concurrency and observing
saturation behavior, provides the most reliable basis for exploiting SSD parallelism in real systems.