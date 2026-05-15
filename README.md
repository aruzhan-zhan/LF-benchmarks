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
    // ... heavy gradient descent math ...
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
    // ... heavy gradient descent math ...
=} deadline(1 msec) {=
    // Safely catches execution if math exceeds physical deadline
    printf("[WARNING] Optimizer missed 1ms real-time deadline!\n");
    // Trigger emergency braking logic here
=}
```
