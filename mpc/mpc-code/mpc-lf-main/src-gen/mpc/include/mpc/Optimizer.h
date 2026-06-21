#ifndef _optimizer_H
#define _optimizer_H
#ifndef _OPTIMIZER_H // necessary for arduino-cli, which automatically includes headers that are not used
#ifndef TOP_LEVEL_PREAMBLE_1732513254_H
#define TOP_LEVEL_PREAMBLE_1732513254_H
/*Correspondence: Range: [(5, 4), (70, 1)) -> Range: [(0, 0), (65, 1)) (verbatim=true; src=/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/mpc.lf)*/#include <math.h>
#include <string.h>

// MPC parameters
#define NX      2        
#define NU      1        
#define HORIZON 10      
#define DT      0.001    
#define OPT_ITER 50      
#define OPT_LR   0.01 

// Cost weights
#define Q_POS   10.0     
#define Q_VEL   1.0                         
#define R_INPUT 0.1

// Input saturation
#define U_MAX   5.0
#define U_MIN  -5.0

// Linear system matrices
static const double A[NX][NX] = {
    {1.0, DT },
    {0.0, 1.0}
};
static const double B[NX][NU] = {
    {0.5 * DT * DT},
    {DT           }
};

// Created struct to pass state across LF ports
typedef struct {
    double data[NX];
} state_t; 

// We past the exact C math function here because it is pure C code
static double compute_cost(const double x0[NX], const double xref[NX], const double u_seq[HORIZON][NU])
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
        for (int j = 0; j < NU; j++) cost += R_INPUT * u_seq[k][j] * u_seq[k][j];

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
#endif // TOP_LEVEL_PREAMBLE_1732513254_H
#ifdef __cplusplus
extern "C" {
#endif
#include "../include/api/schedule.h"
#include "../include/core/reactor.h"
#ifdef __cplusplus
}
#endif
typedef struct optimizer_self_t{
    self_base_t base; // This field is only to be used by the runtime, not the user.
    double* current_ref;
    int end[0]; // placeholder; MSVC does not compile empty structs
} optimizer_self_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    state_t value;
    #ifdef FEDERATED
    #ifdef FEDERATED_DECENTRALIZED
    tag_t intended_tag;
    #endif
    interval_t physical_time_of_arrival;
    #endif
} optimizer_x_current_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    state_t value;
    #ifdef FEDERATED
    #ifdef FEDERATED_DECENTRALIZED
    tag_t intended_tag;
    #endif
    interval_t physical_time_of_arrival;
    #endif
} optimizer_x_ref_in_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    double value;
    #ifdef FEDERATED
    #ifdef FEDERATED_DECENTRALIZED
    tag_t intended_tag;
    #endif
    interval_t physical_time_of_arrival;
    #endif
} optimizer_u_apply_t;
#endif
#endif
