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