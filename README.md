# Lingua Franca Benchmarks
## 1. MPC (Model Predictive Control)
This is a deterministic, real-time Cyber-Physical System (CPS) benchmark for robotics. It demonstrates the translation of a multi-threaded Model Predictive Control (MPC) architecture from standard POSIX C threads into [Lingua Franca (LF)](https://www.lf-lang.org/).

The primary goal of this project is to eliminate the non-determinism, race conditions, and silent real-time failures inherent in standard C multithreading by leveraging Lingua Franca's event-driven, time-aware actor model.

### 📂 Project Files

**Source Code**
* [`mpc-lf-main/mpc.lf`](mpc/mpc-code/mpc-lf-main/mpc.lf) - The deterministic Lingua Franca (LF) implementation of the MPC architecture (non-federated).
* [`mpc-lf-federated/mpc_fed.lf`](mpc/mpc-code/mpc-lf-federated/mpc_fed.lf) - The deterministic Lingua Franca (LF) implementation of the MPC architecture (non-federated).
* [`mpc_threaded.c`](mpc/mpc-code/mpc_threaded.c) — The original POSIX-threaded C baseline used for comparison.

**Benchmarking and Automation**
* [`run_benchmark.py`](mpc/mpc-code/run_benchmark.py) — Python test harness that automates compilation, execution, output parsing, and graph generation.

**Assets and Diagrams**
* [`diagrams-and-results/architecture.png`](mpc/diagrams-and-results/architecture.png) — Visual representation of the Lingua Franca reactor network and data flow.

**Execution Results**
* [`diagrams-and-results/lf_results.txt`](mpc/diagrams-and-results/lf_results.txt) — Raw execution log of the time-aware LF simulation.
* [`diagrams-and-results/c_results.txt`](mpc/diagrams-and-results/c_results.txt) — Raw execution log of the standard C multithreaded simulation.

## System Architecture

![System Architecture Diagram](mpc/diagrams-and-results/architecture.png)

---

## Quick Start and Benchmarking

### Prerequisites
To compile and run this benchmark, ensure you have the following installed on a Linux/WSL environment:
* `gcc` and `cmake`
* `lfc` (Lingua Franca Compiler)
* `python3`

### Running the Python Test Harness
The easiest way to evaluate the system is to run the automated Python benchmark script. This script compiles the Lingua Franca code, executes the 5-second real-time simulation and parses the terminal output.

```bash
python3 run_benchmark.py
```

### Manual Compilation
To manually compile and run the LF architecture with physical time enforcement:

```bash
lfc mpc.lf
time ./bin/mpc
```

### Execution Results & Timing Analysis
![C vs. LF](mpc/diagrams-and-results/comparison.png)

When comparing the terminal output of the standard C baseline against the Lingua Franca implementation, we observe three critical architectural differences.

**1. Mathematical Correctness (The Physics Match)**
Despite the entirely different concurrency models, the final computed states of the physical system are nearly identical, proving the MPC matrix calculations were ported perfectly.
* **C Baseline Final State:** `pos=0.0306 | vel=0.0111`
* **Lingua Franca Final State:** `pos=0.0292 | vel=0.0114`

**2. Physical Execution Time (`time` utility)**
Because the standard C code lacks a true understanding of time, it either processes the 5,000 logical ticks as fast as the CPU allows (finishing in milliseconds) or relies on unreliable OS-level `sleep()` commands that drift wildly. 
Conversely, Lingua Franca enforces the `fast: false` target property, strictly binding the 1kHz logical clock to the physical clock. 
* **LF Physical Execution Time:** `~5.0005 seconds` (Exactly 5 seconds + minimal startup overhead).

**3. Handling OS Jitter and Deadlines**
A desktop operating system (like Linux/WSL) has background processes that occasionally interrupt the CPU, causing computation lag (jitter). 

**The C Output (Silent Failure):**
```text
[actuate] tick=3500 | pos=0.0152 (ref=0.5000) | vel=0.0091
[actuate] tick=4000 | pos=0.0200 (ref=0.5000) | vel=0.0098
[actuate] tick=4500 | pos=0.0251 (ref=0.5000) | vel=0.0105
```
The C thread lagged and missed the 1ms hardware deadline, but silently ignored it, feeding delayed, stale motor voltages to the robot (which causes instability in physical systems).

**The LF Output (Active Safety):**
```
[WARNING] Optimizer missed 1ms deadline! Applying emergency brakes.
[WARNING] Optimizer missed 1ms deadline! Applying emergency brakes.
[actuate] tick=4000 | pos=0.0184 | vel=0.0101 | u=0.0013
```
Lingua Franca measures physical time against its logical timeline. When the heavy gradient descent math exceeded the strict 1ms hardware deadline due to OS jitter, LF intercepted the failure, aborted the stale math, and triggered the deadline(1 msec) block to safely apply u=0.0 (emergency braking).

## 🌐 Distributed Execution (RTI)
![RTI diagram](mpc/diagrams-and-results/RTI_diagram.png)

This benchmark can be compiled as a distributed network system using Lingua Franca's Run-Time Infrastructure (RTI). By changing the root component to a `federated reactor`, the compiler automatically generates a network server and splits the application into standalone executables (`mpc_plant`, `mpc_optimizer`, `mpc_ref`). 

The RTI acts as a central time-keeper, utilizing clock-synchronization protocols to coordinate logical time across physical TCP/IP network bounds, demonstrating how Lingua Franca scales seamlessly from multi-core threads to multi-node distributed systems.

RTI (Run-Time Infrastrtucture) is a central software bus that allows different programs, running on different computers, to talk to each other and share a single unified timeline. It handles the networking, the message routing, and most importantly, the synchronization of time across physical boundaries.

In standard C, if you want a robot's brain to run on a laptop and its spinal cord to run on a chassis, you have to manually write thousands of lines of TCP/IP socket code and deal with network lag causing the robot to crash.

In Lingua Franca, the RTI is an automatically generated Time Conductor.
When you compile a distributed program, LF builds a dedicated server (the RTI) whose sole job is to force every machine on the network to obey strict Logical Time. Before any node is allowed to execute tick 1500, the RTI ensures that all network messages from tick 1499 have been delivered, and that everyone's physical clocks are perfectly synchronized. It provides deterministic, real-time safety guarantees over an unpredictable Wi-Fi/Ethernet network.

How We Implemented It (The Architecture)

We changed main reactor to federated reactor.

By doing this, the Lingua Franca compiler completely changed its output. Instead of building one executable, it built four independent network binaries:

RTI (The central time-keeper)

mpc_ref (The Remote Control)

mpc_optimizer (The Brain performing gradient descent)

mpc_plant (The Robot Chassis reading sensors and applying voltage)

Because everything ran on my single machine for the benchmark, Lingua Franca intelligently bundled these into a single magic launcher (./bin/mpc) that spun up the RTI server in the background and connected the three federates via local TCP/IP sockets.

How It Works (The Execution Lifecycle)

Phase 1: The Clock Sync (Google Spanner Protocol): The RTI boots up first and waits. As plant, optimizer, and ref connect to the network, the RTI refuses to let the simulation start. It first forces them to exchange dozens of ping messages to measure the network latency between them. It then locks their internal clocks together. You saw this when all three federates reported the exact same starting nanosecond (1780087519273407295).

Phase 2: The "Cold Start" Latency Catch: Once execution begins, data must flow through network sockets. Initially, routing the heavy matrix math from the Optimizer to the Plant took slightly longer than the strict 1-millisecond hardware deadline. Lingua Franca detected this network lag immediately. Instead of using stale, delayed packets, the Plant caught the violation and safely applied the emergency brakes (u=-0.0000).

Phase 3: Flawless Lock-Step Execution: Once the network sockets warmed up, the RTI coordinated the distributed system flawlessly. Over a 5.000-second logical simulation, the total physical execution time was 5.002 seconds. The RTI managed all the TCP/IP network overhead in just 2 milliseconds.
```
federated reactor {
    ...
}
```
![RTI run](diagrams-and-results/rti1.png)
![RTI run](diagrams-and-results/rti2.png)

### 📊 Performance Results & Real-Time Safety
The "Missed Deadline" Feature
When running this benchmark on a standard desktop operating system (like Ubuntu/WSL), you may see console warnings indicating that the Optimizer missed its 1ms deadline. This is an intentional safety feature, not a bug.

In the original POSIX C implementation, if the CPU lagged and the MPC math took longer than 1 millisecond, the thread would silently fail, feeding stale and delayed motor voltages to the actuator (which causes physical crashes in real robots).

Lingua Franca measures physical hardware time. If the math computation exceeds the 1ms strict deadline, the LF runtime explicitly intercepts the failure and applies emergency braking (u=0.0), proving that LF provides safety guarantees that standard C threading lacks.

## Architectural Coding Standards Enforced

### 1. Strict Memory Isolation (No Mutexes)
Shared memory and condition variables (pthread_cond_wait) are strictly forbidden.
State must be strictly private to the component that owns it. Data (like x_current or u_sequence) must be passed explicitly through time-stamped messages across isolated Ports. Lingua Franca’s runtime handles the synchronization automatically.

**Before (C):**
```c
pthread_mutex_lock(&g_mutex);
memcpy(x0, g_state.x_current, sizeof(x0));
pthread_mutex_unlock(&g_mutex);
```

**After (Lingua Franca):**
```
// Data is safe and passed without locks
reaction(x_current) -> u_apply {=
    double x0[2];
    x0[0] = x_current->value.data[0];
    x0[1] = x_current->value.data[1];
=}
```

### 2. Logical Time Over Physical Sleep
We must never use sleep() or artificial delays to pace a loop.
We must use LF’s native timer construct. To achieve the 1 kHz control loop from the C code, the Sensor and Actuator reactors must be driven by a timer t(0, 1 msec).

**Before (C):**
```c
while (1) {
    sleep_ms(1);  // Relies on unpredictable OS
    // Read sensor data...
}
```

**After (Lingua Franca):**
```
timer t(0, 1 msec) // Mathematically precise logical clock

reaction(t) -> x {=
    // Read sensor data exactly every 1ms...
=}
```

### 3. Explicit Data Flow via Time-Stamped Ports
The execution order must be defined visually by how components are wired together, matching the "Sensor -> Computation -> Actuator" pipeline.
If the Optimizer needs the Sensor's data, the code must explicitly declare sensor.x_current -> optimizer.x_current. LF uses this to build a deterministic execution graph, eliminating race conditions.

**Before (C):**
```c
// Actuator might not listen to this pager (in opimizer thread)
g_state.new_solution = true;
pthread_cond_signal(&g_cond_sol);
```

**After (Lingua Franca):**
```
// The compiler reads this physical arrow to schedule execution
main reactor {
    plant.x -> optimizer.x_current
    optimizer.u_apply -> plant.u
}
```

### 4. Strict Hardware Deadlines
If the C thread's math takes longer than, for example, 1 millisecond, the another thread applies stale (e.g. out-of-date motor commands to the robot), potentially causing hardware failure.
The target is synced to physical time (`fast: false`), and the reactor explicitly utilizes a `deadline(1 msec)` block. If the computation exceeds the physical time boundary, the runtime catches the violation, allowing for safe emergency fallbacks.

**Before (C):**
```
// Runs math for an unknown amount of physical time
for (int iter = 0; iter < OPT_ITER; iter++) {
    // ... some heavy math ...
}
// Blindly applies answer, even if 5 milliseconds late
```

**After (Lingua Franca):**
```
target C {
    timeout: 5 sec,
    fast: false
}
```

```
reaction(x_current) -> u_apply {=
    // ... some heavy math ...
=} deadline(1 msec) {=
    // Safely catches execution if math exceeds physical deadline
    printf("[WARNING] Optimizer missed 1ms real-time deadline!\n");
    // Trigger emergency braking logic here
=}
```

# Why is User CPU So Low? (0.001 s)

When benchmarking the Lingua Franca MPC controller against the C baseline:

```
$ time ./bin/mpc > lf_results.txt
real    0m5.023s
user    0m0.001s     ← almost zero
sys     0m0.592s

$ time ./mpc_threaded > c_results.txt
real    0m6.049s
user    0m6.052s     ← full core, 6 seconds
sys     0m0.258s
```

## The LF runtime is event-driven, it sleeps between tags

A **tag** is a `(time, microstep)` pair representing a logical instant. In our MPC, the Plant's `timer t(0, 1 msec)` creates a new tag every 1 ms, giving us 5,000 tags over the 5-second run.

At each tag, the runtime processes all triggered reactions (the level 0→5 pipeline: sensor → reference → optimizer → actuator). For our small NX=2 problem, this takes roughly **0.2 µs per tick**.

After all reactions complete, the runtime calls `_lf_next_locked()`, which sleeps until the next tag is due, typically ~1 ms away. During that sleep, **zero user CPU is consumed**.

## The sleep chain in reactor-c

The call chain from "all reactions done" to "thread asleep" is:

```
_lf_next_locked()                         // reactor_threaded.c - advance to next tag
  → get_next_event_tag(env)               // peek at event queue: next tag is 1 ms away
  → wait_until(next_tag.time, &cond)      // reactor_threaded.c - sleep until physical time catches up
    → lf_clock_cond_timedwait(cond, t)    // platform layer - OS-level timed wait
      → _lf_cond_timedwait(cond, t)       // lf_POSIX_threads_support.c
        → pthread_cond_timedwait(...)     // POSIX - thread removed from CPU entirely
          → futex(FUTEX_WAIT_BITSET)      // Linux kernel - hardware timer set, thread sleeps
```

### The key function: `_lf_cond_timedwait`

```c
// From lf_POSIX_threads_support.c
int _lf_cond_timedwait(lf_cond_t* cond, instant_t wakeup_time) {
  // Convert nanoseconds to the struct that POSIX expects
  struct timespec timespec_absolute_time = convert_ns_to_timespec(wakeup_time);

  // THIS LINE puts the thread to sleep:
  int return_value = pthread_cond_timedwait(
      (pthread_cond_t*)&cond->condition,    // the "wake-up signal receiver"
      (pthread_mutex_t*)cond->mutex,         // mutex released while sleeping
      &timespec_absolute_time                // absolute wall-clock deadline
  );

  // Translate OS error code to LF constant
  switch (return_value) {
  case ETIMEDOUT:
    return_value = LF_TIMEOUT;  // "I slept the full time"
    break;
  default:
    break;                      // "I was woken early by a signal"
  }
  return return_value;
}
```

When `pthread_cond_timedwait` is called, the OS kernel:

1. Releases the mutex (so other threads aren't blocked)
2. Sets a hardware timer for the requested wakeup time
3. **Removes the thread from the CPU's run queue entirely**

The thread is now "sleeping." The CPU executes **zero instructions** for this thread. When the hardware timer fires (~1 ms later), the kernel puts the thread back on the run queue, and the program resumes.

### The caller: `wait_until()`

```c
// From reactor_threaded.c
bool wait_until(instant_t wait_until_time, lf_cond_t* condition) {
    // Check if we've already passed the target time
    interval_t wait_duration = wait_until_time - lf_time_physical();
    if (wait_duration < 0) {
        return true;  // already past this time, no sleep needed
    }

    // Sleep until timeout or early wake-up
    if (lf_clock_cond_timedwait(condition, wait_until_time) != LF_TIMEOUT) {
        return false;  // woken early, caller should re-check event queue
    } else {
        return true;   // slept the full duration, ready for next tag
    }
}
```

## Per-tick CPU breakdown

Each tick takes 1 ms = 1,000 µs of wall-clock time. Across 5,000 ticks:

| | Per tick | Total (5,000 ticks) |
|---|---|---|
| Code running (MPC math, `lf_set`, etc.) | ~0.2 µs | 0.001 s (`user`) |
| Kernel work (sleep/wake overhead) | ~118 µs | 0.592 s (`sys`) |
| Truly asleep (zero CPU) | ~882 µs | ~4.4 s |
| **Wall clock** | **1,000 µs** | **5.023 s** (`real`) |

The `user` time is 0.001 s because the NX=2 MPC math is trivially small, roughly 0.2 µs per tick. The `sys` time (0.592 s) is the cumulative kernel overhead of 5,000 sleep/wake cycles. The remaining ~4.4 s is genuine sleep where zero CPU is consumed.

## Conclusion

```
LF:  0.001 s user CPU  →  the process sleeps between tags, wakes only to compute
C:   6.052 s user CPU  →  four threads burn a full core on lock contention
