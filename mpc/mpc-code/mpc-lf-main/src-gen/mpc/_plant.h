#ifndef _PLANT_H
#define _PLANT_H
#include "include/core/reactor.h"
#ifndef TOP_LEVEL_PREAMBLE_1732513254_H
#define TOP_LEVEL_PREAMBLE_1732513254_H
#include <math.h>
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
} _plant_u_t;
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
} _plant_x_t;
typedef struct {
    struct self_base_t base;
#line 104 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/src-gen/mpc/_plant.h"
    #line 106 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/mpc.lf"
    double* plant_x;
    #line 107 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/mpc.lf"
    long tick;
#line 109 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/src-gen/mpc/_plant.h"
    _plant_u_t* _lf_u;
    // width of -2 indicates that it is not a multiport.
    int _lf_u_width;
    // Default input (in case it does not get connected)
    _plant_u_t _lf_default__u;
    _plant_x_t _lf_x;
    int _lf_x_width;
    reaction_t _lf__reaction_0;
    reaction_t _lf__reaction_1;
    trigger_t _lf__t;
    reaction_t* _lf__t_reactions[1];
    trigger_t _lf__u;
    reaction_t* _lf__u_reactions[1];
    #ifdef FEDERATED
    
    #endif // FEDERATED
} _plant_self_t;
_plant_self_t* new__plant();
#endif // _PLANT_H
