#ifndef MPC_SOLVER_H
#define MPC_SOLVER_H

/*
 * MPC Solver Interface
 * 
 * This header defines the contract between the Lingua Franca
 * coordination layer (mpc.lf) and the control math.
 *
 * To plug in your own controller:
 *   1. Keep this header as-is (or change NX/NU for your system)
 *   2. Write your own mpc_solver.c implementing mpc_solve()
 *   3. Recompile with: lfc mpc.lf
 *
 * The Optimizer reactor calls mpc_solve() once per tick and sends
 * the returned value to the Plant. Everything else — timing,
 * scheduling, deadline detection — is handled by LF.
 */

/* State and input dimensions — change these for your system */
#define NX 2    /* number of states  (e.g. position, velocity)  */
#define NU 1    /* number of inputs  (e.g. force)               */

/*
 * Solve for the optimal control input.
 *
 *   x0[NX]   — current measured state
 *   xref[NX] — desired target state
 *
 *   returns  — the control value u to apply this tick
 *
 * This function is called once every 1 ms. If it takes longer
 * than 1 ms of physical time, the LF deadline handler fires
 * and applies the emergency brake (u = 0) instead.
 */
double mpc_solve(const double x0[NX], const double xref[NX]);

#endif /* MPC_SOLVER_H */
