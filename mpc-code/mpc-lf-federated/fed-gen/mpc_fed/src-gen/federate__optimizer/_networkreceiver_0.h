#ifndef _NETWORKRECEIVER_0_H
#define _NETWORKRECEIVER_0_H
#include "include/core/reactor.h"
#ifndef TOP_LEVEL_PREAMBLE_285017676_H
#define TOP_LEVEL_PREAMBLE_285017676_H
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
#ifdef __cplusplus
extern "C" {
#endif
#include "core/federated/federate.h"
#include "network/api/net_abstraction.h"
#include "network/api/net_common.h"
#include "network/api/net_util.h"
#include "core/federated/clock-sync.h"
#include "core/threaded/reactor_threaded.h"
#include "core/utils/util.h"
extern federate_instance_t _fed;
#ifdef __cplusplus
}
#endif
#endif // TOP_LEVEL_PREAMBLE_285017676_H
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
} _networkreceiver_0_msg_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_action_internal_t _base;
    self_base_t* parent;
    bool has_value;
    int source_id;
    state_t value;
    #ifdef FEDERATED
    #ifdef FEDERATED_DECENTRALIZED
    tag_t intended_tag;
    #endif
    interval_t physical_time_of_arrival;
    #endif
} _networkreceiver_0_networkMessage_t;
typedef struct {
    struct self_base_t base;
#line 121 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__optimizer/_networkreceiver_0.h"
#line 122 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__optimizer/_networkreceiver_0.h"
    _networkreceiver_0_networkMessage_t _lf_networkMessage;
    _networkreceiver_0_msg_t _lf_msg;
    int _lf_msg_width;
    reaction_t _lf__reaction_0;
    trigger_t _lf__networkMessage;
    reaction_t* _lf__networkMessage_reactions[1];
} _networkreceiver_0_self_t;
_networkreceiver_0_self_t* new__networkreceiver_0();
#endif // _NETWORKRECEIVER_0_H
