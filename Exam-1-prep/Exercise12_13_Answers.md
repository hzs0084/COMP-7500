# Exercise 12 -- Multicore Challenges for Multithreaded Programming

Multicore systems introduce several challenges:

1.  **Synchronization & Data Races**\
    When multiple threads execute simultaneously on different cores,
    shared data may be accessed concurrently. This requires proper
    synchronization (locks, mutexes, atomics) to prevent incorrect
    behavior.

2.  **Contention & Scalability Limits**\
    As the number of cores increases, shared locks or global data
    structures can become bottlenecks. This limits scalability and
    reduces performance gains (Amdahl's Law).

3.  **Cache Coherence & False Sharing**\
    Each core has its own cache. When threads modify shared data, cache
    coherence traffic increases. False sharing occurs when different
    variables share the same cache line, causing unnecessary performance
    degradation.

------------------------------------------------------------------------

# Exercise 13 -- Observations from Process vs Thread Creation Cost Table

1.  **Process creation (`fork`) is significantly more expensive than
    thread creation (`pthread_create`).**\
    The real time for `fork()` is roughly 10--15× higher than
    `pthread_create()`, showing that processes are much heavier to
    create.

2.  **`fork()` is dominated by system (kernel) time.**\
    The system time for `fork()` is much higher than for
    `pthread_create()`, indicating that process creation requires
    substantial OS-level work (memory management, process control
    structures, etc.).

3.  **Thread creation is more consistent and scalable across
    platforms.**\
    The time for `pthread_create()` remains relatively stable across
    different hardware configurations, while `fork()` varies more
    significantly.


Exercise 13 — Observations (at least 3)

Process creation is ~10–15× more expensive than thread creation (real time).

16-core: 12.5 vs 1.2 ⇒ ~10.4×

8-core: 17.6 vs 1.4 ⇒ ~12.6×

fork() cost is dominated by kernel/system time, much more than pthread_create().

16-core: fork() sys 12.5 vs thread sys 1.3

8-core: fork() sys 15.7 vs thread sys 1.3
This indicates process creation requires much heavier OS work (process structures + address space mgmt).

Thread creation cost is relatively stable across platforms; process creation varies more.

pthread_create() real: 1.2 → 1.4 (small change)

fork() real: 12.5 → 17.6 (large change)

Even on faster/more-parallel hardware, fork() remains expensive compared to threads.
The ratio stays very large on both systems, meaning frequent forking is a bad fit for fine-grained parallelism.

Design implication: if you need many short-lived workers or frequent creation, prefer threads/thread pools over fork(), because fork overhead will dominate runtime.