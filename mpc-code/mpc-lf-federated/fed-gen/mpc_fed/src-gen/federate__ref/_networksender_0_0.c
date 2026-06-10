#include "include/api/schedule.h"
#include <string.h>
#include "low_level_platform/api/low_level_platform.h"
#include "include/federate__ref/NetworkSender_0_0.h"
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
    _networksender_0_0_msg_t** msg = self->_lf_msg;
    int msg_width = self->_lf_msg_width; SUPPRESS_UNUSED_WARNING(msg_width);
    #line 139 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src/federate__ref.lf"
    // Sending from msg[0] in federate federate__ref to optimizer.x_ref_in in federate federate__optimizer
    if (!msg[0]->is_present) {
    return;
    }
    size_t _lf_message_length = sizeof(state_t);
    lf_send_tagged_message(self->base.environment, NEVER, MSG_TYPE_TAGGED_MESSAGE, 0, 2, "federate 2 via the RTI", _lf_message_length, (unsigned char*)&msg[0]->value);
#line 25 "/home/aruzhanmassalina/REPOS/LF-benchmarks/mpc-code/mpc-lf-federated/./fed-gen/mpc_fed/src-gen/federate__ref/_networksender_0_0.c"
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
    self->_lf__msg.last_tag = NEVER_TAG;
    #ifdef FEDERATED_DECENTRALIZED
    self->_lf__msg.intended_tag = (tag_t) { .time = NEVER, .microstep = 0u};
    #endif // FEDERATED_DECENTRALIZED
    self->_lf__msg_reactions[0] = &self->_lf__reaction_0;
    self->_lf__msg.reactions = &self->_lf__msg_reactions[0];
    self->_lf__msg.number_of_reactions = 1;
    #ifdef FEDERATED
    self->_lf__msg.physical_time_of_arrival = NEVER;
    #endif // FEDERATED
    self->_lf__msg.tmplt.type.element_size = sizeof(state_t);
    return self;
}
