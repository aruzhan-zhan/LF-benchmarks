#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/mpc/Reference.h"
#include "_reference.h"
// ***** Start of method declarations.
// ***** End of method declarations.
#include "include/api/reaction_macros.h"
void _referencereaction_function_0(void* instance_args) {
    _reference_self_t* self = (_reference_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _reference_x_ref_t* x_ref = &self->_lf_x_ref;
    #line 83 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-main/mpc.lf"
    state_t ref = {{0.0, 0.0}};
    lf_set(x_ref, ref);
#line 16 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-main/src-gen/mpc/_reference.c"
}
#include "include/api/reaction_macros_undef.h"
#include "include/api/reaction_macros.h"
void _referencereaction_function_1(void* instance_args) {
    _reference_self_t* self = (_reference_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _reference_x_ref_t* x_ref = &self->_lf_x_ref;
    #line 88 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-main/mpc.lf"
    state_t ref = {{1.0, 0.0}};
    lf_set(x_ref, ref);
    printf("[ref    ] reference -> 1.0\n");
#line 27 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-main/src-gen/mpc/_reference.c"
}
#include "include/api/reaction_macros_undef.h"
#include "include/api/reaction_macros.h"
void _referencereaction_function_2(void* instance_args) {
    _reference_self_t* self = (_reference_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _reference_x_ref_t* x_ref = &self->_lf_x_ref;
    #line 94 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-main/mpc.lf"
    state_t ref = {{0.5, 0.0}};
    lf_set(x_ref, ref);
    printf("[ref    ] reference -> 0.5\n");
#line 38 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-main/src-gen/mpc/_reference.c"
}
#include "include/api/reaction_macros_undef.h"
_reference_self_t* new__reference() {
    _reference_self_t* self = (_reference_self_t*)lf_new_reactor(sizeof(_reference_self_t));
    self->_lf__reaction_0.number = 0;
    self->_lf__reaction_0.function = _referencereaction_function_0;
    self->_lf__reaction_0.self = self;
    self->_lf__reaction_0.deadline_violation_handler = NULL;
    self->_lf__reaction_0.STP_handler = NULL;
    self->_lf__reaction_0.name = "?";
    self->_lf__reaction_0.mode = NULL;
    self->_lf__reaction_1.number = 1;
    self->_lf__reaction_1.function = _referencereaction_function_1;
    self->_lf__reaction_1.self = self;
    self->_lf__reaction_1.deadline_violation_handler = NULL;
    self->_lf__reaction_1.STP_handler = NULL;
    self->_lf__reaction_1.name = "?";
    self->_lf__reaction_1.mode = NULL;
    self->_lf__reaction_2.number = 2;
    self->_lf__reaction_2.function = _referencereaction_function_2;
    self->_lf__reaction_2.self = self;
    self->_lf__reaction_2.deadline_violation_handler = NULL;
    self->_lf__reaction_2.STP_handler = NULL;
    self->_lf__reaction_2.name = "?";
    self->_lf__reaction_2.mode = NULL;
    self->_lf__t1.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__t1.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__t1_reactions[0] = &self->_lf__reaction_1;
    self->_lf__t1.reactions = &self->_lf__t1_reactions[0];
    self->_lf__t1.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__t1.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__t1.is_timer = true;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__t1.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__t2.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__t2.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__t2_reactions[0] = &self->_lf__reaction_2;
    self->_lf__t2.reactions = &self->_lf__t2_reactions[0];
    self->_lf__t2.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__t2.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__t2.is_timer = true;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__t2.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__startup.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__startup_reactions[0] = &self->_lf__reaction_0;
    self->_lf__startup.last_tag = NEVER_TAG;
    self->_lf__startup.reactions = &self->_lf__startup_reactions[0];
    self->_lf__startup.number_of_reactions = 1;
    self->_lf__startup.is_timer = false;
    return self;
}
