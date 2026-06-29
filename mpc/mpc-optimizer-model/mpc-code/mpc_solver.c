/*
 * MPC Solver: Gradient Descent on a Double Integrator
 * Plant:    point mass with state [position, velocity]
 * Model:    x(k+1) = A·x(k) + B·u(k)   (linear, discrete-time)
 * Control:  u is acceleration (force per unit mass)
 * Solver:   finite-difference gradient descent, 50 iterations/tick
 *
 * To replace this solver, create your own mpc_solver.c that
 * implements the mpc_solve() function from mpc_solver.h.
 * Examples: PID controller, LQR, quadratic programming,
 *           neural network, lookup table, etc.
 */

#include "mpc_solver.h"
#include <math.h>
#include <string.h>

/* 
 *  MPC parameters
 *  */
#define HORIZON   10       /* prediction steps                  */
#define DT        0.001    /* sample period (must match timer)  */
#define OPT_ITER  50       /* gradient descent iterations/tick  */
#define OPT_LR    0.01     /* learning rate                     */

/* Cost weights */
#define Q_POS     10.0     /* position tracking weight          */
#define Q_VEL     1.0      /* velocity tracking weight          */
#define R_INPUT   0.1      /* control effort weight             */

/* Input saturation */
#define U_MAX     5.0
#define U_MIN    -5.0

/* 
 *  System model: double integrator (point mass)
 *
 *  State:  x = [position, velocity]
 *  Input:  u = acceleration (force / unit mass)
 *
 *  Continuous:   pos' = vel,   vel' = u
 *  Discretized:  pos(k+1) = pos(k) + DT·vel(k) + ½·DT²·u(k)
 *                vel(k+1) = vel(k) + DT·u(k)
 *  */
static const double A[NX][NX] = {
    {1.0, DT },
    {0.0, 1.0}
};

static const double B[NX][NU] = {
    {0.5 * DT * DT},
    {DT           }
};

/* 
 *  Cost function
 *
 *  J = Σ_{k=0}^{N-1} [ (x-xref)ᵀ Q (x-xref) + uᵀ R u ]
 *    + 10 · (x_N - xref)ᵀ Q (x_N - xref)      (terminal cost)
 *  */
static double compute_cost(
    const double x0[NX],
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

        /* Propagate: x(k+1) = A·x(k) + B·u(k) */
        double xn[NX] = {0};
        for (int i = 0; i < NX; i++) {
            for (int j = 0; j < NX; j++) xn[i] += A[i][j]*x[j];
            for (int j = 0; j < NU; j++) xn[i] += B[i][j]*u_seq[k][j];
        }
        memcpy(x, xn, sizeof(x));
    }

    /* Terminal cost (10x state weight) */
    double ex[NX];
    for (int i = 0; i < NX; i++) ex[i] = x[i] - xref[i];
    cost += 10.0 * Q_POS * ex[0]*ex[0] + 10.0 * Q_VEL * ex[1]*ex[1];

    return cost;
}

/* 
 *  mpc_solve: the function the Optimizer reactor calls
 *  */
double mpc_solve(const double x0[NX], const double xref[NX])
{
    /* Warm-started control sequence (persists across ticks) */
    static double u_seq[HORIZON][NU] = {0};
    double eps = 1e-5;

    /* Gradient descent */
    for (int iter = 0; iter < OPT_ITER; iter++) {
        for (int k = 0; k < HORIZON; k++) {
            for (int j = 0; j < NU; j++) {
                /* Central finite difference */
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

    /* Extract first control action */
    double u = u_seq[0][0];

    /* Warm-start shift: move horizon one step forward */
    for (int k = 0; k < HORIZON - 1; k++)
        memcpy(u_seq[k], u_seq[k+1], NU * sizeof(double));
    memset(u_seq[HORIZON-1], 0, NU * sizeof(double));

    return u;
}
