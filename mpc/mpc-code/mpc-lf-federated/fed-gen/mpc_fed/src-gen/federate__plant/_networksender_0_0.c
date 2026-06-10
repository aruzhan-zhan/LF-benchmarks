#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__plant/NetworkSender_0_0.h"
#include "_networksender_0_0.h"
#if defined SENDERINDEXPARAMETER
#undef SENDERINDEXPARAMETER
#endif
#define SENDERINDEXPARAMETER 0
// ***** Start of method declarations.
// ***** End of method declarations.
#include "include/api/reaction_macros.h"
void _networksender_0_0reaction_function_0(void* instance_args) {
    _networksender_0_0_self_t* self = (_networksender_0_0_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    
    #line 159 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src/federate__plant.lf"
    extern reaction_t* port_absent_reaction[];
    void lf_enqueue_port_absent_reactions(environment_t*);
    LF_PRINT_DEBUG("Adding network port absent reaction to table.");
    port_absent_reaction[SENDERINDEXPARAMETER] = &self->_lf__reaction_2;
    LF_PRINT_DEBUG("Added network output control reaction to table. Enqueueing it...");
    lf_enqueue_port_absent_reactions(self->base.environment);
#line 24 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__plant/_networksender_0_0.c"
}
#include "include/api/reaction_macros_undef.h"
#include "include/api/reaction_macros.h"
void _networksender_0_0reaction_function_1(void* instance_args) {
    _networksender_0_0_self_t* self = (_networksender_0_0_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _networksender_0_0_msg_t** msg = self->_lf_msg;
    int msg_width = self->_lf_msg_width; SUPPRESS_UNUSED_WARNING(msg_width);
    #line 168 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src/federate__plant.lf"
    // Sending from msg[0] in federate federate__plant to optimizer.x_current in federate federate__optimizer
    if (!msg[0]->is_present) {
    return;
    }
    size_t _lf_message_length = sizeof(state_t);
    lf_send_tagged_message(self->base.environment, NEVER, MSG_TYPE_TAGGED_MESSAGE, 1, 2, "federate 2 via the RTI", _lf_message_length, (unsigned char*)&msg[0]->value);
#line 39 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__plant/_networksender_0_0.c"
}
#include "include/api/reaction_macros_undef.h"
#include "include/api/reaction_macros.h"
void _networksender_0_0reaction_function_2(void* instance_args) {
    _networksender_0_0_self_t* self = (_networksender_0_0_self_t*)instance_args; SUPPRESS_UNUSED_WARNING(self);
    _networksender_0_0_msg_t** msg = self->_lf_msg;
    int msg_width = self->_lf_msg_width; SUPPRESS_UNUSED_WARNING(msg_width);
    #line 177 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src/federate__plant.lf"
    // If the output port has not been lf_set for the current logical time,
    // send an ABSENT message to the receiving federate
    LF_PRINT_LOG("Executing port absent reaction for port %d to federate %d at time" PRINTF_TIME ".",
              1, 2, (long long) lf_time_logical_elapsed());
    if (msg[0] == NULL || !msg[0]->is_present) {
    LF_PRINT_LOG("The output port is NULL or it is not present.");
        lf_send_port_absent_to_federate(self->base.environment, NEVER, 1, 2);
    }
#line 56 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__plant/_networksender_0_0.c"
}
#include "include/api/reaction_macros_undef.h"
_networksender_0_0_self_t* new__networksender_0_0() {
    _networksender_0_0_self_t* self = (_networksender_0_0_self_t*)lf_new_reactor(sizeof(_networksender_0_0_self_t));
    // Set the default source reactor pointer
    self->_lf_default__msg._base.source_reactor = (self_base_t*)self;
    self->_lf__reaction_0.number = 0;
    self->_lf__reaction_0.function = _networksender_0_0reaction_function_0;
    self->_lf__reaction_0.self = self;
    self->_lf__reaction_0.deadline_violation_handler = NULL;
    self->_lf__reaction_0.STP_handler = NULL;
    self->_lf__reaction_0.name = "?";
    self->_lf__reaction_0.mode = NULL;
    self->_lf__reaction_1.number = 1;
    self->_lf__reaction_1.function = _networksender_0_0reaction_function_1;
    self->_lf__reaction_1.self = self;
    self->_lf__reaction_1.deadline_violation_handler = NULL;
    self->_lf__reaction_1.STP_handler = NULL;
    self->_lf__reaction_1.name = "?";
    self->_lf__reaction_1.mode = NULL;
    self->_lf__reaction_2.number = 2;
    self->_lf__reaction_2.function = _networksender_0_0reaction_function_2;
    self->_lf__reaction_2.self = self;
    self->_lf__reaction_2.deadline_violation_handler = NULL;
    self->_lf__reaction_2.STP_handler = NULL;
    self->_lf__reaction_2.name = "?";
    self->_lf__reaction_2.mode = NULL;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__startup.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__startup_reactions[0] = &self->_lf__reaction_0;
    self->_lf__startup.last_tag = NEVER_TAG;
    self->_lf__startup.reactions = &self->_lf__startup_reactions[0];
    self->_lf__startup.number_of_reactions = 1;
    self->_lf__startup.is_timer = false;
    self->_lf__msg.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__msg.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__msg_reactions[0] = &self->_lf__reaction_1;
    self->_lf__msg.reactions = &self->_lf__msg_reactions[0];
    self->_lf__msg.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__msg.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__msg.tmplt.type.element_size = sizeof(state_t);
    return self;
}
