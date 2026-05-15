## Architectural Coding Standards Enforced

### 1. Strict Memory Isolation (No Mutexes)
The C implementation relies on a global struct, requiring pthreads, mutexes, shared variable (eg. `pthread_mutex_lock`) usage across multiple separate threads, introducing the risk of data corruption, race conditions, and thread blocking (jitter).
The global must be eliminated. Reactors must maintain entirely isolated private memory. Data must be packaged into a discrete struct and passed safely across explicitly defined ports.

### 2. Logical Time Over Physical Sleep
The C threads use sleep functions (eg.`sleep_ms(1)`). This is non-deterministic and highly susceptible to operating system background noise, leading to missed control cycles.
Physical `sleep()` commands must be replaced with logical timers (e.g., `timer t(0, 1 msec)`). The LF runtime schedules reactor execution precisely based on logical time, ensuring deterministic execution independently of the OS scheduler.

### 3. Explicit Data Flow via Time-Stamped Ports
The C implementation uses condition variables (eg. `pthread_cond_wait`) to wake up sleeping threads.
Data flow must be visually and mathematically explicitly defined in the main assembly line (eg. `plant.x -> optimizer.x_current`). The LF compiler analyzes these arrows to map out causality, automatically resolving the feedback loop between reactors within the exact same logical microsecond.

### 4. Strict Hardware Deadlines
If the C thread's math takes longer than, for example, 1 millisecond, the another thread applies stale (e.g. out-of-date motor commands to the robot), potentially causing hardware failure.
The target is synced to physical time (`fast: false`), and the reactor explicitly utilizes a `deadline(1 msec)` block. If the computation exceeds the physical time boundary, the runtime actively catches the violation, allowing for safe emergency fallbacks.
