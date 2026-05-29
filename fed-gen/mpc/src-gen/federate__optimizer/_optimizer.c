#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__optimizer/Optimizer.h"
#include "_optimizer.h"
// ***** Start of method declarations.
// ***** End of method declarations.
#include "include/api/reaction_macros.h"
void _optimizerreaction_function_0(void* instance_args) {
    _optimizer_self_t* self = (_optimizer_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _optimizer_x_ref_in_t* x_ref_in = self->_lf_x_ref_in;
    int x_ref_in_width = self->_lf_x_ref_in_width; SUPPRESS_UNUSED_WARNING(x_ref_in_width);
    #line 117 "/mnt/c/Users/Aida/LF-benchmarks/./fed-gen/mpc/src/federate__optimizer.lf"
    self->current_ref[0] = x_ref_in->value.data[0];
    self->current_ref[1] = x_ref_in->value.data[1];
#line 17 "/mnt/c/Users/Aida/LF-benchmarks/./fed-gen/mpc/src-gen/federate__optimizer/_optimizer.c"
}
#include "include/api/reaction_macros_undef.h"
#include "include/api/reaction_macros.h"
void _optimizerreaction_function_1(void* instance_args) {
    _optimizer_self_t* self = (_optimizer_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _optimizer_x_current_t* x_current = self->_lf_x_current;
    int x_current_width = self->_lf_x_current_width; SUPPRESS_UNUSED_WARNING(x_current_width);
    _optimizer_u_apply_t* u_apply = &self->_lf_u_apply;
    #line 124 "/mnt/c/Users/Aida/LF-benchmarks/./fed-gen/mpc/src/federate__optimizer.lf"
    // 1. read sensor and goal
    double x0[NX];
    x0[0] = x_current->value.data[0];
    x0[1] = x_current->value.data[1];
    
    double xref[NX];
    xref[0] = self->current_ref[0];
    xref[1] = self->current_ref[1];
    
    // 2. memory for the math
    static double u_seq[HORIZON][NU] = {0};
    double eps = 1e-5;
    
    // 3. C math (Gradient Descent)
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
    
    // 4. send the solution
    lf_set(u_apply, u_seq[0][0]);
    
    for (int k = 0; k < HORIZON - 1; k++){
        memcpy(u_seq[k], u_seq[k+1], NU * sizeof(double));
    }
    memset(u_seq[HORIZON-1], 0, NU * sizeof(double));
#line 70 "/mnt/c/Users/Aida/LF-benchmarks/./fed-gen/mpc/src-gen/federate__optimizer/_optimizer.c"
}
void _optimizer_deadline_function1(void* instance_args) {
    _optimizer_self_t* self = (_optimizer_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _optimizer_x_current_t* x_current = self->_lf_x_current;
    int x_current_width = self->_lf_x_current_width; SUPPRESS_UNUSED_WARNING(x_current_width);
    _optimizer_u_apply_t* u_apply = &self->_lf_u_apply;
    #line 169 "/mnt/c/Users/Aida/LF-benchmarks/./fed-gen/mpc/src/federate__optimizer.lf"
    // 5. Hardware safety net
    printf("[WARNING] Optimizer missed 1ms deadline! Applying emergency brakes.\n");
    lf_set(u_apply, 0.0);
#line 81 "/mnt/c/Users/Aida/LF-benchmarks/./fed-gen/mpc/src-gen/federate__optimizer/_optimizer.c"
}
#include "include/api/reaction_macros_undef.h"
_optimizer_self_t* new__optimizer() {
    _optimizer_self_t* self = (_optimizer_self_t*)lf_new_reactor(sizeof(_optimizer_self_t));
    // Set input by default to an always absent default input.
    self->_lf_x_current = &self->_lf_default__x_current;
    // Set the default source reactor pointer
    self->_lf_default__x_current._base.source_reactor = (self_base_t*)self;
    // Set input by default to an always absent default input.
    self->_lf_x_ref_in = &self->_lf_default__x_ref_in;
    // Set the default source reactor pointer
    self->_lf_default__x_ref_in._base.source_reactor = (self_base_t*)self;
    self->_lf__reaction_0.number = 0;
    self->_lf__reaction_0.function = _optimizerreaction_function_0;
    self->_lf__reaction_0.self = self;
    self->_lf__reaction_0.deadline_violation_handler = NULL;
    self->_lf__reaction_0.STP_handler = NULL;
    self->_lf__reaction_0.name = "?";
    self->_lf__reaction_0.mode = NULL;
    self->_lf__reaction_1.number = 1;
    self->_lf__reaction_1.function = _optimizerreaction_function_1;
    self->_lf__reaction_1.self = self;
    self->_lf__reaction_1.deadline_violation_handler = &_optimizer_deadline_function1;
    self->_lf__reaction_1.STP_handler = NULL;
    self->_lf__reaction_1.name = "?";
    self->_lf__reaction_1.mode = NULL;
    self->_lf__x_current.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__x_current.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__x_current_reactions[0] = &self->_lf__reaction_1;
    self->_lf__x_current.reactions = &self->_lf__x_current_reactions[0];
    self->_lf__x_current.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__x_current.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__x_current.tmplt.type.element_size = sizeof(state_t);
    self->_lf__x_ref_in.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__x_ref_in.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__x_ref_in_reactions[0] = &self->_lf__reaction_0;
    self->_lf__x_ref_in.reactions = &self->_lf__x_ref_in_reactions[0];
    self->_lf__x_ref_in.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__x_ref_in.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__x_ref_in.tmplt.type.element_size = sizeof(state_t);
    return self;
}
