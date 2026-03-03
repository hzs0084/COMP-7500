# Operating Systems Exam -- 1 

This document summarizes questions, answers, and explanations from the exam 1

------------------------------------------------------------------------

## 1) MLFQ -- Number of Interruptions

### Question:
A process has a burst time of 40 seconds. The multilevel feedback queue
has quantums: 
- Q1 = 2
- Q2 = 7
- Q3 = 12
- Q4 = 17

How many interruptions occur?

### Solution:

Each time a time quantum expires, a timer interrupt occurs.

-   Q1: 40 − 2 = 38 → interrupt #1
-   Q2: 38 − 7 = 31 → interrupt #2
-   Q3: 31 − 12 = 19 → interrupt #3
-   Q4: 19 − 17 = 2 → interrupt #4
-   Final queue: runs remaining 2 seconds and finishes (no interrupt)

### Final Answer:

**4 interruptions**

------------------------------------------------------------------------

## 2) Which Queue Does the Process Finish In?

After Q4, 2 seconds remain → it moves to Q5 and finishes there.

### Final Answer:

**Queue 5**

------------------------------------------------------------------------

## 3) Hashed Page Table Questions

Hash function:
h(p) = p mod m

To solve: 1. Compute bucket using mod 2. Search chain 3. If not found →
page fault

Examples: - Page 82 (mod 5 = 2) → frame 6
- Page 79 (mod 5 = 4) → frame 35
- Wrong chain example → page 39 (39 mod 5 = 4 but placed in bucket 3)

------------------------------------------------------------------------

## 4) fork() Counting -- Processes

### Question

How many processes:

```c
pid_t pid;
pid = fork1();
if (pid == 0) { /* child process */
    fork2();
    thread_create(...);
    thread_create(...);
    thread_create(...);
}
fork3();
fork4();
```

### Process Counting:

Start: 1
fork1 → 2
fork2 (child only) → 3
fork3 (all 3) → 6
fork4 (all 6) → 12

### Final Answer:

**12 processes total**

------------------------------------------------------------------------

## 5) fork() Counting -- Threads

Only processes inside the if-block create threads.

-   P1 creates 3 threads
-   P2 creates 3 threads

Total threads created: 3 + 3 = **6 threads**

Important rule: When fork() happens, only the calling thread is
duplicated.

------------------------------------------------------------------------

## 6) EDF Scheduling (Preemptive)

Timeline: 0--4 P1
4--7 P2
7--9 P1
9--14 P3
14--18 P1
18--22 P4

### Finish Times:

P1 = 18
P2 = 7
P3 = 14
P4 = 22

### Turnaround Times (TAT = Finish − Arrival):

P1 = 18
P2 = 3
P3 = 5
P4 = 10

### Waiting Times (WT = TAT − Burst):

P1 = 8
P2 = 0
P3 = 0
P4 = 6

### Average Waiting Time:

(8 + 0 + 0 + 6) / 4 = **3.5**

### Missed Deadlines:

Only P4 misses its deadline → **1 process**

------------------------------------------------------------------------

## 7) True/False Concepts

-   Shared memory "far more common" than message passing → False
-   pthread_join waits for thread → True
-   IPC only for cooperation → False
-   Shared memory best for large data → True
-   Message passing requires shared memory → False
-   RR quantum should be small relative to context-switch → False
-   Starvation definition given → True
-   Preemptive priority guarantees hard real-time → False

------------------------------------------------------------------------

## 8) Multiprocessor Scheduling Goal

Evenly distribute workloads across multiple processors.

------------------------------------------------------------------------

## 9) RabbitMQ Exchange

Receives messages from producers and routes them to queues.

------------------------------------------------------------------------

## 10) Multilevel Queue Scheduling

Each queue has its own scheduling algorithm.

------------------------------------------------------------------------

## 11) Shared Memory Permissions

Only processes with proper ownership/permissions can access.

------------------------------------------------------------------------

## 12) Hadoop / YARN

JobTracker split into: 
- ResourceManager
- ApplicationMaster
- JobHistoryServer

Fault recovery uses: 
- Application States
- Cluster States

HDFS = Hadoop Distributed File System

## 13) Makefile

In a Makefile:

```
NAME = value
```

defines a **macro (variable)**.

`$(NAME)` expands to the value.

Here:
- `REMOVE` is defined.
- `$(REMOVE)` expands to `rm`.


### Final Answer

`REMOVE` is the macro.

## Makefile — What is "scheduler"?

### Question

```make
scheduler: scheduler.o utility.o interface.o
    gcc scheduler.o utility.o interface.o -o scheduler
```

Makefile syntax:

```
target: dependencies
        command
```

- `scheduler` = target
- `.o` files = dependencies
- `gcc` line = action/recipe

`scheduler` is a **target file** (the executable being built).

## 14) Forking 

### Question

What is printed by:

```c
int count = 3;

int main() {
    pid_t pid;

    pid = fork();
    if (pid == 0) { /* child process */
        count++;
        return 0;
    }
    else if (pid > 0) {
        printf("count = %d", count);
        return 0;
    }
}
```

------------------------------------------------------------------------

END OF EXAM
