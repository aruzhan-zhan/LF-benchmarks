/*
 * mpc_threaded.c
 * Multithreaded Model Predictive Control (MPC) for robotics / motion control
 *
 * Architecture (4 threads):
 *   1. sensor_thread    – reads plant state at 1 kHz (simulated here)
 *   2. prediction_thread – propagates x(k+1) = A*x + B*u over horizon N
 *   3. optimizer_thread  – solves QP to compute optimal u* (gradient descent demo)
 *   4. actuator_thread   – applies u[0] to the plant at 1 kHz
 *
 * Synchronization:
 *   - A single shared_state struct guarded by a mutex
 *   - Condition variables: cond_new_measurement, cond_new_solution
 *
 * Build:
 *   gcc -O2 -Wall -o mpc_threaded mpc_threaded.c -lm -lpthread
 *
 * Tested on Linux (Ubuntu 22.04) and macOS (POSIX-compliant).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

/* ──────────────────────────────────────────────────────────────────────────
   MPC parameters
   ────────────────────────────────────────────────────────────────────────── */
#define NX      2        /* state dimension  (e.g. [position, velocity]) */
#define NU      1        /* input dimension  (e.g. [force / torque])     */
#define HORIZON 10       /* prediction horizon N                          */
#define DT      0.001    /* sample time [s] — 1 kHz control loop         */
#define OPT_ITER 50      /* gradient-descent iterations per solve         */
#define OPT_LR   0.01    /* gradient-descent learning rate                */

/* Cost weights */
#define Q_POS   10.0     /* weight on position error                      */
#define Q_VEL   1.0      /* weight on velocity error                      */
#define R_INPUT 0.1      /* weight on control effort                      */

/* Input saturation */
#define U_MAX   5.0
#define U_MIN  -5.0

/* Runtime: how many 1-ms ticks before clean shutdown */
#define MAX_TICKS 5000

/* ──────────────────────────────────────────────────────────────────────────
   Linear system:  x(k+1) = A*x(k) + B*u(k)
   Double integrator: pos += vel*dt,  vel += u*dt
   ────────────────────────────────────────────────────────────────────────── */
static const double A[NX][NX] = {
    {1.0, DT },
    {0.0, 1.0}
};
static const double B[NX][NU] = {
    {0.5 * DT * DT},
    {DT           }
};

/* ──────────────────────────────────────────────────────────────────────────
   Shared state (protected by mutex)
   ────────────────────────────────────────────────────────────────────────── */
typedef struct {
    double x_current[NX];        /* latest measured state                  */
    double x_reference[NX];      /* reference / setpoint                   */
    double u_sequence[HORIZON][NU]; /* optimal input sequence from solver   */
    double u_applied[NU];        /* u[0] — what the actuator actually sends */

    bool   new_measurement;      /* sensor has written a fresh state        */
    bool   new_solution;         /* optimizer has written a fresh u*        */
    bool   running;              /* global shutdown flag                    */

    long   tick;                 /* global 1-ms counter                     */
} SharedState;

static SharedState  g_state;
static pthread_mutex_t g_mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cond_meas = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  g_cond_sol  = PTHREAD_COND_INITIALIZER;

/* ──────────────────────────────────────────────────────────────────────────
   Utility: sleep for `ms` milliseconds
   ────────────────────────────────────────────────────────────────────────── */
static void sleep_ms(long ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

/* ──────────────────────────────────────────────────────────────────────────
   Plant simulation helper (double integrator, used by sensor thread)
   ────────────────────────────────────────────────────────────────────────── */
static double g_plant_x[NX] = {0.0, 0.0};  /* true plant state (not shared) */

static void plant_step(double u) {
    double x_new[NX];
    x_new[0] = A[0][0]*g_plant_x[0] + A[0][1]*g_plant_x[1] + B[0][0]*u;
    x_new[1] = A[1][0]*g_plant_x[0] + A[1][1]*g_plant_x[1] + B[1][0]*u;
    memcpy(g_plant_x, x_new, sizeof(x_new));
}

/* ──────────────────────────────────────────────────────────────────────────
   THREAD 1 — Sensor thread (1 kHz)
   Simulates reading encoder / IMU; writes x_current under mutex
   ────────────────────────────────────────────────────────────────────────── */
static void *sensor_thread(void *arg) {
    (void)arg;
    printf("[sensor ] thread started\n");

    while (1) {
        sleep_ms(1);  /* 1 kHz */

        pthread_mutex_lock(&g_mutex);
        if (!g_state.running) { pthread_mutex_unlock(&g_mutex); break; }

        /* Add small Gaussian-like noise (deterministic approximation) */
        double noise = 0.0001 * sin((double)g_state.tick * 0.37);

        g_state.x_current[0] = g_plant_x[0] + noise;
        g_state.x_current[1] = g_plant_x[1] + noise * 0.5;
        g_state.new_measurement = true;
        g_state.tick++;

        if (g_state.tick >= MAX_TICKS) g_state.running = false;

        pthread_cond_signal(&g_cond_meas);
        pthread_mutex_unlock(&g_mutex);
    }

    printf("[sensor ] thread exiting (tick=%ld)\n", g_state.tick);
    return NULL;
}

/* ──────────────────────────────────────────────────────────────────────────
   THREAD 2 — Prediction thread (1 kHz)
   Waits for new measurement, propagates nominal trajectory under current u
   ────────────────────────────────────────────────────────────────────────── */
static void *prediction_thread(void *arg) {
    (void)arg;
    printf("[predict] thread started\n");

    double x_pred[HORIZON + 1][NX];

    while (1) {
        pthread_mutex_lock(&g_mutex);
        while (!g_state.new_measurement && g_state.running)
            pthread_cond_wait(&g_cond_meas, &g_mutex);

        if (!g_state.running) { pthread_mutex_unlock(&g_mutex); break; }

        /* Snapshot current state and u sequence */
        double x0[NX];
        memcpy(x0, g_state.x_current, sizeof(x0));

        double u_seq[HORIZON][NU];
        memcpy(u_seq, g_state.u_sequence, sizeof(u_seq));

        g_state.new_measurement = false;
        pthread_mutex_unlock(&g_mutex);

        /* ── Propagate prediction horizon ── */
        memcpy(x_pred[0], x0, sizeof(x0));
        for (int k = 0; k < HORIZON; k++) {
            for (int i = 0; i < NX; i++) {
                x_pred[k+1][i] = 0.0;
                for (int j = 0; j < NX; j++)
                    x_pred[k+1][i] += A[i][j] * x_pred[k][j];
                for (int j = 0; j < NU; j++)
                    x_pred[k+1][i] += B[i][j] * u_seq[k][j];
            }
        }
        /* Prediction result is used internally by optimizer (passed via shared) */
        /* For brevity we skip a second shared buffer; optimizer re-computes    */
    }

    printf("[predict] thread exiting\n");
    return NULL;
}

/* ──────────────────────────────────────────────────────────────────────────
   QP cost function  J = sum_k [ (x-xref)'Q(x-xref) + u'Ru ]
   ────────────────────────────────────────────────────────────────────────── */
static double compute_cost(const double x0[NX],
                            const double xref[NX],
                            const double u_seq[HORIZON][NU])
{
    double cost = 0.0;
    double x[NX];
    memcpy(x, x0, NX * sizeof(double));

    for (int k = 0; k < HORIZON; k++) {
        /* State cost */
        double ex[NX];
        for (int i = 0; i < NX; i++) ex[i] = x[i] - xref[i];
        cost += Q_POS * ex[0]*ex[0] + Q_VEL * ex[1]*ex[1];

        /* Input cost */
        for (int j = 0; j < NU; j++)
            cost += R_INPUT * u_seq[k][j] * u_seq[k][j];

        /* Propagate */
        double xn[NX] = {0};
        for (int i = 0; i < NX; i++) {
            for (int j = 0; j < NX; j++) xn[i] += A[i][j]*x[j];
            for (int j = 0; j < NU; j++) xn[i] += B[i][j]*u_seq[k][j];
        }
        memcpy(x, xn, sizeof(x));
    }
    /* Terminal cost */
    double ex[NX];
    for (int i = 0; i < NX; i++) ex[i] = x[i] - xref[i];
    cost += 10.0 * Q_POS * ex[0]*ex[0] + 10.0 * Q_VEL * ex[1]*ex[1];

    return cost;
}

/* ──────────────────────────────────────────────────────────────────────────
   THREAD 3 — Optimizer thread  (runs as fast as possible)
   Gradient-descent on the QP; writes u_sequence when converged
   ────────────────────────────────────────────────────────────────────────── */
static void *optimizer_thread(void *arg) {
    (void)arg;
    printf("[optim  ] thread started\n");

    double u_seq[HORIZON][NU];
    memset(u_seq, 0, sizeof(u_seq));

    while (1) {
        /* Snapshot state and reference */
        pthread_mutex_lock(&g_mutex);
        if (!g_state.running) { pthread_mutex_unlock(&g_mutex); break; }
        double x0[NX], xref[NX];
        memcpy(x0,   g_state.x_current,   sizeof(x0));
        memcpy(xref, g_state.x_reference, sizeof(xref));
        pthread_mutex_unlock(&g_mutex);

        /* ── Gradient-descent solve ── */
        double eps = 1e-5;  /* finite-difference step */
        for (int iter = 0; iter < OPT_ITER; iter++) {
            for (int k = 0; k < HORIZON; k++) {
                for (int j = 0; j < NU; j++) {
                    /* Forward difference */
                    double u_p[HORIZON][NU], u_m[HORIZON][NU];
                    memcpy(u_p, u_seq, sizeof(u_seq));
                    memcpy(u_m, u_seq, sizeof(u_seq));
                    u_p[k][j] += eps;
                    u_m[k][j] -= eps;

                    double grad = (compute_cost(x0, xref, u_p) -
                                   compute_cost(x0, xref, u_m)) / (2.0*eps);

                    u_seq[k][j] -= OPT_LR * grad;

                    /* Saturate */
                    if (u_seq[k][j] > U_MAX) u_seq[k][j] = U_MAX;
                    if (u_seq[k][j] < U_MIN) u_seq[k][j] = U_MIN;
                }
            }
        }

        /* Write result */
        pthread_mutex_lock(&g_mutex);
        memcpy(g_state.u_sequence, u_seq, sizeof(u_seq));
        g_state.new_solution = true;
        pthread_cond_signal(&g_cond_sol);
        pthread_mutex_unlock(&g_mutex);

        /* Warm-start: shift horizon by one for next call */
        for (int k = 0; k < HORIZON - 1; k++)
            memcpy(u_seq[k], u_seq[k+1], NU * sizeof(double));
        memset(u_seq[HORIZON-1], 0, NU * sizeof(double));
    }

    printf("[optim  ] thread exiting\n");
    return NULL;
}

/* ──────────────────────────────────────────────────────────────────────────
   THREAD 4 — Actuator thread (1 kHz)
   Waits for a new solution, then applies u[0] every tick
   ────────────────────────────────────────────────────────────────────────── */
static void *actuator_thread(void *arg) {
    (void)arg;
    printf("[actuate] thread started\n");

    long last_print_tick = 0;

    while (1) {
        sleep_ms(1);  /* 1 kHz */

        pthread_mutex_lock(&g_mutex);
        if (!g_state.running) { pthread_mutex_unlock(&g_mutex); break; }

        /* Apply best available u[0] */
        double u_apply[NU];
        memcpy(u_apply, g_state.u_sequence[0], NU * sizeof(double));
        memcpy(g_state.u_applied, u_apply, NU * sizeof(double));

        long tick = g_state.tick;
        double x0 = g_state.x_current[0];
        double x1 = g_state.x_current[1];
        double xr = g_state.x_reference[0];
        pthread_mutex_unlock(&g_mutex);

        /* Simulate the plant one step */
        plant_step(u_apply[0]);

        /* Console log every 500 ticks (0.5 s) */
        if (tick - last_print_tick >= 500) {
            printf("[actuate] tick=%4ld | pos=%.4f (ref=%.4f) | vel=%.4f | u=%.4f\n",
                   tick, x0, xr, x1, u_apply[0]);
            last_print_tick = tick;
        }
    }

    printf("[actuate] thread exiting\n");
    return NULL;
}

/* ──────────────────────────────────────────────────────────────────────────
   Reference trajectory: step from 0 → 1.0 at t=1s, then → 0.5 at t=3s
   ────────────────────────────────────────────────────────────────────────── */
static void *reference_thread(void *arg) {
    (void)arg;
    sleep_ms(1000);  /* wait 1 s */
    pthread_mutex_lock(&g_mutex);
    g_state.x_reference[0] = 1.0;
    g_state.x_reference[1] = 0.0;
    pthread_mutex_unlock(&g_mutex);
    printf("[ref    ] reference → 1.0\n");

    sleep_ms(2000);  /* wait 2 s */
    pthread_mutex_lock(&g_mutex);
    g_state.x_reference[0] = 0.5;
    g_state.x_reference[1] = 0.0;
    pthread_mutex_unlock(&g_mutex);
    printf("[ref    ] reference → 0.5\n");

    return NULL;
}

/* ──────────────────────────────────────────────────────────────────────────
   main
   ────────────────────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== Multithreaded MPC — Robotics Motion Control ===\n");
    printf("    NX=%d  NU=%d  N=%d  dt=%.3f s\n\n", NX, NU, HORIZON, DT);

    /* Init shared state */
    memset(&g_state, 0, sizeof(g_state));
    g_state.running = true;
    g_state.x_reference[0] = 0.0;
    g_state.x_reference[1] = 0.0;

    /* Launch threads */
    pthread_t t_sensor, t_predict, t_optim, t_actuate, t_ref;

    pthread_create(&t_sensor,  NULL, sensor_thread,    NULL);
    pthread_create(&t_predict, NULL, prediction_thread, NULL);
    pthread_create(&t_optim,   NULL, optimizer_thread,  NULL);
    pthread_create(&t_actuate, NULL, actuator_thread,   NULL);
    pthread_create(&t_ref,     NULL, reference_thread,  NULL);

    /* Wait for all threads to finish */
    pthread_join(t_sensor,  NULL);
    pthread_join(t_predict, NULL);
    pthread_join(t_optim,   NULL);
    pthread_join(t_actuate, NULL);
    pthread_join(t_ref,     NULL);

    /* Cleanup */
    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond_meas);
    pthread_cond_destroy(&g_cond_sol);

    printf("\n=== Simulation complete ===\n");
    printf("Final plant state: pos=%.4f  vel=%.4f\n",
           g_plant_x[0], g_plant_x[1]);

    return 0;
}
