#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/mpc/Plant.h"
#include "_plant.h"
// ***** Start of method declarations.
// ***** End of method declarations.
#include "include/api/reaction_macros.h"
void _plantreaction_function_0(void* instance_args) {
    _plant_self_t* self = (_plant_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _plant_x_t* x = &self->_lf_x;
    #line 112 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/mpc.lf"
    double noise = 0.0001 * sin((double)self->tick*0.37);
    state_t current_x;
    current_x.data[0] = self->plant_x[0] + noise;
    current_x.data[1] = self->plant_x[1] + noise*0.5;
    
    lf_set(x, current_x);
    self->tick++;
#line 21 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/src-gen/mpc/_plant.c"
}
#include "include/api/reaction_macros_undef.h"
#include "include/api/reaction_macros.h"
void _plantreaction_function_1(void* instance_args) {
    _plant_self_t* self = (_plant_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _plant_u_t* u = self->_lf_u;
    int u_width = self->_lf_u_width; SUPPRESS_UNUSED_WARNING(u_width);
    #line 123 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/mpc.lf"
     double u_val = u->value;
     double x_new[NX];
    
     // The Physics math
     x_new[0] = A[0][0]*self->plant_x[0] + A[0][1]*self->plant_x[1] + B[0][0]*u_val;
     x_new[1] = A[1][0]*self->plant_x[0] + A[1][1]*self->plant_x[1] + B[1][0]*u_val;
    
     self->plant_x[0] = x_new[0];
     self->plant_x[1] = x_new[1];
    
     if (self->tick % 500 == 0) {
         printf("[actuate] T=%lld ms | tick=%4ld | pos=%.4f | vel=%.4f | u=%.4f\n",
    lf_time_logical_elapsed() / 1000000LL,
    self->tick, self->plant_x[0], self->plant_x[1], u_val);
     }
#line 45 "/mnt/c/Users/Aida/LF-benchmarks/mpc/mpc-code/mpc-lf-main/src-gen/mpc/_plant.c"
}
#include "include/api/reaction_macros_undef.h"
_plant_self_t* new__plant() {
    _plant_self_t* self = (_plant_self_t*)lf_new_reactor(sizeof(_plant_self_t));
    // Set input by default to an always absent default input.
    self->_lf_u = &self->_lf_default__u;
    // Set the default source reactor pointer
    self->_lf_default__u._base.source_reactor = (self_base_t*)self;
    self->_lf__reaction_0.number = 0;
    self->_lf__reaction_0.function = _plantreaction_function_0;
    self->_lf__reaction_0.self = self;
    self->_lf__reaction_0.deadline_violation_handler = NULL;
    self->_lf__reaction_0.STP_handler = NULL;
    self->_lf__reaction_0.name = "?";
    self->_lf__reaction_0.mode = NULL;
    self->_lf__reaction_1.number = 1;
    self->_lf__reaction_1.function = _plantreaction_function_1;
    self->_lf__reaction_1.self = self;
    self->_lf__reaction_1.deadline_violation_handler = NULL;
    self->_lf__reaction_1.STP_handler = NULL;
    self->_lf__reaction_1.name = "?";
    self->_lf__reaction_1.mode = NULL;
    self->_lf__t.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__t.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__t_reactions[0] = &self->_lf__reaction_0;
    self->_lf__t.reactions = &self->_lf__t_reactions[0];
    self->_lf__t.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__t.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__t.is_timer = true;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__t.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__u.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__u.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__u_reactions[0] = &self->_lf__reaction_1;
    self->_lf__u.reactions = &self->_lf__u_reactions[0];
    self->_lf__u.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__u.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__u.tmplt.type.element_size = sizeof(double);
    return self;
}
