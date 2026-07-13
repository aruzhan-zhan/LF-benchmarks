# Lingua Franca Benchmarks

A collection of deterministic, real-time Cyber-Physical System (CPS) benchmarks implemented in [Lingua Franca (LF)](https://www.lf-lang.org/), with POSIX C baselines for comparison.

## Table of Contents

- [1. MPC (Model Predictive Control)](#1-mpc-model-predictive-control)
  - [Project Files](#-project-files)
  - [System Architecture](#system-architecture)
  - [Quick Start and Benchmarking](#quick-start-and-benchmarking)
  - [Execution Results and Timing Analysis](#execution-results--timing-analysis)
  - [Distributed Execution (RTI)](#-distributed-execution-rti)
  - [Performance Results and Real-Time Safety](#-performance-results--real-time-safety)
  - [Architectural Coding Standards](#architectural-coding-standards-enforced)
  - [Why is User CPU So Low?](#why-is-user-cpu-so-low-0001-s)
  - [How Reactors are Connected in Multicore](#how-reactors-are-connected-in-multicore-main-reactor)
  - [How Logical Time Works](#how-logical-time-works-in-lingua-franca)
- [2. Fly-By-Wire (Triple Modular Redundancy)](#2-fly-by-wire-triple-modular-redundancy)
  - [Overview](#overview)
  - [Project Files](#-fly-by-wire-project-files)
  - [Original C Architecture](#original-c-architecture)
  - [Lingua Franca Architecture](#lingua-franca-architecture)
  - [Architectural Translation from C to LF](#architectural-translation-from-c-to-lf)
  - [Quick Start](#quick-start)
  - [Validation Methodology](#validation-methodology)
  - [Validation Results](#validation-results)
  - [Controlled Fault Sequence](#controlled-fault-sequence)
  - [Original C WES Findings](#original-c-wes-findings)
  - [Key Improvements](#key-improvements)


## 1. MPC (Model Predictive Control)
This is a deterministic, real-time Cyber-Physical System (CPS) benchmark for robotics. It demonstrates the translation of a multi-threaded Model Predictive Control (MPC) architecture from standard POSIX C threads into [Lingua Franca (LF)](https://www.lf-lang.org/).

The primary goal of this project is to eliminate the non-determinism, race conditions, and silent real-time failures inherent in standard C multithreading by leveraging Lingua Franca's event-driven, time-aware actor model.

### 📂 Project Files

**Source Code**
* [`mpc-lf-main/mpc.lf`](mpc/mpc-code/mpc-lf-main/mpc.lf) - The deterministic Lingua Franca (LF) implementation of the MPC architecture (non-federated).
* [`mpc-lf-federated/mpc_fed.lf`](mpc/mpc-code/mpc-lf-federated/mpc_fed.lf) - The deterministic Lingua Franca (LF) implementation of the MPC architecture (non-federated).
* [`mpc_threaded.c`](mpc/mpc-code/mpc_threaded.c) - The original POSIX-threaded C baseline used for comparison.

**Benchmarking and Automation**
* [`run_benchmark.py`](mpc/mpc-code/run_benchmark.py) - Python test harness that automates compilation, execution, output parsing, and graph generation.

**Assets and Diagrams**
* [`diagrams-and-results/architecture.png`](mpc/diagrams-and-results/architecture.png) - Visual representation of the Lingua Franca reactor network and data flow.

**Execution Results**
* [`diagrams-and-results/lf_results.txt`](mpc/diagrams-and-results/lf_results.txt) - Raw execution log of the time-aware LF simulation.
* [`diagrams-and-results/c_results.txt`](mpc/diagrams-and-results/c_results.txt) - Raw execution log of the standard C multithreaded simulation.

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
rm -rf src-gen bin
lfc mpc.lf
time ./bin/mpc
```

### Execution Results and Timing Analysis
![C vs. LF](mpc/diagrams-and-results/comparison.png)
![Logical timestamps in LF](mpc/diagrams-and-results/logical_timestamps_printed.png)

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

### What is the RTI?

The RTI (Run-Time Infrastructure) is a central software bus that allows different programs, running on different computers, to talk to each other and share a single unified timeline. It handles the networking, the message routing, and most importantly, the synchronization of time across physical boundaries.

In standard C, if you want a robot's brain to run on a laptop and its spinal cord to run on a chassis, you have to manually write thousands of lines of TCP/IP socket code and deal with network lag causing the robot to crash.

In Lingua Franca, the RTI is an automatically generated **Time Conductor**. When you compile a distributed program, LF builds a dedicated server (the RTI) whose sole job is to force every machine on the network to obey strict Logical Time. Before any node is allowed to execute tick 1500, the RTI ensures that all network messages from tick 1499 have been delivered, and that everyone's physical clocks are perfectly synchronized. It provides deterministic, real-time safety guarantees over an unpredictable Wi-Fi/Ethernet network.

### How We Implemented It

We changed `main reactor` to `federated reactor`:

```lf
federated reactor {
    ref = new Reference()
    plant = new Plant()
    optimizer = new Optimizer()

    ref.x_ref         -> optimizer.x_ref_in
    plant.x           -> optimizer.x_current
    optimizer.u_apply  -> plant.u
}
```

By doing this, the Lingua Franca compiler completely changed its output. Instead of building one executable, it built four independent network binaries:

| Binary | Role |
|---|---|
| `RTI` | The central time-keeper |
| `mpc_ref` | The Remote Control (reference setpoint) |
| `mpc_optimizer` | The Brain (gradient descent MPC solver) |
| `mpc_plant` | The Robot Chassis (sensor readings + actuator commands) |

Because everything ran on a single machine for the benchmark, Lingua Franca intelligently bundled these into a single launcher (`./bin/mpc`) that spun up the RTI server in the background and connected the three federates via local TCP/IP sockets.

### The Execution Lifecycle

**Phase 1: Clock Synchronization (Google Spanner Protocol)**

The RTI boots up first and waits. As `plant`, `optimizer`, and `ref` connect to the network, the RTI refuses to let the simulation start. It first forces them to exchange dozens of ping messages to measure the network latency between them. It then locks their internal clocks together. All three federates reported the exact same starting nanosecond (`1780087519273407295`).

**Phase 2: Latency Catch**

Once execution begins, data must flow through network sockets. Initially, routing the heavy matrix math from the Optimizer to the Plant took slightly longer than the strict 1 ms hardware deadline. Lingua Franca detected this network lag immediately. Instead of using stale, delayed packets, the Plant caught the violation and safely applied the emergency brakes (`u = 0.0`).

**Phase 3: Lock-Step Execution**

Once the network sockets warmed up, the RTI coordinated the distributed system flawlessly. Over a 5.000-second logical simulation, the total physical execution time was 5.002 seconds. The RTI managed all the TCP/IP network overhead in just 2 milliseconds.

![RTI startup and clock synchronization](mpc/diagrams-and-results/rti1.png)
![RTI execution and shutdown](mpc/diagrams-and-results/rti2.png)


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

At each tag, the runtime processes all triggered reactions (the level 0→5 pipeline: sensor → reference → optimizer → actuator).

After all reactions complete, the runtime calls `_lf_next_locked()`, which sleeps until the next tag is due, typically ~1 ms away. During that sleep, **zero user CPU is consumed**.

## The sleep chain in reactor-c

The call chain from "all reactions done" to "thread asleep" is:

```
_lf_next_locked()                         // core/threaded/reactor_threaded.c: advance to next tag
  → get_next_event_tag(env)               // peek at event queue: next tag is 1 ms away
  → wait_until(next_tag.time, &cond)      // reactor_threaded.c: sleep until physical time catches up
    → lf_clock_cond_timedwait(cond, t)    // platform layer: OS-level timed wait. core/clock.c
      → _lf_cond_timedwait(cond, t)       // mpc/low_level_platform/impl/src/lf_POSIX_threads_support.c
        → pthread_cond_timedwait(...)     // POSIX: thread removed from CPU entirely
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
    return_value = LF_TIMEOUT;  // slept the full time
    break;
  default:
    break;                      // woken up early by a signal
  }
  return return_value;
}
```

When the hardware timer fires (~1 ms later), the kernel puts the thread back on the run queue, and the program resumes.

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

## Conclusion

```
LF:  0.001 s user CPU  →  the process sleeps between tags, wakes only to compute
C:   6.052 s user CPU  →  four threads burn a full core on lock contention
```

# How Reactors are Connected in Multicore (Main Reactor)

All reactors live in a single process, share an address space, and are connected by three compile-time mechanisms:

1. **Pointer aliasing:** ports are connected by making the input point to the output's memory
2. **Static trigger arrays:** each reaction knows its downstream triggers at compile time
3. **Level-ordered scheduling:** reactions are dispatched by integer priority, with a worker thread pool (deterministic execution order)

All three are set up once during initialization in the generated `mpc.c` file, inside `_lf_initialize_trigger_objects()`. Nothing is computed or negotiated at runtime.

## The three `->` arrows in `mpc.lf`

```lf
main reactor {
    ref = new Reference()
    plant = new Plant()
    optimizer = new Optimizer()

    ref.x_ref         -> optimizer.x_ref_in     // connection 1
    plant.x           -> optimizer.x_current    // connection 2
    optimizer.u_apply  -> plant.u               // connection 3
}
```

Each of these arrows compiles into all three mechanisms described below.

---

## Mechanism 1: Port Wiring: Pointer Aliasing (Zero-Copy)

Near the bottom of `mpc.c`, under the comment `// Connect inputs and outputs`, each `->` arrow becomes a single pointer assignment:

```c
// Connection 1: ref.x_ref -> optimizer.x_ref_in
mpc_optimizer_self[dst_runtime]->_lf_x_ref_in =
    (_optimizer_x_ref_in_t*)&mpc_ref_self[src_runtime]->_lf_x_ref;

// Connection 2: plant.x -> optimizer.x_current
mpc_optimizer_self[dst_runtime]->_lf_x_current =
    (_optimizer_x_current_t*)&mpc_plant_self[src_runtime]->_lf_x;

// Connection 3: optimizer.u_apply -> plant.u
mpc_plant_self[dst_runtime]->_lf_u =
    (_plant_u_t*)&mpc_optimizer_self[src_runtime]->_lf_u_apply;
```

### What this means

The destination's input port **is** the source's output port: same memory address. When the Plant's sensor calls `lf_set(x, current_x)`, it writes into `plant._lf_x.value`. When the Optimizer reads `x_current->value`, it's reading from the exact same struct through a pointer cast. The data is "there" the moment `lf_set` writes it.

### How to find these lines

Search for `// Connect inputs and outputs` in `mpc.c`. The comments above each block identify the connection:

```c
// Connect mpc.ref.x_ref(0,1)->[mpc.optimizer.x_ref_in(0,1)] to port mpc.optimizer.x_ref_in(0,1)
```

---

## Mechanism 2: Trigger Arrays: when the output is produced, who wakes up

Each reaction has a pre-built array of downstream triggers. These are set up in the `// non-nested deferred initialize` section of `mpc.c`.

### The trigger wiring

```c
// Plant sensor (reaction_0) → triggers optimizer's x_current input
mpc_plant_self[src_runtime]->_lf__reaction_0
    .triggers[triggers_index[src_runtime] + src_channel][0]
    = &mpc_optimizer_self[dst_runtime]->_lf__x_current;

// Optimizer MPC (reaction_1) → triggers plant's u input
mpc_optimizer_self[src_runtime]->_lf__reaction_1
    .triggers[triggers_index[src_runtime] + src_channel][0]
    = &mpc_plant_self[dst_runtime]->_lf__u;

// Reference (reaction_0) → triggers optimizer's x_ref_in input
mpc_ref_self[src_runtime]->_lf__reaction_0
    .triggers[triggers_index[src_runtime] + src_channel][0]
    = &mpc_optimizer_self[dst_runtime]->_lf__x_ref_in;
```

> **Note:** Search for `// Point to destination port` to find each line.

### The `output_produced` flag

Each reaction also tracks which output ports it can produce. This is how the runtime knows whether `lf_set` was called:

```c
// Plant sensor knows it can produce on port plant.x
mpc_plant_self[0]->_lf__reaction_0.output_produced[count++]
    = &mpc_plant_self[0]->_lf_x.is_present;
```

### How triggers fire at runtime

After a reaction runs, `_lf_worker_invoke_reaction()` (in `reactor_threaded.c`) calls `schedule_output_reactions()`:

```c
static void _lf_worker_invoke_reaction(env, worker_number, reaction) {
    _lf_invoke_reaction(env, reaction, worker_number);      // run the reaction's C code
    schedule_output_reactions(env, reaction, worker_number); // walk triggers, enqueue downstream
}
```

`schedule_output_reactions()` (in `core/reactor_common.c`) does:

```
for each output port of this reaction:
    if output_produced[i] is true:            // was lf_set() called?
        for each trigger in triggers[i]:      // walk the static trigger array
            for each reaction on that trigger:
                enqueue it into the scheduler (sorted by level)
```

### Debug output proof

The debug log confirms this cascade for every tick:

```
Worker 0: Invoking reaction mpc.plant reaction 0        ← sensor runs, calls lf_set(x, ...)
  Output 0 has been produced.                            ← is_present = true
  Trigger 0x648f933063d8 lists 1 reactions.              ← follows triggers[0][0]
  Enqueueing reaction mpc.optimizer reaction 1, level 4  ← downstream reaction enqueued
```

---

## Mechanism 3: Level order

Each reaction is assigned a topological level at compile time. The scheduler processes all reactions at level N before any at level N+1.
From https://github.com/lf-lang/lingua-franca
core/src/main/java/org/lflang/generator/ReactionInstanceGraph.java
```c
  /**
   * Analyze the dependencies between reactions and assign each reaction instance a level. This
   * method removes nodes from this graph as it assigns levels. Any remaining nodes are part of
   * causality cycles.
   *
   * <p>This procedure is based on Kahn's algorithm for topological sorting. Rather than
   * establishing a total order, we establish a partial order. In this order, the level of each
   * reaction is the least upper bound of the levels of the reactions it depends on.
   */
  private void assignLevels() {
    List<ReactionInstance.Runtime> start = new ArrayList<>(rootNodes());

    // All root nodes start with level 0.
    for (Runtime origin : start) {
      origin.level = 0;
    }

    // No need to do any of this if there are no root nodes;
    // the graph must be cyclic.
    while (!start.isEmpty()) {
      Runtime origin = start.remove(0);
      Set<Runtime> toRemove = new LinkedHashSet<>();
      Set<Runtime> downstreamAdjacentNodes = getDownstreamAdjacentNodes(origin);

      // Visit effect nodes.
      for (Runtime effect : downstreamAdjacentNodes) {
        // Stage edge between origin and effect for removal.
        toRemove.add(effect);

        // Update level of downstream node.
        if (effect.level <= origin.level) {
          effect.level = origin.level + 1;
        }
      }
      // Remove visited edges.
      for (Runtime effect : toRemove) {
        removeEdge(effect, origin);
        // If the effect node has no more incoming edges,
        // then move it in the start set.
        if (getUpstreamAdjacentNodes(effect).isEmpty()) {
          start.add(effect);
        }
      }

      // Remove visited origin.
      removeNode(origin);
      assignPortLevel(origin);

      // Update the number of reactions per level for the enclave that contains the origin reaction.
      ReactionInstance reaction = origin.getReaction();
      ReactorInstance enclaveTop = reaction.getContainingEnclaveReactor();
      // Update numReactionsPerLevel info
      adjustNumReactionsPerLevel(origin.level, enclaveTop);
    }
  }
```

### Level assignments in `mpc.c`

```c
// Level 0: ref startup + plant sensor (can run in PARALLEL)
mpc_ref_self[0]->_lf__reaction_0.index   = lf_combine_deadline_and_level(1000000, 0);
mpc_plant_self[0]->_lf__reaction_0.index  = lf_combine_deadline_and_level(1000000, 0);

// Level 1: ref timer t1
mpc_ref_self[0]->_lf__reaction_1.index    = lf_combine_deadline_and_level(1000000, 1);

// Level 2: ref timer t2
mpc_ref_self[0]->_lf__reaction_2.index    = lf_combine_deadline_and_level(1000000, 2);

// Level 3: optimizer handles x_ref_in
mpc_optimizer_self[0]->_lf__reaction_0.index = lf_combine_deadline_and_level(1000000, 3);

// Level 4: optimizer MPC math (has 1ms deadline)
mpc_optimizer_self[0]->_lf__reaction_1.index = lf_combine_deadline_and_level(1000000, 4);

// Level 5: plant actuator (downstream of everything)
mpc_plant_self[0]->_lf__reaction_1.index  = lf_combine_deadline_and_level(9223372036854775807, 5);
```

### The scheduler array

```c
size_t num_reactions_per_level[6] = {2, 1, 1, 1, 1, 1};
```

This single array is the entire compiled schedule:
- **Level 0** has **2** reactions (Reference startup + Plant sensor): these can run on separate worker threads in **parallel**
- **Levels 1–5** each have **1** reaction: strictly sequential

### Debug output proof

```
Worker 0 popping reaction with level 0     ← plant sensor
Worker 1 popping reaction with level 0     ← ref startup (PARALLEL: different worker)
...
Worker 0 popping reaction with level 3     ← optimizer ref handler
Worker 0 popping reaction with level 4     ← optimizer MPC
Worker 0 popping reaction with level 5     ← plant actuator
```

---

## Putting It All Together: One Tick

Here is the complete pipeline for a single 1 ms tick at tag `(T, 0)`:

```
Timer fires at tag (T, 0)
    │
    ├── _lf_pop_events() pulls timer event from event queue
    │   └── enqueues Plant.reaction_0 (L0) and Ref.reaction_0 (L0)
    │
    ├── Worker 0 runs Plant.reaction_0 (sensor)                    LEVEL 0
    │   ├── lf_set(x, current_x)
    │   │   └── writes into plant._lf_x.value, sets is_present=true
    │   └── schedule_output_reactions()
    │       └── walks triggers[0][0] → enqueues Optimizer.reaction_1 (L4)
    │
    ├── Worker 1 runs Ref.reaction_0 (startup)                     LEVEL 0  ← PARALLEL
    │   ├── lf_set(x_ref, ref)
    │   │   └── writes into ref._lf_x_ref.value, sets is_present=true
    │   └── schedule_output_reactions()
    │       └── walks triggers[0][0] → enqueues Optimizer.reaction_0 (L3)
    │
    ├── Worker 0 runs Optimizer.reaction_0 (L3)                    LEVEL 3
    │   └── reads x_ref_in->value
    │       └── pointer dereference → reads ref._lf_x_ref (same memory)
    │       └── no output produced → no cascade
    │
    ├── Worker 0 runs Optimizer.reaction_1 (L4)                    LEVEL 4
    │   ├── reads x_current->value
    │   │   └── pointer dereference → reads plant._lf_x (same memory)
    │   ├── MPC gradient descent (50 iterations)
    │   ├── lf_set(u_apply, u_seq[0][0])
    │   └── schedule_output_reactions()
    │       └── walks triggers[0][0] → enqueues Plant.reaction_1 (L5)
    │
    ├── Worker 0 runs Plant.reaction_1 (L5, actuator)              LEVEL 5
    │   └── reads u->value
    │       └── pointer dereference → reads optimizer._lf_u_apply (same memory)
    │       └── updates plant_x state, no output → end of chain
    │
    └── _lf_next_locked()
        └── wait_until() → pthread_cond_timedwait() → sleep ~1 ms
```

---

## Contrast: Main Reactor vs. Federated Reactor

| | Main Reactor (multicore) | Federated Reactor |
|---|---|---|
| **Process model** | Single binary, shared address space, worker thread pool | Separate binary per reactor, separate address spaces |
| **Data transfer** | Pointer alias: zero-copy, same memory | Serialize → TCP socket → RTI routes → TCP → deserialize |
| **Signaling** | `schedule_output_reactions()` walks a static C array in-process | `MSG_TYPE_TAGGED_MESSAGE` sent over network |
| **Time coordination** | Single scheduler advances all reactors via `_lf_next_locked()` | RTI exchanges `NET` / `TAG` messages to grant time advancement |
| **Overhead per message** | One pointer dereference + array walk | Network round-trip + serialization/deserialization |

The `mpc.lf` source code is **identical** in both modes. The three `->` connection lines mean the same thing semantically: the compiler decides whether they become pointer casts (main reactor) or network channels (federated reactor) based on the top-level reactor declaration.

---

## Source File Reference

| Evidence | Location |
|---|---|
| Port pointer aliasing | `mpc.c`: search `// Connect inputs and outputs` |
| Trigger array wiring | `mpc.c`: search `// Point to destination port` |
| `output_produced` flags | `mpc.c`: search `output_produced[count++]` |
| Level assignments | `mpc.c`: search `lf_combine_deadline_and_level` |
| Scheduler array | `mpc.c`: search `num_reactions_per_level` |
| Worker invoke + cascade | `reactor_threaded.c`: `_lf_worker_invoke_reaction()` |
| Sleep between tags | `reactor_threaded.c`: `_lf_next_locked()` → `wait_until()` |
| OS-level sleep | `lf_POSIX_threads_support.c`: `_lf_cond_timedwait()` |

# How Logical Time Works in Lingua Franca

Logical time in Lingua Franca is is a **variable that jumps forward from event to event.** The runtime reads the next event's timestamp from a priority queue and sets the current tag to that value. Between events, logical time does not advance: the thread sleeps.

## The Variable That Holds Logical Time

Every environment has one field that stores the current logical time:

```c
env->current_tag    // struct: { .time = nanoseconds, .microstep = int }
```

A **tag** is a `(time, microstep)` pair. The `time` component is an absolute timestamp in nanoseconds. The `microstep` handles logically simultaneous events: if two things happen at the same physical time, they get microsteps 0, 1, 2, etc.

For our MPC running at 1 kHz for 5 seconds, the tags are:

```
(0, 0)          ← startup
(1000000, 0)    ← 1 ms
(2000000, 0)    ← 2 ms
(3000000, 0)    ← 3 ms
...
(5000000000, 0) ← 5 seconds (stop tag)
```

There is no intermediate logical time between tags: no 0.5 ms, no 1.5 ms. Logical time jumps directly from one event to the next.

## How Logical Time Advances

The advancement happens inside `_lf_next_locked()` in `reactor_threaded.c`, in three steps:

### Step 1: Peek at the event queue

```c
tag_t next_tag = get_next_event_tag(env);
```

The event queue is a **priority queue sorted by tag.** `get_next_event_tag()` peeks at the earliest event without removing it. For our MPC, this returns the next timer tick (e.g., `{.time = start_time + 3000000, .microstep = 0}` for the 3 ms tag).

### Step 2: Sleep until physical time catches up

```c
// Inside the while(true) loop in _lf_next_locked():
if (wait_until(next_tag.time, &env->event_q_changed)) {
    break;  // slept the full time: ready to advance
}
```

`wait_until()` calls `lf_clock_cond_timedwait()`, which calls `pthread_cond_timedwait()`, which puts the thread to sleep until either the target time is reached or the condition variable is signaled. During this sleep, zero user CPU is consumed.

### Step 3: Jump logical time forward

```c
_lf_advance_tag(env, next_tag);
// This does: env->current_tag = next_tag
```

Logical time is now at the new tag. It didn't increment through intermediate values, it jumped directly. If the previous tag was `(2000000, 0)` and the next event is at `(3000000, 0)`, logical time jumps from 2 ms to 3 ms in one assignment.

### After advancing: pop events and run reactions

```c
_lf_start_time_step(env);     // reset ports from previous tag
_lf_pop_events(env);          // pull events at this tag, enqueue triggered reactions
```

`_lf_pop_events()` removes all events with the current tag from the event queue and enqueues their triggered reactions into the scheduler's reaction queue, sorted by level. Workers then pick up and execute those reactions.

## Where Events Come From

Events enter the priority queue in two ways:

### 1. Timers: periodic events

In the generated `mpc.c`, the Plant's timer is registered at initialization:

```c
mpc_plant_self[0]->_lf__t.offset = 0;        // first fire at time 0
mpc_plant_self[0]->_lf__t.period = MSEC(1);   // then every 1 ms
```

When a timer fires at tag `(T, 0)`, the runtime inserts the **next** firing into the event queue at tag `(T + period, 0)`. The debug output shows this:

```
Inserting event in the event queue with elapsed tag (2000000, 0).
```

That's the 1 ms timer saying "fire me again at 2 ms." This creates the chain of events that drives the entire MPC loop.

Similarly, the Reference reactor's one-shot timers are registered:

```c
mpc_ref_self[0]->_lf__t1.offset = SEC(1);   // fire once at t = 1 second
mpc_ref_self[0]->_lf__t1.period = 0;         // period = 0 → one-shot, no repeat

mpc_ref_self[0]->_lf__t2.offset = SEC(3);   // fire once at t = 3 seconds
mpc_ref_self[0]->_lf__t2.period = 0;
```

### 2. Reactions that call `lf_set`: cascades within the same tag

When a reaction calls `lf_set()` on an output port, the downstream reactions are enqueued at the **current** tag, logical time does not advance. The cascade happens within the same logical instant:

```
Tag (1000000, 0):
    Plant sensor runs          → lf_set(x, ...)      → enqueues Optimizer L4
    Optimizer runs             → lf_set(u_apply, ...) → enqueues Plant actuator L5
    Plant actuator runs        → updates state, no output
    
    All at the SAME logical time: 1 ms
```

## Event Queue State During Execution

At startup, the event queue contains:

```
(0, 0)            ← Plant timer (period=1ms) + startup events
(1000000000, 0)   ← Reference t1 (fires once at 1 second)
(3000000000, 0)   ← Reference t2 (fires once at 3 seconds)
```

After processing tag `(0, 0)`, the Plant timer inserts its next firing:

```
(1000000, 0)      ← Plant timer next tick (1 ms)
(1000000000, 0)   ← Reference t1 (still waiting)
(3000000000, 0)   ← Reference t2 (still waiting)
```

After processing tag `(1000000, 0)`:

```
(2000000, 0)      ← Plant timer next tick (2 ms)
(1000000000, 0)   ← Reference t1
(3000000000, 0)   ← Reference t2
```

This pattern continues: each tick pops one timer event and inserts the next one. The Reference timers sit in the queue until their time comes.

## Logical Time vs. Physical Time

| | Logical time | Physical time |
|---|---|---|
| **What it is** | A variable: `env->current_tag` | The hardware clock: `clock_gettime()` |
| **How it advances** | Jumps from event to event | Flows continuously |
| **Who controls it** | The event queue | Physics (the real world) |
| **Read via** | `lf_time_logical_elapsed()` | `lf_time_physical_elapsed()` |

The deadline mechanism connects the two: if physical time exceeds logical time + deadline, the violation handler fires. This is checked in `_lf_worker_handle_deadline_violation_for_reaction()`:

```c
if (reaction->deadline >= 0LL) {
    instant_t physical_time = lf_time_physical();
    if (physical_time > lf_time_add(env->current_tag.time, reaction->deadline)) {
        // Deadline violation: invoke handler (emergency brake)
    }
}
```

## Conclusion

Logical time **jumps** from event to event. If a program had no events between 1 ms and 500 ms, logical time would jump directly from 1 ms to 500 ms in one step, and `wait_until()` would sleep for 499 ms. The event queue drives everything. No events means no time advancement.

This is why `lf_time_logical_elapsed()` works in print statements: it just reads `env->current_tag.time - start_time` and returns the difference. It's reading a variable, not measuring a clock.

## The Complete Cycle: One Tick

```
env->current_tag = (T, 0)           ← logical time is at T

_lf_pop_events()                    ← pull events at tag (T, 0)
  └── enqueues triggered reactions

Workers execute reactions:
  L0: Plant sensor → lf_set(x)     ← cascade: enqueue L4
  L4: Optimizer MPC → lf_set(u)    ← cascade: enqueue L5
  L5: Plant actuator               ← end of chain

_lf_next_locked():
  next_tag = get_next_event_tag()   ← peek: (T + 1ms, 0)
  wait_until(next_tag.time)         ← sleep until physical time reaches T + 1ms
  _lf_advance_tag(env, next_tag)    ← env->current_tag = (T + 1ms, 0)

env->current_tag = (T + 1ms, 0)    ← logical time jumped forward

...repeat 5,000 times...
```

## Source File Reference

| Concept | Location |
|---|---|
| Current tag variable | `env->current_tag` (defined in `environment.h`) |
| Tag advancement | `_lf_advance_tag()` called in `_lf_next_locked()` (`reactor_threaded.c`) |
| Event queue peek | `get_next_event_tag()` (`reactor_threaded.c`) |
| Event popping | `_lf_pop_events()` (`reactor_common.c`) |
| Sleep between tags | `wait_until()` → `lf_clock_cond_timedwait()` → `_lf_cond_timedwait()` |
| Timer registration | `mpc.c`:`_lf__t.offset` and `_lf__t.period` fields |
| Timer rescheduling | `lf_schedule_trigger()` called after timer fires |
| Deadline check | `_lf_worker_handle_deadline_violation_for_reaction()` (`reactor_threaded.c`) |
| Read logical time | `lf_time_logical_elapsed()` → reads `env->current_tag.time - start_time` (mpc/core/reactor_common.c)|
| Read physical time | `lf_time_physical()` → calls `clock_gettime(CLOCK_MONOTONIC)` (mpc/low_level_platform/impl/src/lf_windows_support.c)|



## 2. Fly-By-Wire (Triple Modular Redundancy)

### Overview

This benchmark is a deterministic Lingua Franca implementation of the [Wabri Fly-By-Wire benchmark](https://github.com/Wabri/Fly-By-Wire), a fault-tolerant flight-control system originally implemented as seven communicating POSIX C processes.

The system uses **Triple Modular Redundancy (TMR)**. Three identical Primary Flight Computers independently process the same GPS input and calculate the aircraft speed. A Warning Electronics System compares their outputs using two-out-of-three voting:

* If all three PFCs agree, the system continues normally.
* If two PFCs agree, the remaining PFC is identified as faulty.
* If no two PFCs agree, the system enters an emergency state.

The original C implementation uses several unrelated operating-system communication mechanisms:

* UNIX sockets
* Named pipes
* Shared files
* POSIX signals
* PID files
* Process creation with `fork()`
* Log-file polling

The Lingua Franca implementation replaces these mechanisms with a deterministic reactor network connected through typed ports and coordinated by logical time.

---

### 📂 Fly-By-Wire Project Files

**Main Implementations**

* [`src/fbw.lf`](fbw-lf/src/fbw.lf): Main Fly-By-Wire LF implementation with probabilistic fault generation.
* [`src/fbw_controlled_test.lf`](fbw-lf/src/fbw_controlled_test.lf): Deterministic end-to-end fault test using a fixed `STOP → CONT → BIAS` sequence.
* [`src/fbw_c_equiv_validation.lf`](fbw-lf/src/fbw_c_equiv_validation.lf): Compatibility build that reproduces the original C numerical semantics for direct counter-by-counter comparison.
* [`src/fbw_nav.h`](fbw-lf/src/fbw_nav.h): NMEA parsing, Haversine distance calculation, message types, status codes, and fault helpers.

**Behavioral Tests**

* [`tests/behavior/wes_lf_behavior_test.lf`](fbw-lf/tests/behavior/wes_lf_behavior_test.lf): Isolated test of the corrected symmetric LF voting logic.
* [`tests/behavior/wes_original_behavior_test.c`](fbw-lf/tests/behavior/wes_original_behavior_test.c): Small C harness reproducing the original WES decision structure.

**Validation Scripts**

* [`scripts/compare_fbw.py`](fbw-lf/scripts/compare_fbw.py): Compares numerical C and LF speed outputs by counter.
* [`scripts/compare_behavior.py`](fbw-lf/scripts/compare_behavior.py): Compares original C, expected TMR, and LF voting decisions.
* [`scripts/check_e2e_faults.py`](fbw-lf/scripts/check_e2e_faults.py): Automatically verifies the complete controlled LF fault sequence.

**Input Data**

* [`data/G18.txt`](fbw-lf/data/G18.txt): Complete GPS/NMEA dataset from the original benchmark.
* [`data/G18_validation.txt`](fbw-lf/data/G18_validation.txt): A 100-record moving-data slice used for numerical validation.

**Diagrams**

* [`diagrams/fbw_original_system_architecture.png`](fbw-lf/diagrams/fbw_original_system_architecture.png): Original POSIX C process and IPC architecture.
* [`diagrams/fbw_lf_architecture.png`](fbw-lf/diagrams/fbw_lf_architecture.png): Lingua Franca reactor and port architecture.

**Validation Results**

* [`results/numerical/summary.txt`](fbw-lf/results/numerical/summary.txt): Numerical equivalence summary.
* [`results/numerical/fbw_c_equiv_comparison.csv`](fbw-lf/results/numerical/fbw_c_equiv_comparison.csv) — Counter-by-counter numerical comparison.
* [`results/behavior/summary.txt`](fbw-lf/results/behavior/summary.txt): Voting behavior comparison.
* [`results/e2e/summary.txt`](fbw-lf/results/e2e/summary.txt): Automated end-to-end fault validation.
* [`results/e2e/fault_trace.txt`](fbw-lf/results/e2e/fault_trace.txt): Condensed trace of the controlled fault sequence.

---

### Original C Architecture

![Original Fly-By-Wire Architecture](fbw-lf/diagrams/fbw_original_system_architecture.png)

The original benchmark separates the system into seven POSIX processes:

| Component  | Purpose                                                       |
| ---------- | ------------------------------------------------------------- |
| `PFC1`     | Processes GPS data and sends its result through a UNIX socket |
| `PFC2`     | Processes GPS data and sends its result through a named pipe  |
| `PFC3`     | Processes GPS data and sends its result through a shared file |
| Transducer | Receives the three PFC outputs                                |
| WES        | Performs two-out-of-three voting                              |
| FMAN       | Injects process faults through POSIX signals                  |
| PFCDS      | Logs faults and disconnects the system during an emergency    |

Although all three PFCs perform the same computation, each one communicates through a different IPC mechanism. The architecture therefore combines process management, sockets, pipes, shared files, signal handling, PID tracking, and log-file synchronization.

This makes the system difficult to reason about as one deterministic application. Correctness depends not only on the navigation calculation, but also on operating-system process scheduling and the behavior of several independent communication mechanisms.

---

### Lingua Franca Architecture

![Lingua Franca Fly-By-Wire Architecture](fbw-lf/diagrams/fbw_lf_architecture.png)

The LF implementation preserves the logical components of the original system while replacing the operating-system infrastructure with reactors and typed ports.

```lf
main reactor {
    pfc1 = new PFC(id=1)
    pfc2 = new PFC(id=2)
    pfc3 = new PFC(id=3)

    trans = new Transducer()
    fman = new FailureGenerator()
    wes = new WES()
    pfcds = new DisconnectSwitch()

    pfc1.speed_out -> trans.speed1
    pfc2.speed_out -> trans.speed2
    pfc3.speed_out -> trans.speed3

    trans.out1 -> wes.speed1
    trans.out2 -> wes.speed2
    trans.out3 -> wes.speed3

    wes.status -> pfcds.status_in

    fman.fault1 -> pfc1.fault_in
    fman.fault2 -> pfc2.fault_in
    fman.fault3 -> pfc3.fault_in
}
```

The connection graph explicitly describes both communication and execution dependencies. The LF compiler uses this graph to generate the required scheduling and message-delivery code.

All three PFCs are instances of the same reactor and communicate through the same typed interface. Their behavior no longer depends on whether the original channel was implemented with a socket, pipe, or file.

---

### Architectural Translation from C to LF

| Original POSIX C mechanism            | Lingua Franca replacement               |
| ------------------------------------- | --------------------------------------- |
| Seven processes created with `fork()` | Seven reactor instances                 |
| `sleep(1)` control loops              | `timer t(0, 1 sec)`                     |
| UNIX socket                           | Typed LF port                           |
| Named pipe                            | Typed LF port                           |
| Shared data file                      | Typed LF port                           |
| PID files                             | Compile-time reactor references         |
| `SIGSTOP`                             | Structured STOP fault message           |
| `SIGCONT`                             | Structured CONT fault message           |
| `SIGINT`                              | Structured interrupt fault message      |
| `SIGUSR1` bias injection              | Structured BIAS fault message           |
| WES polling `speedPFC*.log` files     | Direct timestamped input ports          |
| Manual process shutdown               | `lf_request_stop()`                     |
| OS-dependent message ordering         | Deterministic tag and reaction ordering |

A fault message can carry several fault flags at one logical tag:

```c
typedef struct {
    int stop;
    int interrupt;
    int cont;
    int bias;
} fault_msg_t;
```

This structure is necessary because an LF port carries one value at each tag. Instead of writing repeatedly to the same port and overwriting earlier faults, FMAN combines every generated fault for that tick into one message.

---

### Quick Start

#### Prerequisites

To build and run the benchmark on Linux or WSL, install:

* `lfc`: Lingua Franca Compiler
* `gcc`
* `python3`

Clone this repository and enter the benchmark directory:

```bash
git clone https://github.com/aruzhan-zhan/LF-benchmarks.git
cd LF-benchmarks/fbw-lf
```

#### Run the Main Benchmark

The main implementation uses the complete GPS input and probabilistic fault generation:

```bash
lfc src/fbw.lf
./bin/fbw
```

The PFCs execute once per logical second. FMAN probabilistically generates STOP, interrupt, continuation, and bias faults.

Example output:

```text
[FMAN  ] T=42 s | BIAS sent to PFC3
[PFC3  ] T=42 s | BIAS scheduled
[PFC3  ] T=42 s | BIAS applied: 2.6841 -> 12.0000
[WES   ] T=42 s | ERROR: PFC3 discordant
[PFCDS ] T=42 s | ERROR: PFC3 faulty. Logged.
```

#### Run the Deterministic End-to-End Test

The controlled test executes in fast mode and injects a known fault sequence:

```bash
lfc src/fbw_controlled_test.lf
./bin/fbw_controlled_test > e2e_fault_output.txt
python3 scripts/check_e2e_faults.py
```

The test injects:

| Logical time | Event                     |
| -----------: | ------------------------- |
|        `5 s` | STOP PFC2                 |
|        `8 s` | CONT PFC2                 |
|      `640 s` | Apply BIAS to PFC3        |
|      `641 s` | Verify automatic recovery |

#### Run the Numerical C-Equivalence Build

Compile the compatibility implementation:

```bash
lfc src/fbw_c_equiv_validation.lf
./bin/fbw_c_equiv_validation > lf_c_equiv_output.txt
```

Compare it with the PFC1 log produced by the original C benchmark:

```bash
python3 scripts/compare_fbw.py \
  --c-log /path/to/Fly-By-Wire/log/speedPFC1.log \
  --lf-log lf_c_equiv_output.txt \
  --out fbw_c_equiv_comparison.csv
```

#### Run the Isolated Voting Test

Compile and run the original C decision structure:

```bash
gcc -Wall -Wextra -O0 \
  tests/behavior/wes_original_behavior_test.c \
  -o wes_original_behavior_test

./wes_original_behavior_test > c_behavior_output.txt
```

Compile and run the LF WES test:

```bash
lfc tests/behavior/wes_lf_behavior_test.lf
./bin/wes_lf_behavior_test > lf_behavior_output.txt
```

Compare the decisions:

```bash
python3 scripts/compare_behavior.py \
  c_behavior_output.txt \
  lf_behavior_output.txt
```

---

### Validation Methodology

The port was evaluated at three levels.

#### 1. Numerical Equivalence

The numerical compatibility build preserves two behaviors from the original C PFC:

* Single-precision `float` arithmetic
* The original one-sample output delay

Both implementations process the same 100-record NMEA input slice. Their outputs are matched by counter and compared using an absolute tolerance of `1 × 10⁻⁴`.

This test evaluates whether the navigation parsing and speed calculation were ported correctly without allowing architectural improvements to change the baseline behavior.

#### 2. Voting-Logic Validation

The original C WES decision structure and the LF WES are tested using the same nine synthetic input scenarios.

The scenarios cover:

* All three PFCs agreeing
* One PFC containing a biased speed
* One PFC containing a stale counter
* Recovery after a fault
* Three different counters
* Three different speeds

The LF implementation is evaluated against the intended symmetric two-out-of-three TMR rule.

#### 3. End-to-End Fault Validation

The deterministic LF test evaluates the complete reactor chain:

```text
FMAN
  ↓
PFC
  ↓
Transducer
  ↓
WES
  ↓
PFCDS
```

The automated checker verifies fault generation, PFC state changes, WES fault isolation, PFCDS reporting, recovery, and the absence of an unexpected emergency.

---

### Validation Results

#### Numerical Equivalence

| Metric                 |              Result |
| ---------------------- | ------------------: |
| C records              |                 100 |
| LF records             |                 100 |
| Common counters        |                 100 |
| Missing counters       |                   0 |
| Tolerance              |        `1.0 × 10⁻⁴` |
| Maximum absolute error |       `4.93 × 10⁻⁷` |
| Mean absolute error    |     `2.3069 × 10⁻⁷` |
| RMSE                   | `2.72175513 × 10⁻⁷` |
| Mismatched samples     |               **0** |

```text
RESULT: PASS - all counters and speeds match.
```

The maximum observed error is more than 200 times smaller than the selected tolerance. The remaining difference is consistent with the number of decimal places written to the original C log.

#### Voting Behavior

| Scenario             | Original C   | Expected TMR | LF           |
| -------------------- | ------------ | ------------ | ------------ |
| All agree            | `OK`         | `OK`         | `OK`         |
| PFC3 biased          | `ERROR_PFC3` | `ERROR_PFC3` | `ERROR_PFC3` |
| PFC2 stale           | `ERROR_PFC2` | `ERROR_PFC2` | `ERROR_PFC2` |
| Recovered            | `OK`         | `OK`         | `OK`         |
| PFC1 stale           | `ERROR_PFC1` | `ERROR_PFC1` | `ERROR_PFC1` |
| Different counters   | `EMERGENCY`  | `EMERGENCY`  | `EMERGENCY`  |
| PFC1 biased          | `ERROR_PFC2` | `ERROR_PFC1` | `ERROR_PFC1` |
| PFC2 biased          | `ERROR_PFC2` | `ERROR_PFC2` | `ERROR_PFC2` |
| All speeds different | `ERROR_PFC2` | `EMERGENCY`  | `EMERGENCY`  |

The LF implementation matched the expected symmetric TMR behavior in all nine scenarios.

The original C WES disagreed with the intended TMR decision in two scenarios.

#### End-to-End Fault Test

```text
Checks passed: 13
Checks failed: 0

RESULT: PASS - STOP, CONT, BIAS, WES voting,
PFCDS reporting, and recovery all worked end to end.
```

---

### Controlled Fault Sequence

The deterministic fault run demonstrates how faults propagate through the complete LF architecture.

#### STOP PFC2

At logical time `5 s`, FMAN stops PFC2:

```text
[FMAN  ] T=5 s | TEST: STOP sent to PFC2
[PFC2  ] T=5 s | STOPPED by failure generator
```

PFC1 and PFC3 advance to counter 5, while the most recently received PFC2 value remains at counter 4:

```text
[WES   ] T=5 s | ERROR: PFC2 discordant |
PFC1={c=5,s=0.0000}
PFC2={c=4,s=0.0000}
PFC3={c=5,s=0.0000}
```

PFCDS receives the WES status:

```text
[PFCDS ] T=5 s | ERROR: PFC2 faulty. Logged.
```

The WES continues to identify PFC2 as discordant at logical times 6 and 7.

#### CONT PFC2

At logical time `8 s`, FMAN resumes PFC2:

```text
[FMAN  ] T=8 s | TEST: CONT sent to PFC2
[PFC2  ] T=8 s | RESUMED by failure generator
[WES   ] T=8 s | OK: all speeds agree: 0.0000
```

The LF PFC continues advancing its internal GPS state and counter while its output is stopped. Therefore, it can immediately rejoin the other PFCs when CONT is received instead of remaining several samples behind.

#### BIAS PFC3

At logical time `640 s`, FMAN corrupts one transmitted PFC3 value:

```text
[FMAN  ] T=640 s | TEST: BIAS sent to PFC3
[PFC3  ] T=640 s | BIAS applied: 2.6841 -> 12.0000
```

The internal PFC3 navigation state is not corrupted. Only the transmitted sample is modified.

The WES compares all three timestamped outputs and isolates PFC3:

```text
[WES   ] T=640 s | ERROR: PFC3 discordant |
PFC1={c=640,s=2.6841}
PFC2={c=640,s=2.6841}
PFC3={c=640,s=12.0000}

[PFCDS ] T=640 s | ERROR: PFC3 faulty. Logged.
```

At the following logical tick, all three outputs agree again:

```text
[WES   ] T=641 s | OK: all speeds agree: 2.7225
```

---

### Original C WES Findings

The original C WES does not implement a fully symmetric two-out-of-three comparison.

Its first branch compares both the counters and speeds of PFC1 and PFC2. Some later branches compare only counters. This causes incorrect fault isolation in certain edge cases.

#### PFC1 Bias

Input:

```text
PFC1 = {counter=16, speed=28.0}
PFC2 = {counter=16, speed=7.0}
PFC3 = {counter=16, speed=7.0}
```

Correct result:

```text
ERROR_PFC1
```

Original C result:

```text
ERROR_PFC2
```

LF result:

```text
ERROR_PFC1
```

#### All Three Speeds Different

Input:

```text
PFC1 = {counter=18, speed=7.0}
PFC2 = {counter=18, speed=8.0}
PFC3 = {counter=18, speed=9.0}
```

Correct result:

```text
EMERGENCY
```

Original C result:

```text
ERROR_PFC2
```

LF result:

```text
EMERGENCY
```

The LF WES calculates all three pairwise matches symmetrically:

```c
match_12 = same_counter_12 && same_speed_12;
match_13 = same_counter_13 && same_speed_13;
match_23 = same_counter_23 && same_speed_23;
```

The decision is then independent of branch ordering:

```c
if (match_12 && match_13) {
    result = STATUS_OK;
} else if (match_12) {
    result = STATUS_ERROR_PFC3;
} else if (match_13) {
    result = STATUS_ERROR_PFC2;
} else if (match_23) {
    result = STATUS_ERROR_PFC1;
} else {
    result = STATUS_EMERGENCY;
}
```

---

### Key Improvements

#### 1. Uniform Communication

All three PFCs now use the same typed LF interface. Their behavior is independent of the communication mechanism selected by the operating system.

#### 2. Explicit Data Flow

The architecture is visible directly in the main reactor. Every communication path is represented by an LF connection.

#### 3. Deterministic Voting

The WES processes timestamped values according to logical tags and uses a symmetric pairwise TMR decision.

#### 4. Structured Fault Delivery

Faults are represented as data rather than asynchronous POSIX signals. Their delivery and processing order are deterministic.

#### 5. Immediate STOP/CONT Recovery

A stopped PFC suppresses its output but continues advancing its internal GPS state. When it resumes, it is aligned with the same sample and counter as the other PFCs.

#### 6. Transient Bias Isolation

A BIAS fault changes only the transmitted result. It does not permanently corrupt the PFC's internal navigation state.

#### 7. Reproducible Validation

The repository includes:

* Fixed input datasets
* Compatibility and controlled test builds
* Automated validation scripts
* Machine-readable output formats
* Stored result summaries
* Counter-by-counter comparison data

---

### Validation Scope and Limitation

The numerical comparison uses PFC1 from the original C benchmark because its socket communication path produced a complete 100-record output log.

During testing, the original PFC2 named-pipe path did not produce output in the test environment. Therefore, complete original-C voting behavior was evaluated by extracting the WES decision structure into an isolated C harness rather than relying on the incomplete end-to-end IPC run.

The LF end-to-end test evaluates the complete LF reactor system independently, including FMAN, all three PFCs, the Transducer, WES, and PFCDS.

---

### Attribution

This implementation is based on the original [Wabri Fly-By-Wire](https://github.com/Wabri/Fly-By-Wire) benchmark, distributed under the MIT License.

See [`fbw-lf/THIRD_PARTY_NOTICES.md`](fbw-lf/THIRD_PARTY_NOTICES.md) for details.
