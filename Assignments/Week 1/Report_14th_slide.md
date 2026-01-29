Name: Aghiad Khertabeel, 
Matrikel: 185359

# OpenMP Pragmas and the Execution Model

## 1. Introduction
OpenMP is a shared-memory parallel programming model that combines compiler directives (pragmas), a runtime library, and environment variables. Slide 14 of the lecture highlights a central idea: OpenMP is not executed “magically” at runtime. Instead, the compiler translates OpenMP pragmas into low-level code that interacts with an OpenMP runtime system.

This report explains how OpenMP pragmas are processed by the compiler and presents the complete OpenMP execution model, including thread creation, work-sharing, synchronization, and performance aspects.

This report is based on the 14th slide of the lecture.

---

## 2. OpenMP Pragmas

### 2.1 What Are Pragmas?
In C and C++, OpenMP is primarily expressed through pragmas, for example:

```c
#pragma omp parallel
{
    // parallel region
}
```

Pragmas are **compiler directives**, not library calls. They instruct the compiler to generate parallel code when OpenMP support is enabled.

To enable OpenMP in GCC or Clang, a program must be compiled with the following flag:

```bash
gcc -fopenmp program.c
```

The `-fopenmp` flag enables parsing of OpenMP constructs and automatically links the OpenMP runtime library (e.g., `libgomp` in GCC).

---

## 3. Compiler Processing of OpenMP Pragmas

OpenMP pragmas are semantic instructions to the compiler. From them, the compiler generates additional helper functions, calls into the OpenMP runtime and data structures that describe the data/scope context (shared/private/firstprivate/…) and scheduling.

Depending on the toolchain, the steps differ slightly:
- **GCC** (libgomp) generates calls such as `GOMP_parallel_start`, `GOMP_parallel_end`, `GOMP_loop_*`, `GOMP_barrier`.
- **Clang/LLVM** (libomp) typically uses the **Intel/LLVM KMP interface** and generates calls such as `__kmpc_fork_call`, `__kmpc_for_static_init`, `__kmpc_barrier`.

### 3.1 Parsing and Semantic Analysis
The compiler frontend (e.g., Clang or the GCC frontend) recognizes `#pragma omp ...` only when OpenMP is enabled (e.g., via `-fopenmp`). Then it:
- **Validates clauses** (e.g., `private(x)`, `reduction(+:sum)`, `schedule(dynamic)`),
- **Determines data attributes** (shared/private/firstprivate),
- Checks **variable scopes** and **default rules** (e.g., `default(none)`).

The result is an internal representation (AST/IR nodes) from which code is generated later.

### 3.2 Outlining and SPMD Structure
As already described, outlining is the central step: the code of the parallel region is moved into a separate function.

**Key details of this process:**
- The compiler generates an *outlined function* (e.g., `__omp_outlined`) that contains the body of the parallel region.
- A wrapper/launcher structure ensures that the runtime executes this function on a team of threads.
- Often a context object (struct) is generated that holds addresses of `shared` variables and copies of `firstprivate` variables.

Conceptual pattern:

```c
// generated context for variables from the enclosing scope
typedef struct {
  int *a;      // shared: passed as a pointer
  int b;       // firstprivate: passed by value
} __omp_ctx_t;

static void __omp_outlined(__omp_ctx_t *ctx) {
  // access shared via ctx->a, firstprivate via ctx->b
  work(*(ctx->a), ctx->b);
}

int main() {
  __omp_ctx_t ctx = { .a = &a, .b = b };
  __omp_runtime_fork(__omp_outlined, &ctx);
  __omp_runtime_join();
}
```

> Important note: The exact names and signatures vary, but the basic principle (outlined body + context + runtime fork/join) remains the same.

### 3.3 Lowering of Data-Sharing Attributes
OpenMP defines clear rules for variable visibility inside parallel regions. The compiler *lowers* these rules into concrete memory operations:

- **shared(x)**: All threads reference the *same memory location*. In practice, the **address** of `x` is passed to the outlined function.
- **private(x)**: Each thread gets its **own uninitialized instance**. The compiler typically creates a local variable in the outlined function (often in the thread’s stack frame).
- **firstprivate(x)**: Like `private`, but **initialized** with the value before entering the parallel region. This is usually implemented via a copy in the context object.
- **lastprivate(x)**: Additionally, the value from the “last” iteration/section is written back at the end of work-sharing constructs. This requires extra logic in the join/epilogue phase.
- **reduction(op: x)**: The compiler generates a **private reduction temporary** per thread and a **combining phase** at the end (often with runtime assistance or atomics/locks, depending on implementation and optimization).

Conceptual example for `reduction(+:sum)`:

```c
int sum = 0;
#pragma omp parallel reduction(+:sum)
{
  sum += f();
}
// roughly equivalent to:
// per thread: local_sum += f();
// at the end: sum = sum + local_sum (combined)
```

### 3.4 Lowering of Work-Sharing Constructs (omp for / sections / single)
Work-sharing constructs **do not create new threads**, but distribute work within an existing team. Compiler and runtime cooperate:

#### 3.4.1 `omp for`
The compiler transforms the loop so that the iteration space is split into **chunks** and assigned to threads.
- With **static scheduling**, much can be done deterministically by the compiler; the runtime only provides bounds (or the compiler computes them directly).
- With **dynamic/guided scheduling**, the runtime is more involved (threads fetch chunks at runtime).

Conceptual pattern:

```c
#pragma omp parallel
{
  #pragma omp for schedule(dynamic)
  for (int i=0; i<N; i++) {
    body(i);
  }
}
```

Is roughly transformed into:
- `parallel` fork (team creation)
- inside: runtime loop initialization (scheduling)
- thread-local loop over assigned chunks
- runtime loop finalization

#### 3.4.2 `omp sections`
The compiler generates IDs for sections and invokes runtime support to assign each section to a thread.

#### 3.4.3 `omp single`
A runtime mechanism selects exactly **one** thread (often via a “single” token). There is an optional implicit barrier depending on `nowait`.

### 3.5 Synchronization: Barriers, Critical, Atomic, Flush
OpenMP synchronization is implemented via runtime calls or special machine instructions:

- **Barrier**: usually a direct runtime call (e.g., `GOMP_barrier` or `__kmpc_barrier`).
- **critical**: typically implemented using a runtime lock/mutex, often via a named critical object.
- **atomic**: if possible, implemented as a true **atomic CPU operation** (e.g., `lock xadd` on x86), otherwise via locks.
- **flush**: enforces memory ordering/visibility; depending on architecture/compiler via memory fences or runtime assistance.

### 3.6 Implicit Barriers and `nowait`
Many OpenMP constructs have **implicit barriers** (e.g., at the end of `parallel`, `for`, `sections`, `single`).

The compiler must:
- **insert** these barriers to guarantee OpenMP semantics,
- and **remove** them when `nowait` is specified.

Example:

```c
#pragma omp for nowait
for (...) { ... }
// -> no implicit wait at the end
```

This has direct performance implications because barriers introduce waiting time.

### 3.7 Tasks: Code Generation for `omp task`
For tasks, the compiler typically generates:
- a task body function (similar to outlining),
- a task descriptor structure (containing function pointer, data environment, flags, etc.),
- a runtime call to **enqueue** the task.

Conceptually:

```c
#pragma omp task
{ work(x); }
```

Becomes roughly:
- create task object
- attach context (captured variables)
- `runtime_submit_task(task)`

The runtime then decides which thread executes the task and when (work-stealing/queues, implementation-dependent).

### 3.8 Optimizations and Code Generation Details
Compilers try to minimize OpenMP overhead:
- **Parallel region fusion**: merging adjacent parallel regions when correctness allows.
- **If-conversion for `if(parallel: expr)`**: if `expr` is false at compile time or runtime, execution is serial.
- **Inlining/outlining balance**: outlined functions may be further optimized or inlined when possible, while preserving semantics.
- **Vectorization + `simd`**: `#pragma omp simd` influences vectorization decisions; this is a separate path from thread-level parallelism.

---

## 4. The OpenMP Runtime System

The OpenMP runtime library is responsible for several key tasks:

- Creation and management of thread pools
- Formation of thread teams
- Scheduling of work for constructs such as `omp for`
- Implementation of synchronization mechanisms (barriers, locks, atomics)
- Management of tasks and task dependencies

The runtime reads configuration values from environment variables (e.g., `OMP_NUM_THREADS`) and from runtime API calls (e.g., `omp_set_num_threads`).

---

## 5. The OpenMP Execution Model

### 5.1 Fork–Join Model
OpenMP follows a **fork–join execution model**:

1. A single thread begins program execution (the *initial thread*).
2. Upon encountering a `parallel` construct, this thread *forks* a team of threads.
3. Each thread executes the same outlined function (SPMD model).
4. At the end of the parallel region, there is an **implicit barrier**.
5. The threads *join*, and only the original thread continues execution.

This behavior is defined by the OpenMP specification.

### 5.2 Thread Teams and Implicit Tasks
Each parallel region creates a *thread team*. For each thread in the team, the runtime creates an **implicit task** that executes the code of the parallel region.

---

## 6. Work-Sharing Constructs

Within a parallel region, OpenMP provides several work-sharing constructs:

- `#pragma omp for` – distributes loop iterations
- `#pragma omp sections` – assigns code blocks to individual threads
- `#pragma omp single` – executes code on only one thread

These constructs do not create new threads. Instead, they control how the existing threads of a team divide the work, usually via runtime scheduling mechanisms.

---

## 7. Runtime Control and Environment Variables

The behavior of OpenMP can be controlled dynamically:

- `OMP_NUM_THREADS` – specifies the number of threads per team
- `OMP_SCHEDULE` – controls the scheduling strategy of loops
- `omp_set_num_threads(int)` – runtime API for controlling the number of threads

These settings influence the execution of parallel regions without requiring recompilation.

---

## 8. Performance Aspects

### 8.1 Fork–Join Overhead
Creating and synchronizing thread teams introduces non-negligible overhead. Frequently entering and leaving parallel regions can significantly degrade performance.

### 8.2 Implicit Barriers
Each parallel region ends with an implicit barrier. While this guarantees correctness, it can limit scalability if threads frequently wait for each other.

### 8.3 Differences Between Runtime and Compiler Implementations
Different OpenMP implementations (e.g., GCC vs. LLVM) differ in scheduling strategies and barrier implementations, which can affect performance.

---

## 9. References

1. https://www.openmp.org/specifications/
2. https://gcc.gnu.org/onlinedocs/libgomp/
3. https://openmp.llvm.org/
4. https://gcc.gnu.org/onlinedocs/libgomp/Enabling-OpenMP.html
5. https://docs.linaroforge.com/
6. https://blog.rwth-aachen.de/hpc_import_20210107/attachments/35947076/36143199.pdf

